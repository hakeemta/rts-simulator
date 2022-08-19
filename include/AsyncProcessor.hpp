#ifndef ASYNC_HPP
#define ASYNC_HPP

#include <Processor.hpp>
#include <memory>
#include <thread>

class AsyncProcessor : public Processor {
public:
  AsyncProcessor();
  AsyncProcessor(const AsyncProcessor &source) = delete;
  AsyncProcessor &operator=(const AsyncProcessor &source) = delete;
  AsyncProcessor(AsyncProcessor &&source);
  AsyncProcessor &operator=(AsyncProcessor &&source);
  ~AsyncProcessor();

  void keepThread(std::unique_ptr<std::thread> thread);
  void releaseThread();

private:
  std::unique_ptr<std::thread> _thread;
};

#endif
