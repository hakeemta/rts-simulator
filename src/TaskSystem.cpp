#include <TaskSystem.hpp>
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
    _processors.emplace_back(std::make_unique<Processor>());
  }
  _timer = std::make_shared<Timer>();
};

TaskSystem::TaskSystem(TaskSystem &&source) {
  _m = source._m;
  _util = source._util;

  _t = source._t;
  _dt = source._dt;
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

  _t = source._t;
  _dt = source._dt;
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
  _t = 0;
  _dt = 0;
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

void TaskSystem::runTasks(const std::vector<int> &indices, time_t dt) {
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
    task->dispatch(std::move(_processors[k]), dt);
  }

  refresh(_processors);
  refresh(_readyTasks);
}

void TaskSystem::idleTasks(TaskSubSet &tasks, time_t dt) {
  /* Dispatches (idles) tasks without processors.
   */

  for (auto &task : tasks) {
    task->dispatch(nullptr, dt);
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

  auto task = std::make_unique<AsyncTask>(params);
  if (task->util() == 0) {
    return;
  }
  _util += task->util();
  assert(_util <= _m);

  Task::Parameters p = task->params();
  std::vector<time_t> v{p.C, p.D, p.T};
  _dt = std::reduce(v.begin(), v.end(), _dt,
                    [](const time_t &init, const time_t &first) {
                      return std::gcd(init, first);
                    });
  _L = std::lcm(_L, p.T);

  task->linkTimer(_timer);
  _readyTasks.emplace_back(std::move(task));
  _n += 1;
}

void TaskSystem::reset() {
  /* Acquires any resources from the disptached tasks.
     Returns all tasks to ready and resets them.
  */
  _t = 0;
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

TaskState TaskSystem::operator()(const std::vector<int> &indices,
                                 time_t proportion) {
  /* Dispatches tasks to run selected ready and
    idle the rest (and previously completed).
    Steps tasks to check for completed tasks and
    release resources.
  */

  auto dt = _dt * proportion;
  runTasks(indices, dt);
  idleTasks(_readyTasks);
  idleTasks(_completedTasks);

  _t += dt;
  _timer->increment(dt);
  // Wait for any awaken threads
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  for (auto &task : _dispatchedTasks) {
    if (!task->stepped(_t)) {
      continue;
    }

    // std::cout << task->id() << " stepped!" << std::endl;
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
