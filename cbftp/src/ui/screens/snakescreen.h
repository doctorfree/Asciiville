#pragma once

#include <list>
#include <string>
#include <vector>

#include "../uiwindow.h"

namespace Snake {

struct Pos {
  unsigned int x;
  unsigned int y;
};

enum class Direction {
  UP = 0,
  RIGHT = 1,
  DOWN = 2,
  LEFT = 3
};

enum class Pixel {
  EMPTY,
  SNAKEPART,
  FRUIT
};

enum class State {
  RUNNING,
  WON,
  LOST
};

}

class SnakeScreen : public UIWindow {
public:
  SnakeScreen(Ui *);
  void initialize(unsigned int row, unsigned int col);
  void redraw();
  void update();
  bool keyPressed(unsigned int ch);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  void start();
  void grow();
  bool placeFruit();
  void lose();
  void win();
  Snake::Pixel & getPixel(unsigned int y, unsigned int x);
  Snake::Pos getTarget(Snake::Direction direction) const;
  std::list<Snake::Pos> snakepos;
  std::vector<Snake::Pixel> grid;
  Snake::Direction direction;
  Snake::Direction lastmoveddirection;
  int score;
  unsigned int xmax;
  unsigned int ymax;
  Snake::State state;
  int timepassed;
};
