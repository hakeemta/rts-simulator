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

std::vector<std::tuple<time_t, time_t>>
loadTaskset(const std::string &filename) {
  std::ifstream filestream(filename);
  if (!filestream.is_open()) {
    std::cout << "Failed to open file: " << filename << std::endl;
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

int main(int argc, char **argv) {
  std::string str;
  std::for_each(argv + 1, argv + argc,
                [&](const char *c_str) { str += std::string(c_str) + " "; });

  std::string filename;
  int m = 2, L = 0; // m is number of processors and L is number of steps
  if (!str.empty()) {
    std::istringstream strStream(str);
    strStream >> filename >> m >> L;
  }

  auto tasks = loadTaskset(filename);
  if (tasks.empty()) {
    return 0;
  }

  TaskSystem system = TaskSystem(m);
  for (auto &task : tasks) {
    auto [C, T] = task;
    system.addTask(Task::Parameters{C, T});
  }
  if (L == 0) {
    L = system.L();
  }
  std::cout << "Filename:" << filename << ", #Procs.:" << m << ", #Steps: " << L
            << std::endl;
  std::cout << "Total Util.: " << system.util() << std::endl << std::endl;

  auto state = system.readyState();
  time_t t = 0;
  for (int i = 0; i < L; i++) {
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
