#pragma once

#define SHORTESTKEY 4

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;

class ChangeKeyScreen : public UIWindow {
public:
  ChangeKeyScreen(Ui *);
  ~ChangeKeyScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  bool active;
  bool mismatch;
  bool oldmismatch;
  bool tooshort;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::string operation;
};
