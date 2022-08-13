#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

class Processor {
public:
  Processor() { _id = _idCount++; }
  int capacity{1};
  int id() const { return _id; };

private:
  int _id;
  static int _idCount; // global variable for counting processor object ids
};

#endif
