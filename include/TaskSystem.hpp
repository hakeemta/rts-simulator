#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include <Processor.hpp>
#include <Task.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <tuple>
#include <vector>

using TaskState =
    std::vector<std::tuple<int, Task::Parameters, Task::Attributes>>;
using TaskPtr = std::unique_ptr<Task>;
using TaskSubSet = std::vector<TaskPtr>;

// Auxiliary class to set and access global time in a thread-safe manner
class Timer {
public:
  void increment(time_t dt);
  void synchronize(const time_t next = 0);
  void reset() { _value = 0; };

private:
  time_t _value{0};
  std::mutex _mutex;
  std::condition_variable _condition;
};

class TaskSystem {
public:
  TaskSystem(int m = 1);
  TaskSystem(const TaskSystem &source);
  TaskSystem &operator=(const TaskSystem &source);
  TaskSystem(TaskSystem &&source);
  TaskSystem &operator=(TaskSystem &&source);
  ~TaskSystem(){};

  const time_t M() const { return _m; }
  const time_t N() const { return _n; }
  double util() const { return _util; };
  const time_t T() const { return _t; }
  const time_t dt() const { return _dt; };
  const time_t L() const { return _L; };

  void addTask(Task::Parameters params);
  void reset();
  TaskState readyState() { return getState(_ready); };
  TaskState completedState() { return getState(_completed); };
  TaskState operator()(const std::vector<int> &indices, time_t proportion = 1);

private:
  int _m{1};
  int _n{0};
  double _util{0.0};

  std::vector<ProcessorPtr> _processors;
  TaskSubSet _ready;
  TaskSubSet _dispatched;
  TaskSubSet _completed;

  time_t _t{0};
  time_t _dt{0};
  time_t _L{1};
  std::shared_ptr<Timer> _timer;

  void invalidate();
  TaskState getState(const TaskSubSet &tasks);
  void runTasks(const std::vector<int> &indices, time_t dt = 1);
  void idleTasks(TaskSubSet &tasks, time_t dt = 1);
  void acquireResources(TaskPtr &task);
};

#endif
