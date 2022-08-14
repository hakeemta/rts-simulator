#include <Timer.hpp>

void Timer::increment(time_t dt) {
  std::lock_guard<std::mutex> lock(_mutex);
  _value += dt;
  _condition.notify_all();
}

void Timer::synchronize(const time_t next) {
  std::unique_lock<std::mutex> lock(_mutex);
  _condition.wait(lock, [this, &next] { return _value == next; });
}
