#include <ctime>

enum class TaskStatus { IDLE, RUNNING, COMPLETED };

struct TaskParameters {
  time_t C;
  time_t D;
  time_t T;
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
  Task(time_t C, time_t D, time_t T);
  Task(TaskParameters params);
  double Util();
  bool Ready();
  void Reset();
  const TaskAttributes operator()() const;
  TaskStatus Step(bool selected, time_t delta);

private:
  time_t _t;
  long _releases;
  double _util{0.0};
  TaskParameters _params;
  TaskAttributes _attrs;
  TaskStatus _status{TaskStatus::IDLE};

  time_t _Laxity();
  void _Update();
};
