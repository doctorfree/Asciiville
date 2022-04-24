#pragma once

#include "uicommand.h"
#include "ncurseswrap.h"

#include "../core/threading.h"
#include "../core/blockingqueue.h"
#include "../core/semaphore.h"

enum class Window {
  MAIN,
  INFO,
  LEGEND
};

class Renderer {
public:
  Renderer();
  bool init();
  void run();
  unsigned int getCol() const;
  unsigned int getRow() const;
  void resize(bool infoenabled, bool legendenabled);
  void adjust(bool infoenabled, bool legendenabled);
  void refresh(bool infoenabled, bool legendenabled);
  void bell();
  void hideCursor();
  void showCursor();
  void moveCursor(unsigned int row, unsigned int col);
  void erase(Window window = Window::MAIN);
  void kill();
  unsigned int read();
  void highlight(bool highlight, Window window = Window::MAIN);
  void bold(bool bold, Window window = Window::MAIN);
  void color(bool enabled, int color, Window window = Window::MAIN);
  void printStr(unsigned int row, unsigned int col, const std::string & str, bool highlight = false, bool bold = false, int color = -1, int maxlen = -1, bool rightalign = false, Window window = Window::MAIN);
  void printStr(unsigned int row, unsigned int col, const std::basic_string<unsigned int> & str, bool highlight = false, bool bold = false, int color = -1, int maxlen = -1, bool rightalign = false, Window window = Window::MAIN);
  void printChar(unsigned int row, unsigned int col, unsigned int c, bool highlight = false, bool bold = false, int color = -1, Window window = Window::MAIN);
  void setDefaultColors(int fgcolor, int bgcolor);

private:
  void initIntern();
  void resizeIntern(bool infoenabled, bool legendenabled);
  void adjustIntern(bool infoenabled, bool legendenabled);
  void refreshIntern(bool infoenabled, bool legendenabled);
  WINDOW* getWindow(Window window);
  Core::Thread<Renderer> thread;
  Core::BlockingQueue<UICommand> uiqueue;
  Core::Semaphore eventcomplete;
  WINDOW* main;
  WINDOW* info;
  WINDOW* legend;
  unsigned int col;
  unsigned int row;
  bool initret;
  bool dead;
};
