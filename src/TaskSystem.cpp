#include <TaskSystem.hpp>
#include <cassert>
#include <chrono>
#include <iostream>
#include <numeric>
#include <set>
#include <thread>
#include <utils.hpp>

// Init static variables
int Task::_idCount = 0;
int Processor::_idCount = 0;
std::mutex AsyncTask::_mutex;

TaskSystem::TaskSystem(int m) : _m(m) {
  /* Initializes the task system with the set number of processors.
   */
  for (int i = 0; i < m; i++) {
    _processors.emplace_back(std::make_unique<AsyncProcessor>());
  }
  _timer = std::make_shared<Timer>();
  _display = std::make_shared<Display>(_m);
};

TaskSystem::TaskSystem(TaskSystem &&source) {
  _m = source._m;
  _util = source._util;

  _quantumSize = source._quantumSize;
  _timer = std::move(source._timer);

  _readyTasks = std::move(source._readyTasks);
  _dispatchedTasks = std::move(source._dispatchedTasks);
  _completedTasks = std::move(source._completedTasks);
  _processors = std::move(source._processors);

  source.invalidate();
}

TaskSystem &TaskSystem::operator=(TaskSystem &&source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _util = source._util;

  _quantumSize = source._quantumSize;
  _timer = std::move(source._timer);

  _readyTasks = std::move(source._readyTasks);
  _dispatchedTasks = std::move(source._dispatchedTasks);
  _completedTasks = std::move(source._completedTasks);
  _processors = std::move(source._processors);

  source.invalidate();
  return *this;
}

void TaskSystem::invalidate() {
  _m = 1;
  _util = 0;
  _quantumSize = 0;
}

TaskState TaskSystem::getState(const TaskSubSet &tasks) {
  /* Packs tuples of (id, parameters, attributes) of tasks.
   */
  TaskState state;
  for (int i = 0; i < tasks.size(); i++) {
    const auto &task = tasks[i];
    state.emplace_back(task->id(), task->params(), task->attrs());
  }
  return state;
}

void TaskSystem::dispatchTasks(const std::vector<int> &indices, time_t dt) {
  /* Runs validations for selected task indices
    to check for potential timing faults.

    Dispatches (runs) tasks with assigned processors.
  */

  if (indices.empty()) {
    return;
  }

  if (_processors.size() < indices.size()) {
    throw std::out_of_range("More jobs than the available processors!");
  }

  std::set<int> uniqueIndices(indices.begin(), indices.end());
  if (uniqueIndices.size() < indices.size()) {
    throw std::out_of_range("Jobs are not unique!");
  }

  auto maxIndex = *std::max_element(indices.begin(), indices.end());
  if (maxIndex >= _readyTasks.size()) {
    throw std::out_of_range("At least one job is out of index!");
  }

  for (int k = 0; k < indices.size(); k++) {
    const auto &i = indices[k];
    auto &task = _dispatchedTasks.emplace_back(std::move(_readyTasks[i]));
    task->allocateProcessor(std::move(_processors[k]));
    task->dispatch(dt);
  }

  refresh(_processors);
  refresh(_readyTasks);
}

void TaskSystem::dispatchTasks(TaskSubSet &tasks, time_t dt) {
  /* Idles tasks without processors.
   */

  for (auto &task : tasks) {
    task->dispatch(dt);
    _dispatchedTasks.emplace_back(std::move(task));
  }
  tasks.clear();
}

void TaskSystem::acquireResources(TaskPtr &task) {
  /* Releases processors and threads from tasks to the pool.
   */
  auto processor = task->releaseProcessor();
  if (processor != nullptr) {
    processor->releaseThread();
    _processors.emplace_back(std::move(processor));
  }
}

void TaskSystem::addTask(Task::Parameters params) {
  /* Creates a new task and validates its utilization.
     Recomputes the system's timing attributes
     and adds the task to ready.
   */

  auto task = std::make_unique<AsyncTask>(params, _timer, _display);
  if (task->util() == 0) {
    return;
  }
  _util += task->util();
  assert(_util <= _m);

  Task::Parameters p = task->params();
  std::vector<time_t> v{p.C, p.D, p.T};
  _quantumSize = std::reduce(v.begin(), v.end(), _quantumSize,
                             [](const time_t &init, const time_t &first) {
                               return std::gcd(init, first);
                             });
  _hyperperiod = std::lcm(_hyperperiod, p.T);

  _readyTasks.emplace_back(std::move(task));
  _n += 1;
}

void TaskSystem::loadTasks(std::string filename) {
  auto tasks = loadTaskset(filename);
  if (tasks.empty()) {
    return;
  }

  for (auto &task : tasks) {
    auto [C, T] = task;
    addTask(Task::Parameters{C, T});
  }

  char title[64];
  sprintf(title, "Util:%.2f, #Tasks:%d, #Procs:%d, #Steps:%ld", _util, _n, _m,
          _hyperperiod);
  _display->updateStatus(title);
}

void TaskSystem::reset() {
  /* Acquires any resources from the disptached tasks.
     Returns all tasks to ready and resets them.
  */
  _timer->reset();

  for (auto &task : _dispatchedTasks) {
    acquireResources(task);
    _readyTasks.emplace_back(std::move(task));
  }

  for (auto &task : _completedTasks) {
    _readyTasks.emplace_back(std::move(task));
  }

  for (auto &task : _readyTasks) {
    task->reset();
  }

  refresh(_dispatchedTasks);
  refresh(_completedTasks);
}

void TaskSystem::displayListings() {
  int readyCount = 0;
  int completedCount = 0;
  for (auto &task : _dispatchedTasks) {
    if (task->status() == Task::Status::IDLE) {
      _display->updateList(Display::ListingType::IDLE, readyCount++, task->id(),
                           (*task)());
    }
  }
}

TaskState TaskSystem::operator()(const std::vector<int> &indices,
                                 time_t proportion) {
  /* Dispatches tasks to run selected ready and
    idle the rest (and previously completed).
    Steps tasks to check for completed tasks and
    release resources.
  */

  auto dt = _quantumSize * proportion;
  dispatchTasks(indices, dt);
  dispatchTasks(_readyTasks, dt);
  dispatchTasks(_completedTasks, dt);

  for (time_t t = 0; t < dt; t++) {
    _display->clearLists();
    displayListings();
    _timer->increment();
    // Wait for any awaken threads
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  auto t = _timer->get();
  for (auto &task : _dispatchedTasks) {
    if (!task->stepped(t)) {
      continue;
    }

    acquireResources(task);
    if (task->ready()) {
      _readyTasks.emplace_back(std::move(task));
    } else {
      _completedTasks.emplace_back(std::move(task));
    }
  }
  refresh(_dispatchedTasks);

  return readyState();
}
