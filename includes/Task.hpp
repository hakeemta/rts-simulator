#ifndef TASK_HPP
#define TASK_HPP

#include <ctime>

enum class TaskStatus { IDLE, RUNNING, COMPLETED };

struct TaskParameters {
  TaskParameters(time_t C, time_t T, time_t D = 0, time_t O = 0)
      : C(C), T(T), D(D), O(O) {
    if (D == 0) {
      this->D = T;
    }
  }
  time_t C;
  time_t T;
  time_t D;
  time_t O{0};
};

struct TaskAttributes {
  time_t Ct;
  time_t Dt;
  time_t Rt;
  time_t Lt;
};

class Task {
public:
  Task(TaskParameters params);
  double Util() const { return _util; };
  void Reset(bool start = true);
  const TaskParameters Parameters() const { return _params; };
  const TaskAttributes operator()() const { return _attrs; };
  bool Ready();
  bool Step(bool selected = false, time_t delta = 1);

private:
  time_t _t{0};
  long _releases{0};
  double _util{0.0};
  TaskParameters _params;
  TaskAttributes _attrs;
  TaskStatus _status{TaskStatus::IDLE};

  void _Update(bool reset = true);
};

#endif
