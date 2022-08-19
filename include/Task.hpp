#ifndef TASK_HPP
#define TASK_HPP

#include <Processor.hpp>
#include <Resource.hpp>
#include <ctime>
#include <memory>

using ProcessorPtr = std::unique_ptr<Processor>;

class Task : public Resource {
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
  Task(const Task &source) = delete;
  Task &operator=(const Task &source) = delete;
  Task(Task &&source);
  Task &operator=(Task &&source);
  ~Task(){};

  double util() const { return _params.U; };
  const Parameters &params() const { return _params; };
  const Attributes &attrs() const { return _attrs; };
  const Status status() const { return _status; };
  std::string operator()() const;

  void reset(bool start = true);
  bool ready();
  void allocateProcessor(ProcessorPtr processor) {
    _processor = std::move(processor);
  };
  ProcessorPtr releaseProcessor() { return std::move(_processor); };
  virtual void dispatch(time_t dt = 1);
  virtual bool stepped(const time_t t) { return _t == t; };

protected:
  time_t _t{0};

private:
  Parameters _params;
  Attributes _attrs;
  ProcessorPtr _processor;
  Status _status{Status::IDLE};

  void invalidate();
  void update(bool reload = true);
  virtual bool hasProcessor() { return _processor != nullptr; }

  static int _idCount; // Global variable for counting task object ids
};

#endif
