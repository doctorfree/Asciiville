#pragma once

#include <list>
#include <memory>
#include <utility>


#include "../uiwindow.h"
#include "../menuselectoption.h"

class KeyBinds;
class MenuSelectOptionElement;

class KeyBindsScreen : public UIWindow {
public:
  KeyBindsScreen(Ui *);
  ~KeyBindsScreen();
  void initialize(unsigned int row, unsigned int col, KeyBinds* keybinds);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void repopulate();
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  KeyBinds* realkeybinds;
  std::shared_ptr<KeyBinds> tempkeybinds;
  std::list<std::pair<int, int>> actionandscope;
  bool addextrakey;
};
