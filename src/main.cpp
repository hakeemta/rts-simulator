#include <cmath>
#include <iostream>
#include <vector>

#include "TaskSystem.hpp"
#include "algorithms/PFair.hpp"

// Init static variable
int Processor::_idCount = 0;

int main() {
  TaskSystem system(2);

  system.addTask(Task::Parameters{1, 2});
  system.addTask(Task::Parameters{3, 6});

  system.addTask(Task::Parameters{1, 3});
  system.addTask(Task::Parameters{2, 9});
  system.addTask(Task::Parameters{2, 9});
  system.addTask(Task::Parameters{2, 9});

  // TaskSystem systemSnapshot(std::move(system));
  // system = std::move(systemSnapshot);

  std::cout << "Total Util.: " << system.Util() << std::endl;

  system.start();
  auto states = system();
  int m = system.M();
  time_t t = 0;
  for (int i = 0; i < 1800; i++) {
    if (i != 0 && i % 200 == 0) {
      system.reset();
      states = system();
    }

    t = system.T();
    auto indices = PFair::PF(t, m, states);
    assert(indices.size() <= m);

    states = system(indices);
  }

  return 0;
}
