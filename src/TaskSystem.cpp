#include <TaskSystem.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <set>
#include <thread>

// Init static variables
int Task::_idCount = 0;
int Processor::_idCount = 0;

template <class T>
void copy(std::vector<std::unique_ptr<T>> &dest,
          const std::vector<std::unique_ptr<T>> &src) {
  dest.clear();
  dest.reserve(src.size());
  std::transform(
      src.begin(), src.end(), std::back_inserter(dest),
      [](const auto &entity) { return std::make_unique<T>(*entity); });
}

template <class T> void refresh(std::vector<std::unique_ptr<T>> &v) {
  /* Purges null (allocated) entities.
   */
  v.erase(std::remove_if(v.begin(), v.end(),
                         [](auto &entity) { return entity == nullptr; }),
          v.end());
}

TaskSystem::TaskSystem(int m) : _m(m) {
  /* Initializes the task system with the set number of processors.
   */
  for (int i = 0; i < m; i++) {
    _processors.emplace_back(std::make_unique<Processor>());
  }
};

TaskSystem::TaskSystem(const TaskSystem &source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  copy(_ready, source._ready);
  copy(_dispatched, source._dispatched);
  copy(_completed, source._completed);
  copy(_processors, source._processors);
}

TaskSystem &TaskSystem::operator=(const TaskSystem &source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  copy(_ready, source._ready);
  copy(_dispatched, source._dispatched);
  copy(_completed, source._completed);
  copy(_processors, source._processors);

  return *this;
}

void TaskSystem::invalidate() {
  _m = 1;
  _t = 0;
  _util = 0;
  _dt = 0;
}

TaskSystem::TaskSystem(TaskSystem &&source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _ready = std::move(source._ready);
  _dispatched = std::move(source._dispatched);
  _completed = std::move(source._completed);
  _processors = std::move(source._processors);

  source.invalidate();
}

TaskSystem &TaskSystem::operator=(TaskSystem &&source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _ready = std::move(source._ready);
  _dispatched = std::move(source._dispatched);
  _completed = std::move(source._completed);
  _processors = std::move(source._processors);

  source.invalidate();
  return *this;
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
  _dt = std::reduce(v.begin(), v.end(), _dt,
                    [](const time_t &init, const time_t &first) {
                      return std::gcd(init, first);
                    });
  _L = std::lcm(_L, p.T);

  _ready.emplace_back(std::move(task));
  _n += 1;
}

void TaskSystem::acquireResources(std::unique_ptr<Task> &task) {
  /* Releases processors from tasks to the pool.
   */
  auto processor = task->releaseProcessor();
  if (processor != nullptr) {
    _processors.emplace_back(std::move(processor));
  }
}

void TaskSystem::reset() {
  /* Acquires any resources from the disptached tasks.
     Returns all tasks to ready and resets them.
  */
  _t = 0;

  for (auto &task : _dispatched) {
    acquireResources(task);
    _ready.emplace_back(std::move(task));
  }

  for (auto &task : _completed) {
    _ready.emplace_back(std::move(task));
  }

  for (auto &task : _ready) {
    task->reset();
  }

  refresh(_dispatched);
  refresh(_completed);
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
  if (maxIndex >= _ready.size()) {
    throw std::out_of_range("At least one job is out of index!");
  }

  for (int k = 0; k < indices.size(); k++) {
    const auto &i = indices[k];
    auto &task = _dispatched.emplace_back(std::move(_ready[i]));
    task->allocate(std::move(_processors[k]), dt);
  }

  refresh(_processors);
  refresh(_ready);
}

void TaskSystem::idleTasks(TaskSubSet &tasks, time_t dt) {
  /* Dispatches (idles) tasks without processors.
   */

  for (auto &task : tasks) {
    task->allocate(nullptr, dt);
    _dispatched.emplace_back(std::move(task));
  }
  tasks.clear();
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
  idleTasks(_ready);
  idleTasks(_completed);

  _t += dt;
  for (auto &task : _dispatched) {
    if (!task->step(_t)) {
      continue;
    }

    acquireResources(task);
    if (task->ready()) {
      _ready.emplace_back(std::move(task));
    } else {
      _completed.emplace_back(std::move(task));
    }
  }
  refresh(_dispatched);

  return readyState();
}
