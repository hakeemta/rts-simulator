#include <TaskSystem.hpp>
#include <cassert>
#include <chrono>
#include <iostream>
#include <numeric>
#include <set>
#include <utils.hpp>

// Init static variables
int Task::_idCount = 0;
int Processor::_idCount = 0;

TaskSystem::TaskSystem(int m, bool log) : _m(m) {
  /* Initializes the task system with the set number of processors.
   */
  for (int i = 0; i < m; i++) {
    _processors.emplace_back(std::make_unique<Processor>());
  }

  if (log) {
    _display = std::make_shared<Display>(_m);
  }

  Task::resetIdCount();
  Processor::resetIdCount();
};

TaskSystem::TaskSystem(TaskSystem &&source) {
  _m = source._m;
  _n = source._n;
  _util = source._util;

  _t = source._t;
  _quantumSize = source._quantumSize;
  _hyperperiod = source._hyperperiod;

  _display = std::move(source._display);

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
  _n = source._n;
  _util = source._util;

  _t = source._t;
  _quantumSize = source._quantumSize;
  _hyperperiod = source._hyperperiod;

  _display = std::move(source._display);

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
  _hyperperiod = 1;
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
    auto &processor = _processors[k];
    auto procIdx = processor->id() - 1;
    auto &task = _dispatchedTasks.emplace_back(std::move(_readyTasks[i]));
    task->allocateProcessor(std::move(processor));
    task->dispatch(dt);

    if (_display != nullptr) {
      _display->updateTrace(procIdx, task->id());
      _display->updateList(Display::ListingType::RUNNING, procIdx, task->id(),
                           task->toString());
    }
  }

  refresh(_processors);
  refresh(_readyTasks);
}

void TaskSystem::dispatchTasks(TaskSubSet &tasks, time_t dt) {
  /* Idles tasks without processors.
   */

  int readyCount = 0;
  for (auto &task : tasks) {
    task->dispatch(dt);
    auto &_task = _dispatchedTasks.emplace_back(std::move(task));

    if (_display == nullptr) {
      continue;
    }
    if (_task->status() == Task::Status::IDLE) {
      _display->updateList(Display::ListingType::IDLE, readyCount++,
                           _task->id(), task->toString());
    }
  }
  tasks.clear();
}

void TaskSystem::acquireResources(TaskPtr &task) {
  /* Releases processors from tasks to the pool.
   */
  if (task->hasProcessor()) {
    _processors.emplace_back(task->releaseProcessor());
  }
}

void TaskSystem::addTask(Task::Parameters params) {
  /* Creates a new task and validates its utilization.
     Recomputes the system's timing attributes
     and adds the task to ready.
   */

  auto task = std::make_unique<Task>(params);
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

  _display->updateStatus(toString());
}

void TaskSystem::reset() {
  /* Acquires any resources from the disptached tasks.
     Returns all tasks to ready and resets them.
  */
  _t = 0;

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

time_t TaskSystem::nextEventAt() const {
  time_t nearest;
  for (auto &task : _readyTasks) {
    nearest = std::min({nearest, task->attrs().Ct, task->attrs().Dt});
  }

  for (auto &task : _completedTasks) {
    nearest = std::min({nearest, task->attrs().Dt});
  }

  return nearest;
};

TaskState TaskSystem::operator()(const std::vector<int> &indices,
                                 time_t proportion) {
  /* Dispatches tasks to run selected ready and
    idle the rest (and previously completed).
    Steps tasks to check for completed tasks and
    release resources.
  */
  if (_display != nullptr) {
    _display->clearLists();
  }

  auto dt = _quantumSize * proportion;
  dispatchTasks(indices, dt);
  dispatchTasks(_readyTasks, dt);
  dispatchTasks(_completedTasks, dt);

  _t += dt;
  for (auto &task : _dispatchedTasks) {
    if (!task->stepped(_t)) {
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

std::string TaskSystem::toString() const {
  char str[64];
  sprintf(str, "U=%.2f, n=%d, m=%d, H=%ld, dt=%ld", _util, _n, _m, _hyperperiod,
          _quantumSize);
  return std::string(str);
}