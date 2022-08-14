#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <Resource.hpp>

class Processor : public Resource {
public:
  Processor() : Resource(_idCount++) {}
  int capacity{1};

private:
  static int _idCount; // global variable for counting processor object ids
};

#endif
