#ifndef ASYNCTASK_HPP
#define ASYNCTASK_HPP

#include <Task.hpp>
#include <Timer.hpp>
#include <ctime>
#include <memory>
#include <mutex>

class AsyncTask : public Task {
public:
  AsyncTask(Parameters params);
  AsyncTask(const AsyncTask &source);
  AsyncTask &operator=(const AsyncTask &source);
  AsyncTask(AsyncTask &&source);
  AsyncTask &operator=(AsyncTask &&source);
  ~AsyncTask();

  void releaseThread();
  void linkTimer(std::shared_ptr<Timer> timer) { _timer = timer; };
  bool stepped(const time_t t) override { return _doneDispatched; }
  void dispatch(ProcessorPtr processor = nullptr, time_t dt = 1) override;

private:
  std::shared_ptr<Timer> _timer;
  bool _doneDispatched{false};
  std::unique_ptr<std::thread> _thread;

  void asyncStep();
  static std::mutex _mutex; // Shared by all tasks for protecting cout
};

#endif
