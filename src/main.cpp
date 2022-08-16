#include <TaskSystem.hpp>
#include <algorithms/PFair.hpp>
#include <chrono>
#include <cmath>
#include <functional>
#include <ncurses.h>
#include <sstream>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

WINDOW *drawListing(int height, int width, int starty, int startx,
                    std::string title, std::string heading) {
  init_pair(100, COLOR_BLACK, COLOR_GREEN);
  auto frameWin = newwin(height, width, starty, startx);
  mvwprintw(frameWin, 0, 1, title.c_str());
  wattron(frameWin, COLOR_PAIR(100));
  mvwprintw(frameWin, 1, 1, heading.c_str());
  wattroff(frameWin, COLOR_PAIR(100));
  wrefresh(frameWin);

  auto win = newwin(height - 2, width, starty + 2, startx);
  return win;
}

void createColors() {
  for (int c = 1; c <= 7; c++) {
    init_pair(c, c, COLOR_BLACK);
  }
}

int main(int argc, char **argv) {
  initscr();
  start_color();
  createColors();

  const int _m = 2;
  const int traceWidth = 102;
  WINDOW *wins[_m];

  mvprintw(1, 1, "Proc.");

  for (int i = 0; i < _m; i++) {
    int starty = 3 * (i + 1);
    mvprintw(starty, 3, "%d", i + 1);

    wins[i] = newwin(3, traceWidth, starty - 1, 5);
    box(wins[i], 0, 0);
  }

  int timeStarty = 3 * (_m + 1) - 1;
  mvprintw(timeStarty, 1, "Time");
  for (int t = 0; t < traceWidth; t += 10) {
    mvprintw(timeStarty, t + 6, "%d", t);
  }
  refresh();

  for (int i = 0; i < _m; i++) {
    wrefresh(wins[i]);
  }

  int listStarty = timeStarty + 2;
  const int listHeight = 16, listWidth = 46;
  init_pair(1, COLOR_BLACK, COLOR_GREEN);

  auto readyWin = drawListing(listHeight, listWidth, listStarty, 0, "Ready",
                              "  TID\tC(t)\tD(t)\tL(t)\tU\tRels.");

  auto runningHeight = _m + 3;
  auto runningWin =
      drawListing(runningHeight, listWidth, listStarty, listWidth + 2,
                  "Running", "  TID\tC(t)\tD(t)\tL(t)\tU\tRels.");

  auto completedWin = drawListing(
      (listHeight - runningHeight), listWidth, (listStarty + runningHeight),
      listWidth + 2, "Completed", "  TID\tC(t)\tD(t)\tL(t)\tU\tRels.");

  // getchar();
  // endwin();
  // return 0;

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

    wclear(readyWin);
    for (int i = 0; i < state.size(); i++) {
      auto [id, params, attrs] = state[i];

      wattron(readyWin, COLOR_PAIR(id));
      mvwprintw(readyWin, (i + 1), 1, "|");
      wattroff(readyWin, COLOR_PAIR(id));

      mvwprintw(readyWin, (i + 1), 3, "%d", id);
      wrefresh(readyWin);
    }

    t = system.T();
    auto indices = PFair::PF(t, m, state);
    assert(indices.size() <= m);

    state = system.operator()(indices);
    auto completed = system.completedState();
    // std::cout << "[t=" << system.T() << "] Dispatcher on main proc. ["
    //           << std::this_thread::get_id() << "]" << std::endl
    //           << std::endl;
  }

  getchar();
  endwin();
  return 0;
}
