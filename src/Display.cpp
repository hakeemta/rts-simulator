#include <Display.hpp>

Display::Display(int numProcessors) : _numProcessors(numProcessors) {
  initscr();
  cbreak();
  start_color();

  for (int c = 1; c <= 7; c++) {
    init_pair(c, c, COLOR_BLACK);
  }

  for (int i = 0; i < _numProcessors; i++) {
    std::deque<int> q;
    _traces.emplace_back(q);
  }

  drawTraces();
  drawListings();
}

Display::~Display() { endwin(); }

void Display::updateStatus(std::string title) {
  mvprintw(1, 1, title.c_str());
  refresh();
}

void Display::drawTime() {
  wclear(_timeWin);
  for (int t = 0; t < _traceWidth + 1; t += 10) {
    auto offset = (_timeOffset % 10);
    mvwprintw(_timeWin, 0, (t - offset), "%d", (t + _timeOffset - offset));
  }
  wrefresh(_timeWin);
}

void Display::drawTraces() {
  mvprintw(2, 1, "Proc.");
  for (int i = 0; i < _numProcessors; i++) {
    int starty = 3 * (i + 1) + 1;
    mvprintw(starty, 3, "%d", i + 1);

    WINDOW *win = newwin(3, _traceWidth, starty - 1, 6);
    box(win, 0, 0);
    _traceWins.emplace_back(win);
  }

  int timeStarty = 3 * (_numProcessors + 1);
  mvprintw(timeStarty, 1, "Time");
  refresh();

  _timeWin = newwin(1, _traceWidth + 4, timeStarty, 6);
  drawTime();

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

  _readyWin = drawListing(listHeight, listWidth, listStarty, 0, "Ready",
                          "  TID\tC(t)\tD(t)\tL(t)\tU\tRels.");

  auto runningHeight = _numProcessors + 3;
  _runningWin = drawListing(runningHeight, listWidth, listStarty, listWidth + 2,
                            "Running", "  TID\tC(t)\tD(t)\tL(t)\tU\tRels.");
}

void Display::updateTrace(int index, int value) {
  std::lock_guard<std::mutex> lock(_mutex);
  auto &trace = _traces[index];
  trace.push_back(value);
  if (trace.size() > (_traceWidth - 2)) {
    trace.pop_front();
    _timeOffset += 1;
    drawTime();
  }

  auto win = _traceWins[index];
  for (int i = 0; i < trace.size(); i++) {
    auto color = COLOR_PAIR(trace[i]);
    wattron(win, color);
    mvwprintw(win, 1, (i + 1), "|");
    wattroff(win, color);
  }
  wrefresh(win);
}

void Display::updateList(ListingType type, int index, int value,
                         std::string state) {
  std::lock_guard<std::mutex> lock(_mutex);

  WINDOW *win = _readyWin;
  if (type == ListingType::RUNNING) {
    win = _runningWin;
  }

  auto color = COLOR_PAIR(value);
  wattron(win, color);
  mvwprintw(win, index, 1, "|");
  wattroff(win, color);

  mvwprintw(win, index, 3, state.c_str());
  wrefresh(win);
}

void Display::clearLists() {
  wclear(_readyWin);
  wrefresh(_readyWin);

  wclear(_runningWin);
  wrefresh(_runningWin);
}
