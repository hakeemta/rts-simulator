#ifndef TASK_SYSTEM_HPP
#define TASK_SYSTEM_HPP

#include "Task.hpp"
#include <vector>

class TaskSystem {
public:
  TaskSystem(){};
  TaskSystem(int m) : _m(m){};
  TaskSystem(const TaskSystem &source);
  TaskSystem &operator=(const TaskSystem &source);
  TaskSystem(TaskSystem &&source);
  TaskSystem &operator=(TaskSystem &&source);
  ~TaskSystem();

  double Util() const { return _util; };
  void Reset();
  void Commit();
  void AddTask(std::unique_ptr<Task> task);
  bool Run(const std::vector<int> &indices);

private:
  int _m{1};
  double _util{0.0};
  std::vector<std::shared_ptr<Task>> _tasks;
  time_t _t{0};
  time_t _dt{0};
};

#endif
