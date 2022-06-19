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

  _readyTasks = source._readyTasks;
  _completedTasks = source._completedTasks;
}

TaskSystem &TaskSystem::operator=(const TaskSystem &source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _readyTasks.clear();
  _completedTasks.clear();
  _readyTasks = source._readyTasks;
  _completedTasks = source._completedTasks;

  return *this;
}

TaskSystem::TaskSystem(TaskSystem &&source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _readyTasks = std::move(source._readyTasks);
  _completedTasks = std::move(source._completedTasks);

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

  _readyTasks.clear();
  _completedTasks.clear();
  _readyTasks = std::move(source._readyTasks);
  _completedTasks = std::move(source._completedTasks);

  // Invalidate the source data
  source._m = 1;
  source._t = 0;
  source._util = 0;
  source._dt = 0;

  return *this;
}

TaskSystem::~TaskSystem() {}

void TaskSystem::Reset() {
  _t = 0;

  std::vector<std::shared_ptr<Task>> newReady;
  for (auto &tau : _completedTasks) {
    tau->Reset();
    newReady.emplace_back(tau);
  }

  for (auto &tau : _readyTasks) {
    tau->Reset();
    newReady.emplace_back(tau);
  }

  // Move new ready tasks
  _readyTasks = std::move(newReady);
}

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
  _readyTasks.emplace_back(std::move(task));
}

const std::vector<std::pair<Task::Parameters, Task::Attributes>>
TaskSystem::operator()() const {
  std::vector<std::pair<Task::Parameters, Task::Attributes>> state;
  std::transform(_readyTasks.begin(), _readyTasks.end(),
                 std::back_inserter(state),
                 [](const std::shared_ptr<Task> &tau) {
                   return std::pair(tau->Params(), tau->Attrs());
                 });
  return state;
};

const std::vector<std::pair<Task::Parameters, Task::Attributes>>
TaskSystem::operator()(const std::vector<int> &indices) {
  std::vector<std::shared_ptr<Task>> newReady;

  // Completed tasks
  auto dt = _dt;
  _completedTasks.erase(std::remove_if(_completedTasks.begin(),
                                       _completedTasks.end(),
                                       [&newReady, dt](auto &tau) {
                                         if (tau->Step(false, dt)) {
                                           newReady.emplace_back(tau);
                                           return true;
                                         }
                                         return false;
                                       }),
                        _completedTasks.end());

  // Selected ready tasks
  for (const auto &index : indices) {
    auto tau = std::move(_readyTasks[index]);
    if (tau->Step(true, _dt)) {
      newReady.emplace_back(tau);
    } else {
      _completedTasks.emplace_back(tau);
    }
  }
  _readyTasks.erase(std::remove_if(_readyTasks.begin(), _readyTasks.end(),
                                   [](auto &tau) { return tau == nullptr; }),
                    _readyTasks.end());

  // New ready tasks
  for (auto &tau : _readyTasks) {
    if (tau->Step(false, _dt)) {
      newReady.emplace_back(tau);
    }
  }

  // Move new ready tasks
  _readyTasks = std::move(newReady);
  _t += 1;
  return this->operator()();
}
