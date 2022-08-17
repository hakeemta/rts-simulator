#include <Display.hpp>

Display::Display(int numProcessors, std::string title)
    : _numProcessors(numProcessors) {
  initscr();
  cbreak();
  start_color();

  for (int c = 1; c <= 7; c++) {
    init_pair(c, c, COLOR_BLACK);
  }

  mvprintw(1, 1, title.c_str());
  drawTraces();
  drawListings();
}

void Display::drawTraces() {
  mvprintw(2, 1, "Proc.");
  for (int i = 0; i < _numProcessors; i++) {
    int starty = 3 * (i + 1) + 1;
    mvprintw(starty, 3, "%d", i + 1);

    WINDOW *win = newwin(3, _traceWidth, starty - 1, 5);
    box(win, 0, 0);
    _traceWins.emplace_back(win);
  }

  int timeStarty = 3 * (_numProcessors + 1);
  mvprintw(timeStarty, 1, "Time");
  for (int t = 0; t < _traceWidth; t += 10) {
    mvprintw(timeStarty, t + 6, "%d", t);
  }
  refresh();

  for (auto win : _traceWins) {
    wrefresh(win);
  }
}

WINDOW *Display::drawListing(int height, int width, int starty, int startx,
                             std::string title, std::string heading) {
  auto frameWin = newwin(height, width, starty, startx);
  mvwprintw(frameWin, 0, 1, title.c_str());

  init_pair(100, COLOR_BLACK, COLOR_GREEN);
  wattron(frameWin, COLOR_PAIR(100));
  mvwprintw(frameWin, 1, 1, heading.c_str());
  wattroff(frameWin, COLOR_PAIR(100));

  wrefresh(frameWin);

  auto win = newwin((height - 2), width, (starty + 2), startx);
  return win;
}

void Display::drawListings() {
  int listStarty = 3 * (_numProcessors + 1) + 2;
  const int listHeight = 16, listWidth = 46;
  init_pair(1, COLOR_BLACK, COLOR_GREEN);

  _readyWin = drawListing(listHeight, listWidth, listStarty, 0, "Ready",
                          "  TID\tC(t)\tD(t)\tL(t)\tU\tRels.");

  auto runningHeight = _numProcessors + 3;
  _runningWin = drawListing(runningHeight, listWidth, listStarty, listWidth + 2,
                            "Running", "  TID\tC(t)\tD(t)\tL(t)\tU\tRels.");

  _completedWin = drawListing((listHeight - runningHeight), listWidth,
                              (listStarty + runningHeight), listWidth + 2,
                              "Completed", "  TID\tC(t)\tD(t)\tL(t)\tU\tRels.");
}

void Display::updateTrace(int index, int value) {
  ; //
}

Display::~Display() { endwin(); }