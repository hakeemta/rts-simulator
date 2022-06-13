#include "../includes/Task.hpp"
#include <cassert>
#include <stdexcept>
#include <vector>

Task::Task(TaskParameters params) : _params(params) {
  assert(_params.U <= 1.0);
  Reset();
}

void Task::_Update(bool reload) {
  if (reload) {
    _attrs.Ct = _params.C;
    _attrs.Dt = _params.D;
  }

  _attrs.Lt = _attrs.Dt - _attrs.Ct;
  _attrs.Rt = _params.D - _attrs.Lt;
}

void Task::Reset(bool start) {
  if (start) {
    _t = 0;
    _attrs.releases = 1;
  }

  _status = TaskStatus::IDLE;
  _Update();
}

bool Task::Ready() {
  return (_status == TaskStatus::IDLE) || (_status == TaskStatus::RUNNING);
}

bool Task::Step(bool selected, time_t delta) {
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
  _Update(false);

  if (_attrs.Lt < 0) {
    assert(_attrs.Rt > _params.D);
    throw std::out_of_range("Task deadline miss");
  }

  auto next_r = _params.O + (_attrs.releases * _params.T);
  if (_t >= next_r) {
    _attrs.releases += 1;
    Reset(false);
  }

  return Ready();
}
