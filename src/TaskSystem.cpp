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

void TaskSystem::_DeepCopy(const Tasks &src, Tasks &dst) {
  std::transform(src.begin(), src.end(), std::back_inserter(dst),
                 [](const auto &tau) { return std::make_unique<Task>(*tau); });
}

TaskSystem::TaskSystem(const TaskSystem &source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _DeepCopy(source._ready, _ready);
  _DeepCopy(source._completed, _completed);
}

TaskSystem &TaskSystem::operator=(const TaskSystem &source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  // _ready.clear();
  // _completed.clear();
  _DeepCopy(source._ready, _ready);
  _DeepCopy(source._completed, _completed);

  return *this;
}

void TaskSystem::_Invalidate() {
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
  _completed = std::move(source._completed);

  // Invalidate the source data
  source._Invalidate();
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
  _completed = std::move(source._completed);

  // Invalidate the source data
  source._Invalidate();

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
  Tasks newReady = std::move(_completed);

  for (auto &tau : _ready) {
    newReady.emplace_back(std::move(tau));
  }

  for (auto &tau : newReady) {
    tau->Reset();
  }

  // Move new ready tasks
  _ready = std::move(newReady);
}

const States TaskSystem::_GetStates(const Tasks &_tasks) const {
  States states;
  std::transform(_tasks.begin(), _tasks.end(), std::back_inserter(states),
                 [](const std::unique_ptr<Task> &tau) {
                   return std::pair(tau->Params(), tau->Attrs());
                 });
  return states;
}

const States TaskSystem::operator()() const { return _GetStates(_ready); };

const States TaskSystem::Completed() const { return _GetStates(_completed); }

const States TaskSystem::operator()(const std::vector<int> &indices) {
  Tasks newReady;

  // Completed tasks
  auto dt = _dt;
  _completed.erase(std::remove_if(_completed.begin(), _completed.end(),
                                  [&newReady, dt](auto &tau) {
                                    if (tau->Step(false, dt)) {
                                      newReady.emplace_back(std::move(tau));
                                      return true;
                                    }
                                    return false;
                                  }),
                   _completed.end());

  // Selected ready tasks
  for (const auto &index : indices) {
    auto &tau = _ready[index];
    if (tau->Step(true, _dt)) {
      newReady.emplace_back(std::move(tau));
    } else {
      _completed.emplace_back(std::move(tau));
    }
  }
  _ready.erase(std::remove_if(_ready.begin(), _ready.end(),
                              [](auto &tau) { return tau == nullptr; }),
               _ready.end());

  // New ready tasks
  for (auto &tau : _ready) {
    if (tau->Step(false, _dt)) {
      newReady.emplace_back(std::move(tau));
    }
  }

  // Move new ready tasks
  _ready = std::move(newReady);
  _t += _dt;
  return this->operator()();
}
