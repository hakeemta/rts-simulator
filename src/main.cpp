#include "../includes/TaskSystem.hpp"
#include <iostream>
#include <vector>

int main() {
  Task task{TaskParameters{2, 3}};
  std::cout << task.Util() << std::endl;

  task.Step(true);
  task.Step();
  TaskAttributes attrs = task();

  TaskSystem system(2);
  system.AddTask(std::move(std::make_unique<Task>(TaskParameters{2, 4})));
  system.AddTask(std::move(std::make_unique<Task>(TaskParameters{3, 6})));

  TaskSystem systemSnapshot(std::move(system));
  system = std::move(systemSnapshot);

  std::cout << system.Util() << std::endl;

  return 0;
}
