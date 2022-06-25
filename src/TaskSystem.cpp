#include <TaskSystem.hpp>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <thread>

void Timer::increment(time_t delta) {
  std::lock_guard<std::mutex> lock(_mutex);
  _value += delta;
  _condition.notify_all();
}

time_t Timer::synchronize(const time_t next) {
  std::unique_lock<std::mutex> lock(_mutex);
  _condition.wait(lock, [this, &next] { return _value == next; });
  return _value;
}

// template <class T> void Pool<T>::copy(const Pool<T> &source) {
//   std::lock_guard<std::mutex> lock(_mutex);
//   std::transform(source._entities.begin(), source._entities.end(),
//                  std::back_inserter(_entities), [](const auto &entity) {
//                    return std::make_unique<T>(*entity);
//                  });
// }

// template <class T> Pool<T>::Pool(const Pool<T> &source) { copy(source); }

// template <class T> Pool<T> &Pool<T>::operator=(const Pool<T> &source) {
//   std::lock_guard<std::mutex> lock(_mutex);
//   copy(source);
//   return *this;
// }

// template <class T> Pool<T>::Pool(Pool<T> &&source) {
//   _entities = std::move(source._entities);
// }

// template <class T> Pool<T> &Pool<T>::operator=(Pool<T> &&source) {
//   std::lock_guard<std::mutex> lock(_mutex);
//   _entities = std::move(source._entities);
//   return *this;
// }

template <class T> int Pool<T>::size() {
  // std::lock_guard<std::mutex> lock(_mutex);
  return _entities.size();
}

template <class T> void Pool<T>::add(std::shared_ptr<T> entity) {
  std::lock_guard<std::mutex> lock(_mutex);
  _entities.push_back(std::move(entity));
}

template <class T> std::shared_ptr<T> &Pool<T>::operator[](int index) {
  // std::lock_guard<std::mutex> lock(_mutex);
  return _entities[index];
}

template <class T> void Pool<T>::refresh() {
  // std::lock_guard<std::mutex> lock(_mutex);
  _entities.erase(
      std::remove_if(_entities.begin(), _entities.end(),
                     [](auto &entity) { return entity == nullptr; }),
      _entities.end());
}

TaskSystem::TaskSystem() { TaskSystem(1); };

TaskSystem::TaskSystem(int m) : _m(m) {
  for (int i = 0; i < m; i++) {
    _processors.add(std::make_unique<Processor>());
  }
};

// TaskSystem::TaskSystem(const TaskSystem &source) {
//   _m = source._m;
//   _t = source._t;
//   _util = source._util;
//   _dt = source._dt;

//   _ready = source._ready;
//   _running = source._running;
//   _completed = source._completed;

//   _processors = source._processors;
// }

// TaskSystem &TaskSystem::operator=(const TaskSystem &source) {
//   if (this == &source) {
//     return *this;
//   }

//   _m = source._m;
//   _t = source._t;
//   _util = source._util;
//   _dt = source._dt;

//   _ready = source._ready;
//   _running = source._running;
//   _completed = source._completed;

//   _processors = source._processors;

//   return *this;
// }

void TaskSystem::invalidate() {
  _m = 1;
  _t = 0;
  _util = 0;
  _dt = 0;
}

// TaskSystem::TaskSystem(TaskSystem &&source) {
//   _m = source._m;
//   _t = source._t;
//   _util = source._util;
//   _dt = source._dt;

//   _ready = std::move(source._ready);
//   _running = std::move(source._running);
//   _completed = std::move(source._completed);

//   _processors = std::move(source._processors);

//   // Invalidate the source data
//   source.invalidate();
// }

// TaskSystem &TaskSystem::operator=(TaskSystem &&source) {
//   if (this == &source) {
//     return *this;
//   }

//   _m = source._m;
//   _t = source._t;
//   _util = source._util;
//   _dt = source._dt;

//   _ready = std::move(source._ready);
//   _running = std::move(source._running);
//   _completed = std::move(source._completed);

//   _processors = std::move(source._processors);

//   // Invalidate the source data
//   source.invalidate();

//   return *this;
// }

void TaskSystem::addTask(Task::Parameters params) {
  auto task = std::make_unique<Task>(params);
  _util += task->util();
  _n += 1;
  assert(_util <= _m);

  Task::Parameters p = task->Params();
  std::vector<time_t> v{p.C, p.D, p.T};
  _dt = std::reduce(v.begin(), v.end(), _dt,
                    [](const time_t &init, const time_t &first) {
                      return std::gcd(init, first);
                    });
  
  task->linkSystem(shared_from_this());
  _ready.add(std::move(task));
}

void TaskSystem::reset() {
  _t = 0;

  for (int i = 0; i < _ready.size(); i++) {
    auto &task = _ready[i];
    task->reset();
  }

  for (int i = 0; i < _running.size(); i++) {
    auto &task = _running[i];
    task->reset();
    _ready.add(std::move(task));
  }
  _running.refresh();

  for (int i = 0; i < _completed.size(); i++) {
    auto &task = _completed[i];
    task->reset();
    _ready.add(std::move(task));
  }
  _completed.refresh();
}

void TaskSystem::syncTime(const time_t next) { _timer.synchronize(next); }

void TaskSystem::start() {
  for (int i = 0; i < _ready.size(); i++) {
    auto &task = _ready[i];
    task->simulate();
  }
}

const States TaskSystem::getStates(Pool<Task> &tasks) const {
  States states;

  for (int i = 0; i < tasks.size(); i++) {
    const auto &task = tasks[i];
    states.emplace_back(task->Params(), task->Attrs());
  }
  return states;
}

const States TaskSystem::operator()() { return getStates(_ready); };

const States TaskSystem::Completed() { return getStates(_completed); }

void TaskSystem::addToReady(std::shared_ptr<Task> task) {
  _ready.add(std::move(task));
}

void TaskSystem::addToCompleted(std::shared_ptr<Task> task) {
  _completed.add(std::move(task));
}

void TaskSystem::addToProcessors(std::shared_ptr<Processor> processor) {
  _processors.add(std::move(processor));
}

const States TaskSystem::operator()(const std::vector<int> &indices) {
  // Selected ready tasks
  for (int k = 0; k < indices.size(); k++) {
    const auto &index = indices[k];
    auto &task = _ready[index];
    auto &proc = _processors[k];

    std::cout << task->getId() << " SELECTED to run on " << proc->getId()
              << " for " << _dt << std::endl;

    task->allocate(std::move(proc), _dt);
    _running.add(std::move(task));
  }

  // Purge null (allocated) tasks with corresponding processors
  _processors.refresh();
  _ready.refresh();

  // Step unselected and previously completed tasks
  for (int i = 0; i < _ready.size(); i++) {
    _ready[i]->allocate(nullptr, _dt);
  }
  for (int i = 0; i < _completed.size(); i++) {
    _completed[i]->allocate(nullptr, _dt);
  }

  _t += _dt;
  // Sync time
  _timer.increment(_dt);
  for (int i = 0; i < _n; i++) {
    std::this_thread::yield(); // Hopefully wait for other ready/awaken threads
                               // to run first
  }

  return this->operator()();
}
