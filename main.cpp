#include "./includes/Task.hpp"
#include <iostream>

int main() {
  Task task{2, 5, 5};
  std::cout << task.Util() << std::endl;
  return 0;
}
