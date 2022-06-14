#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include "Task.hpp"
#include <vector>

struct Processor {
  int Capacity{1};
};

class TaskSystem {
public:
  TaskSystem();
  TaskSystem(std::vector<std::unique_ptr<Processor>> processors);
  TaskSystem(const TaskSystem &source);
  TaskSystem &operator=(const TaskSystem &source);
  TaskSystem(TaskSystem &&source);
  TaskSystem &operator=(TaskSystem &&source);
  ~TaskSystem();

  double Util() const { return _util; };
  void Reset();
  void Commit();
  void AddTask(TaskParameters params);
  const time_t T() const { return _t; }
  const time_t M() const { return _m; }
  const std::vector<std::pair<TaskParameters, TaskAttributes>>
  operator()() const;
  const std::vector<std::pair<TaskParameters, TaskAttributes>>
  operator()(const std::vector<int> &indices);

private:
  int _m{1};
  double _util{0.0};
  std::vector<std::unique_ptr<Processor>> _processors;
  std::vector<std::shared_ptr<Task>> _completedTasks;
  std::vector<std::shared_ptr<Task>> _readyTasks;

  time_t _t{0};
  time_t _dt{0};
};

#endif
