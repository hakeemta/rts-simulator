#ifndef ASYNCTASK_HPP
#define ASYNCTASK_HPP

#include <Task.hpp>
#include <Timer.hpp>
#include <ctime>
#include <memory>
#include <mutex>

class AsyncTask : public Task {
public:
  AsyncTask(Parameters params, std::shared_ptr<Timer> timer);
  AsyncTask(const AsyncTask &source) = delete;
  AsyncTask &operator=(const AsyncTask &source) = delete;
  AsyncTask(AsyncTask &&source);
  AsyncTask &operator=(AsyncTask &&source);
  ~AsyncTask(){};

  void dispatch(ProcessorPtr processor = nullptr, time_t dt = 1) override;
  bool stepped(const time_t t) override;

private:
  std::shared_ptr<Timer> _timer;
  bool _doneDispatched{false};

  void step();
  static std::mutex _mutex; // Shared by all tasks for protecting cout
};

#endif
