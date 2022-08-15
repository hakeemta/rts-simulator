#include <AsyncTask.hpp>
#include <iostream>
#include <thread>

AsyncTask::AsyncTask(Parameters params) : Task(params) {}

AsyncTask::AsyncTask(const AsyncTask &source) : Task(source) {
  _timer = source._timer;
}

AsyncTask &AsyncTask::operator=(const AsyncTask &source) {
  if (this == &source) {
    return *this;
  }

  Task::operator=(source);
  _timer = source._timer;
  return *this;
}

AsyncTask::AsyncTask(AsyncTask &&source) : Task(source) {
  _timer = std::move(source._timer);
}

AsyncTask &AsyncTask::operator=(AsyncTask &&source) {
  if (this == &source) {
    return *this;
  }

  Task{source};
  _timer = std::move(source._timer);
  return *this;
}

void AsyncTask::step() {
  std::unique_lock<std::mutex> lck(_mutex);
  std::cout << "[t=" << _t << "] Task " << id() << " on proc. "
            << _processor->id() << " [" << std::this_thread::get_id() << "]"
            << std::endl;
  lck.unlock();

  _timer->synchronize(_t);
  _doneDispatched = true;
}

void AsyncTask::dispatch(ProcessorPtr processor, time_t dt) {
  Task::dispatch(std::move(processor), dt);

  _doneDispatched = false;
  if (_processor != nullptr) {
    auto thread = std::make_unique<std::thread>(&AsyncTask::step, this);
    _processor->run(std::move(thread));
  }
}

bool AsyncTask::stepped(const time_t t) {
  if (_processor == nullptr) {
    return Task::stepped(t);
  }
  return _doneDispatched;
}