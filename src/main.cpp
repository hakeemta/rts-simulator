#include <TaskSystem.hpp>
#include <algorithms/PFair.hpp>
#include <chrono>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

std::vector<std::tuple<time_t, time_t>> loadTaskset() {
  std::ifstream filestream("/Users/abdulhakeemabdulrahman/research/"
                           "rts-simulator-py/rts-simulator/taskset.example");
  if (!filestream.is_open()) {
    std::cout << "Failed to open file!" << std::endl;
    return {};
  }

  std::string line;
  std::getline(filestream, line);
  auto U = std::stod(line);

  std::getline(filestream, line);
  auto N = std::stoi(line);

  std::vector<std::tuple<time_t, time_t>> tasks;
  for (int i = 0; i < N; i++) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    time_t C, T;
    char sep;
    if (linestream >> C >> sep >> T) {
      tasks.emplace_back(C, T);
    }
  }

  return tasks;
}

int main() {
  auto tasks = loadTaskset();
  TaskSystem system = TaskSystem(2);

  for (auto &task : tasks) {
    auto [C, T] = task;
    system.addTask(Task::Parameters{C, T});
  }

  TaskSystem systemSnapshot(std::move(system));
  system = std::move(systemSnapshot);
  assert(systemSnapshot.util() == 0.0);
  std::cout << "Total Util.: " << system.util() << std::endl;

  auto state = system.readyState();
  int m = system.M();
  time_t t = 0;
  for (int i = 0; i < 400; i++) {
    if (i != 0 && i % 100 == 0) {
      // system.reset();
      TaskSystem systemSnapshot(std::move(system));
      system = std::move(systemSnapshot);
      state = system.readyState();
    }

    t = system.T();
    auto indices = PFair::PF(t, m, state);
    assert(indices.size() <= m);

    state = system.operator()(indices);
    auto completed = system.completedState();
    std::cout << "[t=" << system.T() << "] Dispatcher on main proc. ["
              << std::this_thread::get_id() << "]" << std::endl
              << std::endl;
  }

  return 0;
}
