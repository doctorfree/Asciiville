#pragma once

#include "../uiwindow.h"

enum class ConfirmationMode {
  INFO,
  NORMAL,
  STRONG
};

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(Ui *);
  void initialize(unsigned int row, unsigned int col, const std::string & message, ConfirmationMode mode);
  void redraw();
  bool keyPressed(unsigned int ch);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::string message;
  ConfirmationMode mode;
  int strongconfirmstep;
};
