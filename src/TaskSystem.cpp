#include <TaskSystem.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <thread>

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
  v.erase(std::remove_if(v.begin(), v.end(),
                         [](auto &entity) { return entity == nullptr; }),
          v.end());
}

TaskSystem::TaskSystem() { TaskSystem(1); };

TaskSystem::TaskSystem(int m) : _m(m) {
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
  copy(_running, source._running);
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
  copy(_running, source._running);
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
  _running = std::move(source._running);
  _completed = std::move(source._completed);
  _processors = std::move(source._processors);

  // Invalidate the source data
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
  _running = std::move(source._running);
  _completed = std::move(source._completed);
  _processors = std::move(source._processors);

  // Invalidate the source data
  source.invalidate();

  return *this;
}

void TaskSystem::addTask(Task::Parameters params) {
  auto task = std::make_unique<Task>(params);
  _util += task->util();
  _n += 1;
  assert(_util <= _m);

  Task::Parameters p = task->Params();
  std::vector<time_t> v{p.C, p.D, p.T};
  _dt = std::reduce(v.begin(), v.end(), _dt,
                    [](const time_t &init, const time_t &first) {
                      return std::gcd(init, first);
                    });

  _ready.emplace_back(std::move(task));
}

void TaskSystem::reset() {
  _t = 0;

  for (int i = 0; i < _ready.size(); i++) {
    auto &task = _ready[i];
    task->reset();
  }

  for (int i = 0; i < _running.size(); i++) {
    auto &task = _running[i];
    task->reset();
    _ready.emplace_back(std::move(task));
  }
  refresh(_running);

  for (int i = 0; i < _completed.size(); i++) {
    auto &task = _completed[i];
    task->reset();
    _ready.emplace_back(std::move(task));
  }
  refresh(_completed);
}

State TaskSystem::getState(std::vector<std::unique_ptr<Task>> &tasks) {
  State state;

  for (int i = 0; i < tasks.size(); i++) {
    const auto &task = tasks[i];
    state.emplace_back(task->Params(), task->Attrs());
  }

  return state;
}

State TaskSystem::operator()() { return getState(_ready); };

State TaskSystem::Completed() { return getState(_completed); }

State TaskSystem::operator()(const std::vector<int> &indices) {
  // Selected ready tasks
  for (int k = 0; k < indices.size(); k++) {
    const auto &index = indices[k];
    auto &task = _ready[index];
    auto &proc = _processors[k];

    // std::cout << task->getId() << " SELECTED to run on " << proc->getId()
    //           << " for " << _dt << std::endl;

    task->allocate(std::move(proc), _dt);
    _running.emplace_back(std::move(task));
  }

  // Purge null (allocated) tasks with corresponding processors
  refresh(_processors);
  refresh(_ready);

  // Step unselected and previously completed tasks
  for (int i = 0; i < _ready.size(); i++) {
    auto &task = _ready[i];
    task->allocate(nullptr, _dt);
    _running.emplace_back(std::move(task));
  }
  _ready.clear();

  for (int i = 0; i < _completed.size(); i++) {
    auto &task = _completed[i];
    task->allocate(nullptr, _dt);
    _running.emplace_back(std::move(task));
  }
  _completed.clear();

  _t += _dt;
  for (int i = 0; i < _running.size(); i++) {
    auto &task = _running[i];
    if (task->step(_t)) {
      // Release resources
      auto _processor = task->releaseProcessor();
      if (_processor != nullptr) {
        _processors.emplace_back(std::move(_processor));
      }

      if (task->ready()) {
        // Add to ready
        _ready.emplace_back(std::move(task));
      } else {
        // Add to completed
        _completed.emplace_back(std::move(task));
      }
    }
  }
  refresh(_running);

  return this->operator()();
}
