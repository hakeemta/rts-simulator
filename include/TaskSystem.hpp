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
using TaskSubSet = std::vector<std::unique_ptr<Task>>;

class TaskSystem {
public:
  TaskSystem(int m = 1);
  TaskSystem(const TaskSystem &source);
  TaskSystem &operator=(const TaskSystem &source);
  TaskSystem(TaskSystem &&source);
  TaskSystem &operator=(TaskSystem &&source);
  ~TaskSystem(){};

  void addTask(Task::Parameters params);
  const time_t M() const { return _m; }
  const time_t N() const { return _n; }
  double util() const { return _util; };
  const time_t T() const { return _t; }
  const time_t dt() const { return _dt; };
  const time_t L() const { return _L; };
  void reset();

  TaskState readyState() { return getState(_ready); };
  TaskState completedState() { return getState(_completed); };
  TaskState operator()(const std::vector<int> &indices, time_t proportion = 1);

private:
  int _m{1};
  int _n{0};
  double _util{0.0};

  std::vector<std::unique_ptr<Processor>> _processors;
  TaskSubSet _ready;
  TaskSubSet _dispatched;
  TaskSubSet _completed;

  time_t _t{0};
  time_t _dt{0};
  time_t _L{1};

  void invalidate();
  void runTasks(const std::vector<int> &indices, time_t dt = 1);
  void idleTasks(TaskSubSet &tasks, time_t dt = 1);
  void acquireResources(std::unique_ptr<Task> &task);
  TaskState getState(const TaskSubSet &tasks);
};

#endif
