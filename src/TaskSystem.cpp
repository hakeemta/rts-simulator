#include "../includes/TaskSystem.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>

TaskSystem::TaskSystem(const TaskSystem &source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _ready_tasks = source._ready_tasks;
  _completed_tasks = source._completed_tasks;
}

TaskSystem &TaskSystem::operator=(const TaskSystem &source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _ready_tasks.clear();
  _completed_tasks.clear();
  _ready_tasks = source._ready_tasks;
  _completed_tasks = source._completed_tasks;

  return *this;
}

TaskSystem::TaskSystem(TaskSystem &&source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _ready_tasks = std::move(source._ready_tasks);
  _completed_tasks = std::move(source._completed_tasks);

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

  _ready_tasks.clear();
  _completed_tasks.clear();
  _ready_tasks = std::move(source._ready_tasks);
  _completed_tasks = std::move(source._completed_tasks);

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
  for (auto &tau : _completed_tasks) {
    tau->Reset();
    newReady.emplace_back(tau);
  }

  for (auto &tau : _ready_tasks) {
    tau->Reset();
    newReady.emplace_back(tau);
  }

  // Move new ready tasks
  _ready_tasks = std::move(newReady);
}

void TaskSystem::AddTask(std::unique_ptr<Task> task) {
  _util += task->Util();
  assert(_util <= _m);

  TaskParameters p = task->Parameters();
  std::vector<time_t> v{p.C, p.D, p.T};
  _dt = std::reduce(v.begin(), v.end(), _dt,
                    [](const time_t &init, const time_t &first) {
                      return std::gcd(init, first);
                    });
  _ready_tasks.emplace_back(std::move(task));
}

const std::vector<std::pair<TaskParameters, TaskAttributes>>
TaskSystem::operator()() const {
  std::vector<std::pair<TaskParameters, TaskAttributes>> state;
  std::transform(_ready_tasks.begin(), _ready_tasks.end(),
                 std::back_inserter(state),
                 [](const std::shared_ptr<Task> &tau) {
                   return std::pair(tau->Parameters(), tau->Attributes());
                 });
  return state;
};

const std::vector<std::pair<TaskParameters, TaskAttributes>>
TaskSystem::operator()(const std::vector<int> &indices) {
  std::vector<std::shared_ptr<Task>> newReady;

  // Completed tasks
  auto dt = _dt;
  _completed_tasks.erase(std::remove_if(_completed_tasks.begin(),
                                        _completed_tasks.end(),
                                        [&newReady, dt](auto &tau) {
                                          if (tau->Step(false, dt)) {
                                            newReady.emplace_back(tau);
                                            return true;
                                          }
                                          return false;
                                        }),
                         _completed_tasks.end());

  // Selected ready tasks
  for (const auto &index : indices) {
    auto tau = std::move(_ready_tasks[index]);
    if (tau->Step(true, _dt)) {
      newReady.emplace_back(tau);
    } else {
      _completed_tasks.emplace_back(tau);
    }
  }
  _ready_tasks.erase(std::remove_if(_ready_tasks.begin(), _ready_tasks.end(),
                                    [](auto &tau) { return tau == nullptr; }),
                     _ready_tasks.end());

  // New ready tasks
  for (auto &tau : _ready_tasks) {
    if (tau->Step(false, _dt)) {
      newReady.emplace_back(tau);
    }
  }

  // Move new ready tasks
  _ready_tasks = std::move(newReady);
  _t += 1;
  return this->operator()();
}
