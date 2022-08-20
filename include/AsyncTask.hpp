#ifndef ASYNCTASK_HPP
#define ASYNCTASK_HPP

#include <AsyncProcessor.hpp>
#include <Display.hpp>
#include <Task.hpp>
#include <Timer.hpp>
#include <ctime>
#include <memory>
#include <mutex>

using AsyncProcessorPtr = std::unique_ptr<AsyncProcessor>;

class AsyncTask : public Task {
public:
  AsyncTask(Parameters params, std::shared_ptr<Timer> timer,
            std::shared_ptr<Display> display);
  AsyncTask(const AsyncTask &source) = delete;
  AsyncTask &operator=(const AsyncTask &source) = delete;
  AsyncTask(AsyncTask &&source);
  AsyncTask &operator=(AsyncTask &&source);
  ~AsyncTask(){};

  void allocateProcessor(AsyncProcessorPtr processor) {
    _asyncProcessor = std::move(processor);
  };
  AsyncProcessorPtr releaseProcessor() { return std::move(_asyncProcessor); };

  bool hasProcessor() override { return _asyncProcessor != nullptr; }
  void dispatch(time_t dt = 1) override;
  bool stepped(const time_t t) override;

private:
  AsyncProcessorPtr _asyncProcessor;
  bool _doneDispatched{false};
  std::shared_ptr<Timer> _timer;
  std::shared_ptr<Display> _display;

  void step(time_t dt);

  static std::mutex _mutex; // Shared by all tasks for protecting cout
};

#endif
