#include "../include/TaskSystem.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>

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

  _ready = source._ready;
  _completed = source._completed;
}

TaskSystem &TaskSystem::operator=(const TaskSystem &source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _ready.clear();
  _completed.clear();
  _ready = source._ready;
  _completed = source._completed;

  return *this;
}

TaskSystem::TaskSystem(TaskSystem &&source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _ready = std::move(source._ready);
  _completed = std::move(source._completed);

  // Invalidate the source data
  source._m = 1;
  source._t = 0;
  source._util = 0;
  source._dt = 0;
}

TaskSystem &TaskSystem::operator=(TaskSystem &&source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _ready.clear();
  _completed.clear();
  _ready = std::move(source._ready);
  _completed = std::move(source._completed);

  // Invalidate the source data
  source._m = 1;
  source._t = 0;
  source._util = 0;
  source._dt = 0;

  return *this;
}

TaskSystem::~TaskSystem() {}

void TaskSystem::AddTask(Task::Parameters params) {
  auto task = std::make_unique<Task>(params);
  _util += task->Util();
  assert(_util <= _m);

  Task::Parameters p = task->Params();
  std::vector<time_t> v{p.C, p.D, p.T};
  _dt = std::reduce(v.begin(), v.end(), _dt,
                    [](const time_t &init, const time_t &first) {
                      return std::gcd(init, first);
                    });
  _ready.emplace_back(std::move(task));
}

void TaskSystem::Reset() {
  _t = 0;

  std::vector<std::shared_ptr<Task>> newReady;
  for (auto &tau : _completed) {
    tau->Reset();
    newReady.emplace_back(tau);
  }

  for (auto &tau : _ready) {
    tau->Reset();
    newReady.emplace_back(tau);
  }

  // Move new ready tasks
  _ready = std::move(newReady);
}

const States
TaskSystem::_GetStates(const std::vector<std::shared_ptr<Task>> _tasks) const {
  States states;
  std::transform(_tasks.begin(), _tasks.end(), std::back_inserter(states),
                 [](const std::shared_ptr<Task> &tau) {
                   return std::pair(tau->Params(), tau->Attrs());
                 });
  return states;
}

const States TaskSystem::operator()() const { return _GetStates(_ready); };

const States TaskSystem::Context() const { return _GetStates(_completed); }

const States TaskSystem::operator()(const std::vector<int> &indices) {
  std::vector<std::shared_ptr<Task>> newReady;

  // Completed tasks
  auto dt = _dt;
  _completed.erase(std::remove_if(_completed.begin(), _completed.end(),
                                  [&newReady, dt](auto &tau) {
                                    if (tau->Step(false, dt)) {
                                      newReady.emplace_back(tau);
                                      return true;
                                    }
                                    return false;
                                  }),
                   _completed.end());

  // Selected ready tasks
  for (const auto &index : indices) {
    auto tau = std::move(_ready[index]);
    if (tau->Step(true, _dt)) {
      newReady.emplace_back(tau);
    } else {
      _completed.emplace_back(tau);
    }
  }
  _ready.erase(std::remove_if(_ready.begin(), _ready.end(),
                              [](auto &tau) { return tau == nullptr; }),
               _ready.end());

  // New ready tasks
  for (auto &tau : _ready) {
    if (tau->Step(false, _dt)) {
      newReady.emplace_back(tau);
    }
  }

  // Move new ready tasks
  _ready = std::move(newReady);
  _t += _dt;
  return this->operator()();
}
