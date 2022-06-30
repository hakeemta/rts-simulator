#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include <Processor.hpp>
#include <Task.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

using State = std::vector<std::pair<Task::Parameters, Task::Attributes>>;

// Auxiliary class to manage entities
template <class T> class Pool {
public:
  Pool(){};

  Pool(const Pool<T> &source);
  Pool<T> &operator=(const Pool<T> &source);
  Pool(Pool<T> &&source);
  Pool<T> &operator=(Pool<T> &&source);
  ~Pool(){};

  int size() const;
  void add(std::unique_ptr<T> entity);
  std::unique_ptr<T> &operator[](int index);
  void refresh();

private:
  std::vector<std::unique_ptr<T>> _entities;

  void copy(const Pool<T> &source);
};

class TaskSystem {
public:
  TaskSystem();
  TaskSystem(int m);
  TaskSystem(const TaskSystem &source);
  TaskSystem &operator=(const TaskSystem &source);
  TaskSystem(TaskSystem &&source);
  TaskSystem &operator=(TaskSystem &&source);
  ~TaskSystem(){};

  void addTask(Task::Parameters params);
  double Util() const { return _util; };
  const time_t M() const { return _m; }
  const time_t T() const { return _t; }
  void reset();

  State operator()();
  State Completed();
  State operator()(const std::vector<int> &indices);

private:
  int _m{1};
  int _n{0};
  double _util{0.0};

  Pool<Processor> _processors;
  Pool<Task> _ready;
  Pool<Task> _running;
  Pool<Task> _completed;

  time_t _t{0};
  time_t _dt{0};

  void invalidate();
  State getState(Pool<Task> &_tasks) const;
};

#endif
