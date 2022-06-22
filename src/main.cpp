#include <cmath>
#include <iostream>
#include <vector>

#include "TaskSystem.hpp"
#include "algorithms/PFair.hpp"

int main() {
  TaskSystem system(2);

  system.AddTask(Task::Parameters{1, 2});
  system.AddTask(Task::Parameters{3, 6});

  system.AddTask(Task::Parameters{1, 3});
  system.AddTask(Task::Parameters{2, 9});
  system.AddTask(Task::Parameters{2, 9});
  system.AddTask(Task::Parameters{2, 9});

  TaskSystem systemSnapshot(std::move(system));
  system = std::move(systemSnapshot);

  std::cout << "Total Util.: " << system.Util() << std::endl;

  auto states = system();
  int m = system.M();
  time_t t = 0;
  for (int i = 0; i < 900; i++) {
    t = system.T();
    auto indices = PFair::PF(t, m, states);
    assert(indices.size() <= m);

    states = system(indices);
  }

  return 0;
}
