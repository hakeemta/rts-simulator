#ifndef TASK_HPP
#define TASK_HPP

#include <ctime>

class Task {
public:
  enum class Status { IDLE, RUNNING, COMPLETED };

  struct Parameters {
    Parameters(time_t C, time_t T, time_t D = 0, time_t O = 0)
        : C(C), T(T), D(D), O(O), U{static_cast<double>(C) / T} {
      if (D == 0) {
        this->D = T;
      }
    }
    time_t C;
    time_t T;
    time_t D;
    time_t O{0};
    double U{0.0};
  };

  struct Attributes {
    time_t Ct;
    time_t Dt;
    time_t Rt;
    time_t Lt;
    time_t releases{1};
  };

  Task(Parameters params);
  double Util() const { return _params.U; };
  void Reset(bool start = true);
  const Parameters &Params() const { return _params; };
  const Attributes &Attrs() const { return _attrs; };

  bool Ready();
  bool Step(bool selected = false, time_t delta = 1);

private:
  time_t _t{0};
  Parameters _params;
  Attributes _attrs;
  Status _status{Status::IDLE};

  void _Update(bool reload = true);
};

#endif
