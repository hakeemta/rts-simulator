#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include <AsyncTask.hpp>
#include <Processor.hpp>
#include <Timer.hpp>
#include <memory>
#include <tuple>
#include <vector>

using TaskState =
    std::vector<std::tuple<int, AsyncTask::Parameters, AsyncTask::Attributes>>;
using TaskPtr = std::unique_ptr<AsyncTask>;
using TaskSubSet = std::vector<TaskPtr>;

class TaskSystem {
public:
  TaskSystem(int m = 1);
  TaskSystem(const TaskSystem &source) = delete;
  TaskSystem &operator=(const TaskSystem &source) = delete;
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
  TaskState readyState() { return getState(_readyTasks); };
  TaskState completedState() { return getState(_completedTasks); };
  TaskState operator()(const std::vector<int> &indices, time_t proportion = 1);

private:
  int _m{1};
  int _n{0};
  double _util{0.0};

  std::vector<ProcessorPtr> _processors;
  TaskSubSet _readyTasks;
  TaskSubSet _dispatchedTasks;
  TaskSubSet _completedTasks;

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
