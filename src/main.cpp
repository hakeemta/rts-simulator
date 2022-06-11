#include "../includes/TaskSystem.hpp"
#include <iostream>
#include <vector>

int main() {
  Task task{TaskParameters{2, 3}};
  std::cout << task.Util() << std::endl;

  task.Step(true);
  task.Step();
  TaskAttributes attrs = task();

  TaskSystem system(1);
  system.AddTask(std::move(std::make_unique<Task>(TaskParameters{1, 2})));
  system.AddTask(std::move(std::make_unique<Task>(TaskParameters{3, 6})));

  TaskSystem systemSnapshot(std::move(system));
  system = std::move(systemSnapshot);

  system.Run({1});
  system.Run({0});

  std::cout << system.Util() << std::endl;

  return 0;
}
