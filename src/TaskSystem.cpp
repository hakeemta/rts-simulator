#include "../includes/TaskSystem.hpp"
#include <iostream>
#include <numeric>

TaskSystem::TaskSystem(const TaskSystem &source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _tasks = source._tasks;
}

TaskSystem &TaskSystem::operator=(const TaskSystem &source) {
  if (this == &source) {
    return *this;
  }

  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _tasks.clear();
  _tasks = source._tasks;

  return *this;
}

TaskSystem::TaskSystem(TaskSystem &&source) {
  _m = source._m;
  _t = source._t;
  _util = source._util;
  _dt = source._dt;

  _tasks = std::move(source._tasks);

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

  _tasks.clear();
  _tasks = std::move(source._tasks);

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
  for (auto &tau : _tasks) {
    tau->Reset();
  }
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
  _tasks.emplace_back(std::move(task));
}

bool TaskSystem::Run(const std::vector<int> &indices) {
  
  return true;
}
