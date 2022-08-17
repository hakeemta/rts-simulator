#ifndef TIMER_HPP
#define TIMER_HPP

#include <mutex>

class Timer {
  /* Auxiliary class to set and access global time in a thread-safe manner
   */
public:
  void increment(time_t dt = 1);
  void synchronize(const time_t next = 0);
  time_t get();
  void reset() { _value = 0; };

private:
  time_t _value{0};
  std::mutex _mutex;
  std::condition_variable _condition;
};

#endif