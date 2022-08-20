#include <AsyncTask.hpp>
#include <iostream>
#include <thread>

AsyncTask::AsyncTask(Parameters params, std::shared_ptr<Timer> timer,
                     std::shared_ptr<Display> display)
    : _timer{timer}, _display(display), Task(params) {}

AsyncTask::AsyncTask(AsyncTask &&source) : Task(std::move(source)) {
  _doneDispatched = source._doneDispatched;
  source._doneDispatched = false;

  if (source.hasProcessor()) {
    _asyncProcessor = std::move(source._asyncProcessor);
  }

  _timer = std::move(source._timer);
  _display = std::move(source._display);
}

AsyncTask &AsyncTask::operator=(AsyncTask &&source) {
  if (this == &source) {
    return *this;
  }

  _doneDispatched = source._doneDispatched;
  source._doneDispatched = false;

  Task{std::move(source)};
  if (source.hasProcessor()) {
    _asyncProcessor = std::move(source._asyncProcessor);
  }

  _timer = std::move(source._timer);
  _display = std::move(source._display);

  return *this;
}

void AsyncTask::step(time_t dt) {
  for (time_t t = 0; t < dt; t++) {
    Task::dispatch();

    _display->updateTrace(_asyncProcessor->id() - 1, id());
    _display->updateList(Display::ListingType::RUNNING,
                         _asyncProcessor->id() - 1, id(), (*this)());

    _timer->synchronize(_t);
  }

  _doneDispatched = true;
}

void AsyncTask::dispatch(time_t dt) {
  if (!hasProcessor()) {
    return Task::dispatch(dt);
  }

  _doneDispatched = false;
  auto thread = std::make_unique<std::thread>(&AsyncTask::step, this, dt);
  _asyncProcessor->keepThread(std::move(thread));
}

bool AsyncTask::stepped(const time_t t) {
  if (!hasProcessor()) {
    return Task::stepped(t);
  }
  return _doneDispatched;
}
