#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include <Processor.hpp>
#include <Task.hpp>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <vector>

using States = std::vector<std::pair<Task::Parameters, Task::Attributes>>;

// Auxiliary class to set and access global time in a thread-safe manner
class Timer {
public:
  void increment(time_t delta);
  void synchronize(const time_t previous = 0);

private:
  time_t _value{0};
  std::mutex _mutex;
  std::condition_variable _condition;
};

// Auxiliary class to manage entities in a thread-safe manner
template <class T> class Pool {
public:
  Pool(){};
  // Pool(const Pool<T> &source);
  // Pool<T> &operator=(const Pool<T> &source);
  // Pool(Pool<T> &&source);
  // Pool<T> &operator=(Pool<T> &&source);
  ~Pool(){};

  int size();
  void add(std::shared_ptr<T> entity);
  std::shared_ptr<T> extract(int index);
  std::shared_ptr<T> &operator[](int index);
  void refresh();

private:
  std::mutex _mutex;
  std::vector<std::shared_ptr<T>> _entities;

  // void copy(const Pool<T> &source);
};

class TaskSystem : public std::enable_shared_from_this<TaskSystem> {
public:
  TaskSystem();
  TaskSystem(int m);
  // TaskSystem(const TaskSystem &source);
  // TaskSystem &operator=(const TaskSystem &source);
  // TaskSystem(TaskSystem &&source);
  // TaskSystem &operator=(TaskSystem &&source);
  ~TaskSystem(){};

  void addTask(Task::Parameters params);
  double Util() const { return _util; };
  const time_t M() const { return _m; }
  const time_t T() const { return _t; }
  void reset();
  void syncTime(const time_t previous);
  const States operator()();
  const States Completed();
  const States operator()(const std::vector<int> &indices);

  void addToReady(std::shared_ptr<Task> task);
  void addToCompleted(std::shared_ptr<Task> task);
  void returnProcessor(std::shared_ptr<Processor> processor);
  void releaseThread(std::unique_ptr<std::thread> thread);

private:
  int _m{1};
  int _n{0};
  double _util{0.0};

  Pool<Processor> _processors;
  Pool<Task> _ready;
  Pool<Task> _running;
  Pool<Task> _completed;
  Pool<std::thread> _threads;
  // std::vector<std::unique_ptr<std::thread>> _threads;

  time_t _t{0};
  time_t _dt{0};
  Timer _timer;

  void invalidate();
  const States getStates(Pool<Task> &_tasks) const;
};

#endif
