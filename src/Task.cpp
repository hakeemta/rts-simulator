#include "../includes/Task.hpp"
#include <stdexcept>
#include <vector>

time_t Task::_Laxity() { return attrs.Dt - attrs.Ct; }

void Task::_Update() {
  attrs = {.Ct = params.C, .Dt = params.D, .Rt = params.C, .Lt = _Laxity()};
  status = TaskStatus::IDLE;
}

double Task::Util() {
  if (util == 0) {
    util = static_cast<double>(params.C) / params.D;
  }
  return util;
}

Task::Task(time_t C, time_t D, time_t T) : params{C, D, T} {}
Task::Task(TaskParameters params) : params(params) {}

bool Task::Ready() {
  return (status == TaskStatus::IDLE) || (status == TaskStatus::RUNNING);
}

void Task::Reset() {
  t = 0;
  releases = 0;
  _Update();
}

TaskStatus Task::Step(bool selected, time_t delta) {
  if (!selected) {
    if (status == TaskStatus::RUNNING) {
      status = TaskStatus::IDLE;
    }
  } else {
    if (!Ready()) {
      throw std::out_of_range("Task execution overrun");
    }

    attrs.Ct -= delta;
    status = attrs.Ct > 0 ? TaskStatus::RUNNING : TaskStatus::COMPLETED;
  }

  t += delta;
  attrs.Dt -= delta;
  attrs.Lt = _Laxity();

  if (status == TaskStatus::IDLE) {
    attrs.Rt += delta;
  }

  if (attrs.Rt > params.D) {
    throw std::out_of_range("Task deadline miss");
  }

  auto r = params.O + (releases * params.T);
  if (t >= (r + params.T)) {
    _Update();
    releases += 1;
  }

  return status;
}
