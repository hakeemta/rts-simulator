#include <AsyncTask.hpp>
#include <iostream>
#include <thread>

AsyncTask::AsyncTask(Parameters params, std::shared_ptr<Timer> timer,
                     std::shared_ptr<Display> display)
    : _timer{timer}, _display(display), Task(params) {}

AsyncTask::AsyncTask(AsyncTask &&source)
    : _timer(std::move(source._timer)), Task(std::move(source)) {}

AsyncTask &AsyncTask::operator=(AsyncTask &&source) {
  if (this == &source) {
    return *this;
  }

  Task{std::move(source)};
  _timer = std::move(source._timer);
  return *this;
}

void AsyncTask::step(time_t dt) {
  for (time_t t = 0; t < dt; t++) {
    Task::dispatch();

    _display->updateTrace(_processor->id() - 1, id());
    if (status() == Status::RUNNING) {
      _display->updateList(Display::ListingType::RUNNING, _processor->id() - 1,
                           id(), (*this)());
    }

    // std::unique_lock<std::mutex> lck(_mutex);
    // std::cout << "[t=" << _t << "] Task " << id() << " on proc. "
    //           << _processor->id() << " [" << std::this_thread::get_id() <<
    //           "]"
    //           << std::endl;
    // lck.unlock();

    _timer->synchronize(_t);
  }

  _doneDispatched = true;
}

void AsyncTask::dispatch(time_t dt) {
  if (_processor == nullptr) {
    return Task::dispatch(dt);
  }

  _doneDispatched = false;
  auto thread = std::make_unique<std::thread>(&AsyncTask::step, this, dt);
  _processor->keepThread(std::move(thread));
}

bool AsyncTask::stepped(const time_t t) {
  if (_processor == nullptr) {
    return Task::stepped(t);
  }
  return _doneDispatched;
}
