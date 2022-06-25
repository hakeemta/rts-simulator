#ifndef TASK_HPP
#define TASK_HPP

#include <Processor.hpp>
#include <ctime>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

// Forward declaration to avoid cyclic includes
class TaskSystem;

class Task : public std::enable_shared_from_this<Task> {
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
  ~Task();
  int getId() const { return _id; };
  double util() const { return _params.U; };
  void reset(bool start = true);
  const Parameters &Params() const { return _params; };
  const Attributes &Attrs() const { return _attrs; };

  bool ready();
  void allocate(std::shared_ptr<Processor> processor = nullptr,
                time_t delta = 1);
  void linkSystem(std::shared_ptr<TaskSystem> system) { _system = system; };
  void simulate();

  std::shared_ptr<Task> getShared() { return shared_from_this(); }

private:
  time_t _t{0};
  Parameters _params;
  Attributes _attrs;
  Status _status{Status::IDLE};

  time_t _delta{0};
  std::shared_ptr<Processor> _processor;
  std::shared_ptr<TaskSystem> _system;

  void step();
  void update(bool reload = true);

  static std::mutex _mutex; // Shared by all tasks for protecting cout
  int _id;
  static int _idCount; // global variable for counting task object ids
  std::unique_ptr<std::thread> _thread;
};

#endif
