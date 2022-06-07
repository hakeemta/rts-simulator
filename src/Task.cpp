#include "../includes/Task.hpp"
#include <stdexcept>
#include <vector>

time_t Task::_Laxity() { return _attrs.Dt - _attrs.Ct; }

void Task::_Update() {
  _attrs = {.Ct = _params.C, .Dt = _params.D, .Rt = _params.C, .Lt = _Laxity()};
  _status = TaskStatus::IDLE;
}

double Task::Util() {
  if (_util == 0) {
    _util = static_cast<double>(_params.C) / _params.D;
  }
  return _util;
}

Task::Task(time_t C, time_t D, time_t T) : _params{C, D, T} {}
Task::Task(TaskParameters params) : _params(params) {}

bool Task::Ready() {
  return (_status == TaskStatus::IDLE) || (_status == TaskStatus::RUNNING);
}

void Task::Reset() {
  _t = 0;
  _releases = 0;
  _Update();
}

TaskStatus Task::Step(bool selected, time_t delta) {
  if (!selected) {
    if (_status == TaskStatus::RUNNING) {
      _status = TaskStatus::IDLE;
    }
  } else {
    if (!Ready()) {
      throw std::out_of_range("Task execution overrun");
    }

    _attrs.Ct -= delta;
    _status = _attrs.Ct > 0 ? TaskStatus::RUNNING : TaskStatus::COMPLETED;
  }

  _t += delta;
  _attrs.Dt -= delta;
  _attrs.Lt = _Laxity();

  if (_status == TaskStatus::IDLE) {
    _attrs.Rt += delta;
  }

  if (_attrs.Rt > _params.D) {
    throw std::out_of_range("Task deadline miss");
  }

  auto r = _params.O + (_releases * _params.T);
  if (_t >= (r + _params.T)) {
    _Update();
    _releases += 1;
  }

  return _status;
}
