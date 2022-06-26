#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include <Processor.hpp>
#include <Task.hpp>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <vector>

using States = std::vector<std::pair<Task::Parameters, Task::Attributes>>;

// Auxiliary class to manage entities

template<class T, template<class> class P>
class Pool {
public:
  Pool(){};

  Pool(const Pool<T, P> &source);
  Pool<T, P> &operator=(const Pool<T, P> &source);
  Pool(Pool<T, P> &&source);
  Pool<T, P> &operator=(Pool<T, P> &&source);
  ~Pool(){};

  int size() const;
  void add(P<T> entity);
  P<T> &operator[](int index);
  void refresh();

private:
  std::vector<P<T>> _entities;

  void copy(const Pool<T, P> &source);
};

class TaskSystem : public std::enable_shared_from_this<TaskSystem> {
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

  const States operator()();
  const States Completed();
  const States operator()(const std::vector<int> &indices);

  void addToReady(std::shared_ptr<Task> task);
  void addToCompleted(std::shared_ptr<Task> task);
  void returnProcessor(std::shared_ptr<Processor> processor);

private:
  int _m{1};
  int _n{0};
  double _util{0.0};

  Pool<Processor, std::shared_ptr> _processors;
  Pool<Task, std::shared_ptr> _ready;
  Pool<Task, std::shared_ptr> _running;
  Pool<Task, std::shared_ptr> _completed;

  time_t _t{0};
  time_t _dt{0};

  void invalidate();
  const States getStates(Pool<Task, std::shared_ptr> &_tasks) const;
};

#endif
