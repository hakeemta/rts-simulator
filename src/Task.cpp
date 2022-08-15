#include <Task.hpp>
#include <cassert>

Task::Task(Parameters params)
    : Resource(++_idCount), _params(params), _attrs(params) {
  /* Initializes a task and validates its utilization.
   */
  assert(_params.U <= 1.0);
  reset();
}

Task::Task(Task &&source) {
  _id = source._id;
  _params = source._params;
  _attrs = source._attrs;
  _status = source._status;
  _t = source._t;

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

  if (source._processor != nullptr) {
    _processor = std::move(source._processor);
  }

  invalidate();
  return *this;
}

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

void Task::dispatch(ProcessorPtr processor, time_t dt) {
  if (processor == nullptr) {
    if (_status == Status::RUNNING) {
      _status = Status::IDLE;
    }
  } else {
    _processor = std::move(processor);

    if (!ready()) {
      throw std::out_of_range("Task execution overrun!");
    }

    _attrs.Ct -= dt;
    _status = _attrs.Ct > 0 ? Status::RUNNING : Status::COMPLETED;
  }

  _t += dt;
  _attrs.Dt -= dt;
  update(false);

  if (_attrs.Lt < 0) {
    assert(_attrs.Rt > _params.D);
    throw std::out_of_range("Task deadline miss!");
  }

  auto next_r = _params.O + (_attrs.releases * _params.T);
  if (_t >= next_r) {
    _attrs.releases += 1;
    reset(false);
  }
}
