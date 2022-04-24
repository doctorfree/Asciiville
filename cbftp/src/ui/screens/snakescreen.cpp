#include "snakescreen.h"

#include <cstdlib>
#include <iterator>

#include "../termint.h"
#include "../ui.h"

#include "../../util.h"

#define CHAR_SNAKEPART 0x2588
#define CHAR_FRUIT '*'

using namespace Snake;

SnakeScreen::SnakeScreen(Ui* ui) : UIWindow(ui, "SnakeScreen"), direction(Direction::UP), lastmoveddirection(Direction::UP), score(0), xmax(0), ymax(0), state(State::RUNNING), timepassed(0) {
  keybinds.addBind('r', KEYACTION_RESET, "Reset");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Close");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Navigate left");
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Navigate right");
}

void SnakeScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  init(row, col);
}

void SnakeScreen::redraw() {
  vv->clear();
  if (state == State::RUNNING && (xmax != col || ymax != row)) {
    start();
  }
  for (unsigned int i = 0; i < grid.size(); ++i) {
    const Pixel & pixel = grid[i];
    if (pixel == Pixel::SNAKEPART) {
      vv->putChar(i / xmax, i % xmax, CHAR_SNAKEPART);
    }
    else if (pixel == Pixel::FRUIT) {
      vv->putChar(i / xmax, i % xmax, CHAR_FRUIT);
    }
  }
}

void SnakeScreen::update() {
  if (state != State::RUNNING) {
    return;
  }
  timepassed += 250;
  Pos target = getTarget(direction);
  if (target.x >= xmax || target.y >= ymax) {
    lose();
    ui->setInfo();
    return;
  }
  lastmoveddirection = direction;
  switch (getPixel(target.y, target.x)) {
    case Pixel::FRUIT:
      grow();
      ++score;
      if (snakepos.size() == grid.size()) {
        win();
      }
      else {
        placeFruit();
      }
      break;
    case Pixel::SNAKEPART:
      lose();
      break;
    case Pixel::EMPTY:
      getPixel(target.y, target.x) = Pixel::SNAKEPART;
      vv->putChar(target.y, target.x, CHAR_SNAKEPART);
      const Pos & tail = snakepos.front();
      getPixel(tail.y, tail.x) = Pixel::EMPTY;
      vv->putChar(tail.y, tail.x, ' ');
      snakepos.pop_front();
      snakepos.push_back(target);
      break;
  }
  ui->setInfo();
}

bool SnakeScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  switch (action) {
    case KEYACTION_UP:
      if (lastmoveddirection != Direction::DOWN) {
        direction = Direction::UP;
      }
      break;
    case KEYACTION_DOWN:
      if (lastmoveddirection != Direction::UP) {
        direction = Direction::DOWN;
      }
      break;
    case KEYACTION_LEFT:
      if (lastmoveddirection != Direction::RIGHT) {
        direction = Direction::LEFT;
      }
      break;
    case KEYACTION_RIGHT:
      if (lastmoveddirection != Direction::LEFT) {
        direction = Direction::RIGHT;
      }
      break;
    case KEYACTION_RESET:
      start();
      ui->redraw();
      break;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string SnakeScreen::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::string SnakeScreen::getInfoLabel() const {
  return "SNAKE";
}

std::string SnakeScreen::getInfoText() const {
  std::string out("SCORE: " + std::to_string(score) + " TIME: " + util::simpleTimeFormat(timepassed / 1000));
  if (state == State::WON) {
    out = "WINNER! " + out;
  }
  else if (state == State::LOST) {
    out = "YOU DIED! " + out;
  }
  return out;
}

void SnakeScreen::start() {
  score = 0;
  xmax = col;
  ymax = row;
  grid = std::vector<Pixel>(xmax * ymax, Pixel::EMPTY);
  snakepos.clear();
  timepassed = 0;
  state = State::RUNNING;
  unsigned int startx = rand() % xmax;
  unsigned int starty = rand() % ymax;
  direction = static_cast<Direction>(rand() % 4);
  lastmoveddirection = direction;
  switch (direction) {
    case Direction::UP:
      if (starty < ymax / 2) {
        direction = Direction::DOWN;
      }
      break;
    case Direction::DOWN:
      if (starty > ymax / 2) {
        direction = Direction::UP;
      }
      break;
    case Direction::LEFT:
      if (startx < xmax / 2) {
        direction = Direction::RIGHT;
      }
      break;
    case Direction::RIGHT:
      if (startx > xmax / 2) {
        direction = Direction::LEFT;
      }
      break;
  }
  snakepos.push_back({startx, starty});
  getPixel(starty, startx) = Pixel::SNAKEPART;
  grow();
  grow();
  for (unsigned int i = 0; i < 10; ++i) {
    placeFruit();
  }
}

void SnakeScreen::grow() {
  Pos newhead = getTarget(direction);
  snakepos.push_back(newhead);
  getPixel(newhead.y, newhead.x) = Pixel::SNAKEPART;
  vv->putChar(newhead.y, newhead.x, CHAR_SNAKEPART);
}

bool SnakeScreen::placeFruit() {
  for (int i = 0; i < 10; ++i) {
    int testpos = rand() % grid.size();
    if (grid[testpos] == Pixel::EMPTY) {
      grid[testpos] = Pixel::FRUIT;
      vv->putChar(testpos / xmax, testpos % xmax, CHAR_FRUIT);
      return true;
    }
  }
  std::list<unsigned int> emptypixels;
  for (unsigned int i = 0; i < grid.size(); ++i) {
    const Pixel & pixel = grid[i];
    if (pixel == Pixel::EMPTY) {
      emptypixels.push_back(i);
    }
  }
  if (emptypixels.empty()) {
    return false;
  }
  int listpos = rand() % emptypixels.size();
  std::list<unsigned int>::const_iterator emptypixel = emptypixels.begin();
  if (listpos) {
    std::advance(emptypixel, listpos - 1);
  }
  grid[*emptypixel] = Pixel::FRUIT;
  vv->putChar(*emptypixel / xmax, *emptypixel % xmax, CHAR_FRUIT);
  return true;
}

void SnakeScreen::lose() {
  state = State::LOST;
}

void SnakeScreen::win() {
  state = State::WON;
}

Pixel & SnakeScreen::getPixel(unsigned int y, unsigned int x) {
  return grid[y * xmax + x];
}

Pos SnakeScreen::getTarget(Direction direction) const {
  Pos target;
  const Pos & head = snakepos.back();
  switch (direction) {
    case Direction::UP:
      target = {head.x, head.y - 1};
      break;
    case Direction::DOWN:
      target = {head.x, head.y + 1};
      break;
    case Direction::LEFT:
      target = {head.x - 1, head.y};
      break;
    case Direction::RIGHT:
      target = {head.x + 1, head.y};
      break;
  }
  return target;
}
