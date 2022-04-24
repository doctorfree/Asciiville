#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Engine;
class ScoreBoard;

class ScoreBoardScreen : public UIWindow {
public:
  ScoreBoardScreen(Ui *);
  ~ScoreBoardScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  Engine * engine;
  std::shared_ptr<ScoreBoard> scoreboard;
  MenuSelectOption table;
  unsigned int currentviewspan;
};
