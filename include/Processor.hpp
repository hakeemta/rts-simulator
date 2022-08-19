#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <Resource.hpp>
#include <memory>

class Processor : public Resource {
public:
  Processor() : Resource(++_idCount){};

private:
  int _capacity{1};
  static int _idCount; // Global variable for counting processor object ids
};

#endif
