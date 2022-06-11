#include "../includes/Task.hpp"
#include <cassert>
#include <stdexcept>
#include <vector>

Task::Task(TaskParameters params) : _params(params) {
  _util = static_cast<double>(_params.C) / _params.D;
  assert(_util <= 1.0);
  Reset();
}

bool Task::_Ready() {
  return (_status == TaskStatus::IDLE) || (_status == TaskStatus::RUNNING);
}

void Task::_Update(bool reset) {
  if (reset) {
    _attrs = {.Ct = _params.C, .Dt = _params.D};
  }

  _attrs.Lt = _attrs.Dt - _attrs.Ct;
  _attrs.Rt = _params.D - _attrs.Lt;
}

void Task::Reset(bool start) {
  if (start) {
    _t = 0;
    _releases = 0;
  }

  _status = TaskStatus::IDLE;
  _Update();
}

TaskStatus Task::Step(bool selected, time_t delta) {
  if (!selected) {
    if (_status == TaskStatus::RUNNING) {
      _status = TaskStatus::IDLE;
    }
  } else {
    if (!_Ready()) {
      throw std::out_of_range("Task execution overrun");
    }

    _attrs.Ct -= delta;
    _status = _attrs.Ct > 0 ? TaskStatus::RUNNING : TaskStatus::COMPLETED;
  }

  _t += delta;
  _attrs.Dt -= delta;
  _Update(false);

  if (_attrs.Lt < 0) {
    assert(_attrs.Rt > _params.D);
    throw std::out_of_range("Task deadline miss");
  }

  auto r = _params.O + (_releases * _params.T);
  if (_t >= (r + _params.T)) {
    _releases += 1;
    Reset(false);
  }

  return _status;
}
