#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <deque>
#include <ncurses.h>
#include <string>
#include <vector>

class Display {
  enum class ListingType { IDLE, RUNNING, COMPLETED };

public:
  Display(int numProcessors = 2, std::string title = "");
  void updateTrace(int index, int value);
  void updateList(ListingType type);
  ~Display();

private:
  int _numProcessors{2};
  const int _traceWidth = 102;
  std::vector<WINDOW *> _traceWins;
  WINDOW *_readyWin;
  WINDOW *_runningWin;
  WINDOW *_completedWin;

  std::vector<std::deque<int>> _traces;

  WINDOW *drawListing(int height, int width, int starty, int startx,
                      std::string title, std::string heading);
  void drawTraces();
  void drawListings();
};

#endif