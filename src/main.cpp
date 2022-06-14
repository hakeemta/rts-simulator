#include <cmath>
#include <iostream>
#include <vector>

#include "../includes/TaskSystem.hpp"
#include "../includes/algorithms/PFair.hpp"

int main() {
  std::vector<std::unique_ptr<Processor>> processors{};
  processors.emplace_back(std::make_unique<Processor>());
  TaskSystem system(std::move(processors));

  system.AddTask(TaskParameters{1, 2});
  system.AddTask(TaskParameters{3, 6});

  system.AddTask(TaskParameters{1, 3});
  system.AddTask(TaskParameters{2, 9});
  system.AddTask(TaskParameters{2, 9});

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
