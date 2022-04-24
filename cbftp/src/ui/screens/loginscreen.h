#pragma once

#include <vector>

#include "../textinputfield.h"
#include "../uiwindow.h"

class LoginScreen : public UIWindow {
public:
  LoginScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
private:
  std::string drawword;
  void randomizeDrawLocation();
  std::vector<std::vector<int> > background;
  std::string passphrase;
  TextInputField passfield;
  int pass_row;
  int pass_col;
  int drawx;
  int drawy;
  bool attempt;
};
