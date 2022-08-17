#include <Display.hpp>
#include <TaskSystem.hpp>
#include <algorithms/PFair.hpp>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <ncurses.h>
#include <sstream>
#include <thread>
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

  // char title[64];
  // sprintf(title, "Util:%.2f, #Tasks:%d, #Procs:%d, #Steps:%d, Filename:%s",
  //         system.util(), system.N(), m, L, filename.c_str());
  // Display display(m, title);

  auto state = system.readyState();
  time_t t = 0;
  for (int i = 0; i < L; i++) {
    if (i != 0 && i % 100 == 0) {
      // system.reset();
      TaskSystem systemSnapshot(std::move(system));
      system = std::move(systemSnapshot);
      state = system.readyState();
    }

    // wclear(readyWin);
    for (int i = 0; i < state.size(); i++) {
      auto [id, params, attrs] = state[i];

      // wattron(readyWin, COLOR_PAIR(id));
      // mvwprintw(readyWin, (i + 1), 1, "|");
      // wattroff(readyWin, COLOR_PAIR(id));

      // mvwprintw(readyWin, (i + 1), 3, "%d", id);
      // wrefresh(readyWin);
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

  getchar();
  endwin();
  return 0;
}
