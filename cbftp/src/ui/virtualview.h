#pragma once

#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "virtualviewelement.h"

#define COLOR_DEFAULT -1

class Renderer;

class VirtualView {
public:
  VirtualView(Renderer& renderer);
  void putStr(unsigned int row, unsigned int col, const std::string& str, bool highlight = false, int maxlen = -1, bool rightalign = false);
  void putStr(unsigned int row, unsigned int col, const std::basic_string<unsigned int>& str, bool highlight = false, int maxlen = -1, bool rightalign = false);
  void putChar(unsigned int row, unsigned int col, int c, bool highlight = false);
  void highlightOn(unsigned int row, unsigned int col, unsigned int len = 1);
  void highlightOff(unsigned int row, unsigned int col, unsigned int len = 1);
  void setColor(unsigned int row, unsigned int col, int fgcolor, int bgcolor = COLOR_DEFAULT, unsigned int len = 1);
  void setBold(unsigned int row, unsigned int col, bool bold, unsigned int len = 1);
  bool tryFocus(unsigned int row, unsigned int col, unsigned int len = 1);
  void resize(unsigned int row, unsigned int col);
  bool isClear(unsigned int row, unsigned int col, unsigned int len = 1) const;
  unsigned int getActualRealRows() const;
  unsigned int getActualRealCols() const;
  void moveCursor(unsigned int row, unsigned int col);
  bool goUp();
  bool goDown();
  bool goLeft();
  bool goRight();
  bool pageDown();
  bool pageUp();
  bool viewPosition(unsigned int row, unsigned int col, unsigned int length);
  void render();
  void reset();
  void clear();
private:
  bool horizontalSlider() const;
  bool verticalSlider() const;
  void expandIfNeeded(unsigned int row, unsigned int col);
  void virtualResize(unsigned int row, unsigned int col);
  void renderVerticalSlider();
  void renderHorizontalSlider();
  void setRedrawAll();
  void drawSliderChar(unsigned int row, unsigned int col, unsigned int c, bool highlight = false);
  Renderer& renderer;
  unsigned int realrows;
  unsigned int realcols;
  unsigned int virtualrows;
  unsigned int virtualcols;
  unsigned int currentviewrow;
  unsigned int currentviewcol;
  std::vector<std::vector<VirtualViewElement>> virtualchars;
  std::vector<std::vector<VirtualViewElement>> realchars;
  std::list<std::pair<unsigned int, unsigned int>> modifiedchars;
  std::set<std::pair<unsigned int, unsigned int>> renderedchars;
  bool redrawall;
  int currentcleariteration;
  int currentredrawiteration;
  unsigned int maxrenderedrow;
  unsigned int maxrenderedcol;
};
