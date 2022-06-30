#include <Task.hpp>
#include <TaskSystem.hpp>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>

// Init static variable
int Task::_idCount = 0;

Task::Task(Parameters params) : _params(params) {
  assert(_params.U <= 1.0);
  _id = _idCount++;
  reset();
}

Task::Task(const Task &source) {
  _id = source._id;
  _t = source._t;
  _params = source._params;
  _attrs = source._attrs;
  _status = source._status;
  if (source._processor != nullptr) {
    _processor = std::make_unique<Processor>(*(source._processor));
  }
}

Task::~Task() {}

void Task::reset(bool start) {
  if (start) {
    _t = 0;
    _attrs.releases = 1;
  }

  _status = Status::IDLE;
  update();
}

bool Task::ready() {
  return (_status == Status::IDLE) || (_status == Status::RUNNING);
}

void Task::update(bool reload) {
  if (reload) {
    _attrs.Ct = _params.C;
    _attrs.Dt = _params.D;
  }

  _attrs.Lt = _attrs.Dt - _attrs.Ct;
  _attrs.Rt = _params.D - _attrs.Lt;
}

void Task::allocate(std::unique_ptr<Processor> processor, time_t delta) {
  if (processor == nullptr) {
    if (_status == Status::RUNNING) {
      _status = Status::IDLE;
    }
  } else {
    _processor = std::move(processor);

    if (!ready()) {
      throw std::out_of_range("Task execution overrun");
    }

    _attrs.Ct -= delta;
    _status = _attrs.Ct > 0 ? Status::RUNNING : Status::COMPLETED;
  }

  _t += delta;
  _attrs.Dt -= delta;
  update(false);

  if (_attrs.Lt < 0) {
    assert(_attrs.Rt > _params.D);
    throw std::out_of_range("Task deadline miss");
  }

  auto next_r = _params.O + (_attrs.releases * _params.T);
  if (_t >= next_r) {
    _attrs.releases += 1;
    reset(false);
  }

  // std::cout << _id << " stepped for " << delta << std::endl;
}

std::unique_ptr<Processor> Task::releaseProcessor() {
  return std::move(_processor);
}

bool Task::step(const time_t t) { return _t == t; }
