#include <cmath>
#include <iostream>
#include <vector>

#include <TaskSystem.hpp>
#include <algorithms/PFair.hpp>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

int main() {
  TaskSystem system = TaskSystem(2);

  system.addTask(Task::Parameters{1, 2});
  system.addTask(Task::Parameters{3, 6});

  system.addTask(Task::Parameters{1, 3});
  system.addTask(Task::Parameters{2, 9});
  system.addTask(Task::Parameters{2, 9});
  // system.addTask(Task::Parameters{2, 9});

  TaskSystem systemSnapshot(std::move(system));
  system = std::move(systemSnapshot);
  assert(systemSnapshot.util() == 0.0);
  std::cout << "Total Util.: " << system.util() << std::endl;

  auto state = system.readyState();
  int m = system.M();
  time_t t = 0;
  for (int i = 0; i < 18000; i++) {
    if (i != 0 && i % 200 == 0) {
      //      system.reset();
      TaskSystem systemSnapshot(system);
      system = systemSnapshot;
      state = system.readyState();
    }

    t = system.T();
    auto indices = PFair::PF(t, m, state);
    assert(indices.size() <= m);

    state = system.operator()(indices);
    auto completed = system.completedState();
    std::cout << "Time done: " << system.T() << std::endl;

    // std::this_thread::sleep_for(50ms);
  }

  return 0;
}
