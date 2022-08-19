#include <AsyncProcessor.hpp>

AsyncProcessor::AsyncProcessor() : Processor() {}

AsyncProcessor::AsyncProcessor(AsyncProcessor &&source) {
  _id = source._id;
  _capacity = source._capacity;

  source._id = 0;
  source._capacity = 0;

  _thread = std::move(source._thread);
}

AsyncProcessor &AsyncProcessor::operator=(AsyncProcessor &&source) {
  if (this == &source) {
    return *this;
  }

  _id = source._id;
  _capacity = source._capacity;

  source._id = 0;
  source._capacity = 0;

  _thread = std::move(source._thread);
  return *this;
}

AsyncProcessor::~AsyncProcessor() { releaseThread(); }

void AsyncProcessor::keepThread(std::unique_ptr<std::thread> thread) {
  _thread = std::move(thread);
};

void AsyncProcessor::releaseThread() {
  if (_thread != nullptr) {
    _thread->join();
    _thread = nullptr;
  }
};
