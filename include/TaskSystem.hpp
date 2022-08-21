#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include <Display.hpp>
#include <Processor.hpp>
#include <Task.hpp>
#include <memory>
#include <tuple>
#include <vector>

using TaskState =
    std::vector<std::tuple<int, Task::Parameters, Task::Attributes>>;
using TaskPtr = std::unique_ptr<Task>;
using TaskSubSet = std::vector<TaskPtr>;

class TaskSystem {
public:
  TaskSystem(int m = 1, bool log = false);
  TaskSystem(const TaskSystem &source) = delete;
  TaskSystem &operator=(const TaskSystem &source) = delete;
  TaskSystem(TaskSystem &&source);
  TaskSystem &operator=(TaskSystem &&source);
  ~TaskSystem(){};

  const int M() const { return _m; }
  const int N() const { return _n; }
  double util() const { return _util; };
  const time_t T() const { return _t; }
  const time_t dt() const { return _quantumSize; };
  const time_t H() const { return _hyperperiod; };

  void addTask(Task::Parameters params);
  void loadTasks(std::string filename);
  void reset();
  TaskState readyState() { return getState(_readyTasks); };
  TaskState completedState() { return getState(_completedTasks); };
  time_t nextEventAt() const;
  TaskState operator()(const std::vector<int> &indices, time_t proportion = 1);
  std::string toString() const;

private:
  int _m{1};
  int _n{0};
  double _util{0.0};

  std::vector<ProcessorPtr> _processors;
  TaskSubSet _readyTasks;
  TaskSubSet _dispatchedTasks;
  TaskSubSet _completedTasks;

  time_t _t{0};
  time_t _quantumSize{0};
  time_t _hyperperiod{1};
  std::shared_ptr<Display> _display;

  void invalidate();
  TaskState getState(const TaskSubSet &tasks);
  void dispatchTasks(const std::vector<int> &indices, time_t dt = 1);
  void dispatchTasks(TaskSubSet &tasks, time_t dt = 1);
  void acquireResources(TaskPtr &task);
};

#endif
