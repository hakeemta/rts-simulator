#include <AsyncTask.hpp>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

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

AsyncTask::~AsyncTask() { releaseThread(); }

void AsyncTask::asyncStep() {
  std::unique_lock<std::mutex> lck(_mutex);
  std::cout << "[t=" << _t << "] Stepping " << id() << " on "
            << std::this_thread::get_id() << std::endl;
  lck.unlock();

  _timer->synchronize(_t);
  _doneDispatched = true;
}

void AsyncTask::releaseThread() {
  if (_thread != nullptr) {
    _thread->join();
    _thread = nullptr;
  }
}

void AsyncTask::dispatch(ProcessorPtr processor, time_t dt) {
  Task::dispatch(std::move(processor), dt);

  _doneDispatched = false;
  _thread = std::make_unique<std::thread>(&AsyncTask::asyncStep, this);
}
