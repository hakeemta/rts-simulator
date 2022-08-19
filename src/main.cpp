#include <TaskSystem.hpp>
#include <algorithms/PFair.hpp>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <ncurses.h>
#include <sstream>
#include <vector>

using namespace std::chrono_literals;

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

  TaskSystem system = TaskSystem(m);
  system.loadTasks(filename);
  if (L == 0) {
    L = system.L();
  }

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
  }

  getchar();
  endwin();
  return 0;
}
