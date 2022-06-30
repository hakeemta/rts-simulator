#include <cmath>
#include <iostream>
#include <vector>

#include <TaskSystem.hpp>
#include <algorithms/PFair.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

// Init static variable
int Processor::_idCount = 0;

int main() {
  TaskSystem system = TaskSystem(2);

  system.addTask(Task::Parameters{1, 2});
  system.addTask(Task::Parameters{3, 6});

  system.addTask(Task::Parameters{1, 3});
  system.addTask(Task::Parameters{2, 9});
  system.addTask(Task::Parameters{2, 9});
  // system.addTask(Task::Parameters{2, 9});

  // // TaskSystem systemSnapshot(std::move(system));
  // // system = std::move(systemSnapshot);

  std::cout << "Total Util.: " << system.Util() << std::endl;

  auto state = system.operator()();
  int m = system.M();
  time_t t = 0;
  for (int i = 0; i < 18000; i++) {
    if (i != 0 && i % 200 == 0) {
      //      system.reset();
      TaskSystem systemSnapshot(system);
      system = systemSnapshot;
      state = system.operator()();
    }

    t = system.T();
    auto indices = PFair::PF(t, m, state);
    assert(indices.size() <= m);

    state = system.operator()(indices);
    auto completed = system.Completed();
    std::cout << "Time done: " << system.T() << std::endl;

    std::this_thread::sleep_for(50ms);
  }

  return 0;
}
