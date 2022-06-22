#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include "Task.hpp"
#include <vector>

using States = std::vector<std::pair<Task::Parameters, Task::Attributes>>;

struct Processor {
  int Capacity{1};
};

class TaskSystem {
public:
  TaskSystem();
  TaskSystem(int m);
  TaskSystem(const TaskSystem &source);
  TaskSystem &operator=(const TaskSystem &source);
  TaskSystem(TaskSystem &&source);
  TaskSystem &operator=(TaskSystem &&source);
  ~TaskSystem();

  void AddTask(Task::Parameters params);
  double Util() const { return _util; };
  const time_t M() const { return _m; }
  const time_t T() const { return _t; }
  void Reset();
  const States operator()() const;
  const States Context() const;
  const States operator()(const std::vector<int> &indices);

private:
  int _m{1};
  double _util{0.0};
  std::vector<std::unique_ptr<Processor>> _processors;
  std::vector<std::shared_ptr<Task>> _completed;
  std::vector<std::shared_ptr<Task>> _ready;

  time_t _t{0};
  time_t _dt{0};

  const States
  _GetStates(const std::vector<std::shared_ptr<Task>> _tasks) const;
};

#endif
