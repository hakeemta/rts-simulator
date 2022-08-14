#ifndef TASK_HPP
#define TASK_HPP

#include <Processor.hpp>
#include <ctime>
#include <memory>
#include <mutex>
#include <vector>

// Forward declaration to avoid cyclic includes
class TaskSystem;
class Timer;

using ProcessorPtr = std::unique_ptr<Processor>;

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
    Attributes(){};
    Attributes(Parameters params)
        : Ct(params.C), Dt(params.D), Lt(params.D - params.C), Rt(params.C) {}
    time_t Ct;
    time_t Dt;
    time_t Lt;
    time_t Rt;
    time_t releases{1};
  };

  Task(Parameters params);
  Task(const Task &source);
  Task &operator=(const Task &source);
  Task(Task &&source);
  Task &operator=(Task &&source);
  ~Task();

  int id() const { return _id; };
  double util() const { return _params.U; };
  const Parameters &params() const { return _params; };
  const Attributes &attrs() const { return _attrs; };

  void reset(bool start = true);
  bool ready();
  ProcessorPtr releaseProcessor() { return std::move(_processor); };
  void releaseThread();
  void linkTimer(std::shared_ptr<Timer> timer) { _timer = timer; };
  bool step(const time_t t) { return _t == t; };
  bool stepped() { return _doneDispatched; }
  void dispatch(ProcessorPtr processor = nullptr, time_t delta = 1);

private:
  time_t _t{0};
  Parameters _params;
  Attributes _attrs;
  Status _status{Status::IDLE};

  ProcessorPtr _processor;
  std::shared_ptr<Timer> _timer;
  bool _doneDispatched{false};
  std::unique_ptr<std::thread> _thread;

  void invalidate();
  void update(bool reload = true);
  void asyncStep();

  int _id;
  static int _idCount;      // Global variable for counting task object ids
  static std::mutex _mutex; // Shared by all tasks for protecting cout
};

#endif
