#include <Processor.hpp>

Processor::Processor() : Resource(++_idCount) {}

Processor::Processor(Processor &&source) {
  _id = source._id;
  source._id = 0;

  _thread = std::move(source._thread);
}

Processor &Processor::operator=(Processor &&source) {
  if (this == &source) {
    return *this;
  }

  _id = source._id;
  source._id = 0;

  _thread = std::move(source._thread);
  return *this;
}

Processor::~Processor() { releaseThread(); }

void Processor::run(std::unique_ptr<std::thread> thread) {
  _thread = std::move(thread);
};

void Processor::releaseThread() {
  if (_thread != nullptr) {
    _thread->join();
    _thread = nullptr;
  }
};
