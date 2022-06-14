#include <cmath>
#include <iostream>
#include <vector>

#include "../includes/TaskSystem.hpp"
#include "../includes/algorithms/PFair.hpp"

int main() {
  TaskSystem system(1);
  system.AddTask(std::move(std::make_unique<Task>(TaskParameters{1, 2})));
  system.AddTask(std::move(std::make_unique<Task>(TaskParameters{3, 6})));
  // system.AddTask(std::move(std::make_unique<Task>(TaskParameters{1, 3})));
  // system.AddTask(std::move(std::make_unique<Task>(TaskParameters{2, 9})));
  // system.AddTask(std::move(std::make_unique<Task>(TaskParameters{2, 9})));

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
