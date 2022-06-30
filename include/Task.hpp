#ifndef TASK_HPP
#define TASK_HPP

#include <Processor.hpp>
#include <ctime>
#include <memory>
#include <vector>

// Forward declaration to avoid cyclic includes
class TaskSystem;

class Task {
public:
  enum class Status { IDLE, RUNNING, COMPLETED };

  struct Parameters {
    Parameters(){};
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

  Task(){};
  Task(Parameters params);
  Task(const Task &source);
  ~Task();
  int getId() const { return _id; };
  double util() const { return _params.U; };
  void reset(bool start = true);
  const Parameters &Params() const { return _params; };
  const Attributes &Attrs() const { return _attrs; };

  bool ready();
  void allocate(std::unique_ptr<Processor> processor = nullptr,
                time_t delta = 1);
  std::unique_ptr<Processor> releaseProcessor();
  bool step(const time_t t);

private:
  time_t _t{0};
  Parameters _params;
  Attributes _attrs;
  Status _status{Status::IDLE};

  std::unique_ptr<Processor> _processor;

  void update(bool reload = true);

  int _id;
  static int _idCount; // global variable for counting task object ids
};

#endif
