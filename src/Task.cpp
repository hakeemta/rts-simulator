#include <Task.hpp>
#include <TaskSystem.hpp>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

std::mutex Task::_mutex;

Task::Task(Parameters params) : _params(params), _attrs(params) {
  /* Initializes a task and validates its utilization.
   */
  assert(_params.U <= 1.0);
  _id = _idCount++;
  reset();
}

Task::Task(const Task &source) {
  _id = source._id;
  _params = source._params;
  _attrs = source._attrs;
  _status = source._status;

  _t = source._t;
  _timer = source._timer;

  if (source._processor != nullptr) {
    _processor = std::make_unique<Processor>(*(source._processor));
  }
}

Task &Task::operator=(const Task &source) {
  if (this == &source) {
    return *this;
  }

  _id = source._id;
  _params = source._params;
  _attrs = source._attrs;
  _status = source._status;

  _t = source._t;
  _timer = source._timer;

  if (source._processor != nullptr) {
    _processor = std::make_unique<Processor>(*(source._processor));
  }

  return *this;
}

Task::Task(Task &&source) {
  _id = source._id;
  _params = source._params;
  _attrs = source._attrs;
  _status = source._status;

  _t = source._t;
  _timer = source._timer;

  if (source._processor != nullptr) {
    _processor = std::move(source._processor);
  }

  invalidate();
}

Task &Task::operator=(Task &&source) {
  if (this == &source) {
    return *this;
  }

  _id = source._id;
  _params = source._params;
  _attrs = source._attrs;
  _status = source._status;

  _t = source._t;
  _timer = source._timer;

  if (source._processor != nullptr) {
    _processor = std::move(source._processor);
  }

  invalidate();
  return *this;
}

Task::~Task() { releaseThread(); }

void Task::invalidate() {
  _id = 0;
  _params = Parameters();
  _attrs = Attributes();
  _status = Status::IDLE;

  _t = 0;
}

void Task::update(bool reload) {
  if (reload) {
    _attrs.Ct = _params.C;
    _attrs.Dt = _params.D;
  }

  _attrs.Lt = _attrs.Dt - _attrs.Ct;
  _attrs.Rt = _params.D - _attrs.Lt;
}

void Task::asyncStep() {
  std::unique_lock<std::mutex> lck(_mutex);
  std::cout << "[t=" << _t << "] Stepping " << id() << " on "
            << std::this_thread::get_id() << std::endl;
  lck.unlock();

  _timer->synchronize(_t);
  _doneDispatched = true;
}

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

void Task::releaseThread() {
  if (_thread != nullptr) {
    _thread->join();
    _thread = nullptr;
  }
}

void Task::dispatch(ProcessorPtr processor, time_t delta) {
  if (processor == nullptr) {
    if (_status == Status::RUNNING) {
      _status = Status::IDLE;
    }
  } else {
    _processor = std::move(processor);

    if (!ready()) {
      throw std::out_of_range("Task execution overrun!");
    }

    _attrs.Ct -= delta;
    _status = _attrs.Ct > 0 ? Status::RUNNING : Status::COMPLETED;
  }

  _t += delta;
  _attrs.Dt -= delta;
  update(false);

  if (_attrs.Lt < 0) {
    assert(_attrs.Rt > _params.D);
    throw std::out_of_range("Task deadline miss!");
  }

  _doneDispatched = false;
  _thread = std::make_unique<std::thread>(&Task::asyncStep, this);

  auto next_r = _params.O + (_attrs.releases * _params.T);
  if (_t >= next_r) {
    _attrs.releases += 1;
    reset(false);
  }
}
