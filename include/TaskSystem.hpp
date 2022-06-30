#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include <Processor.hpp>
#include <Task.hpp>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

using State = std::vector<std::pair<Task::Parameters, Task::Attributes>>;

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

  std::vector<std::unique_ptr<Processor>> _processors;
  std::vector<std::unique_ptr<Task>> _ready;
  std::vector<std::unique_ptr<Task>> _running;
  std::vector<std::unique_ptr<Task>> _completed;

  time_t _t{0};
  time_t _dt{0};

  void invalidate();
  State getState(std::vector<std::unique_ptr<Task>> &_tasks);
};

#endif
