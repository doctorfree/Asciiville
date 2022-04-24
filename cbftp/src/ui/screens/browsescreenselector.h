#pragma once

#include <string>
#include <vector>
#include <utility>

#include "../menuselectoption.h"

#include "browsescreensub.h"

#define BROWSESCREENSELECTOR_HOME "$HOME$"

class Ui;
class VirtualView;
class BrowseScreenAction;
class KeyBinds;

class BrowseScreenSelector : public BrowseScreenSub {
public:
  BrowseScreenSelector(Ui* ui, KeyBinds& keybinds);
  ~BrowseScreenSelector();
  BrowseScreenType type() const override;
  void redraw(unsigned int, unsigned int, unsigned int) override;
  void update() override;
  BrowseScreenAction keyPressed(unsigned int) override;
  std::string getLegendText(int scope) const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  void setFocus(bool) override;
  BrowseScreenAction tryJumpSection(const std::string& section) override;
private:
  Ui* ui;
  VirtualView* vv;
  unsigned int row;
  unsigned int col;
  unsigned int coloffset;
  bool focus;
  MenuSelectOption table;
  std::vector<std::pair<std::string, std::string> > entries;
  unsigned int pointer;
  unsigned int currentviewspan;
  bool gotomode;
};
