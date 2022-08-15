#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <Resource.hpp>
#include <memory>
#include <thread>

class Processor : public Resource {
public:
  Processor();
  Processor(const Processor &source);
  Processor &operator=(const Processor &source);
  Processor(Processor &&source);
  Processor &operator=(Processor &&source);
  ~Processor();

  int capacity{1};
  void run(std::unique_ptr<std::thread> thread);
  void releaseThread();

private:
  std::unique_ptr<std::thread> _thread;
  static int _idCount; // Global variable for counting processor object ids
};

#endif
