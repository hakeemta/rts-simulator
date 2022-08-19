#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <Resource.hpp>
#include <memory>
#include <thread>

class Processor : public Resource {
public:
  Processor() : Resource(++_idCount) {}

protected:
  int _capacity{1};

private:
  static int _idCount; // Global variable for counting processor object ids
};

#endif
