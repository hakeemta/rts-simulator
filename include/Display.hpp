#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <deque>
#include <mutex>
#include <ncurses.h>
#include <string>
#include <vector>

class Display {
public:
  enum class ListingType { IDLE, RUNNING };

  Display(int numProcessors = 2);
  void updateStatus(std::string title = "");
  void updateTrace(int index, int value);
  void updateList(ListingType type, int index, int value, std::string state);
  void clearLists();
  ~Display();

private:
  int _numProcessors{2};
  const int _traceWidth = 100;
  std::vector<WINDOW *> _traceWins;
  WINDOW *_timeWin;
  WINDOW *_readyWin;
  WINDOW *_runningWin;

  std::mutex _mutex;
  time_t _timeOffset{0};
  std::vector<std::deque<int>> _traces;

  WINDOW *drawListing(int height, int width, int starty, int startx,
                      std::string title, std::string heading);
  void drawTime();
  void drawTraces();
  void drawListings();
};

#endif