#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;

class NewKeyScreen : public UIWindow {
public:
  NewKeyScreen(Ui *);
  ~NewKeyScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  bool active;
  bool mismatch;
  bool tooshort;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
};
