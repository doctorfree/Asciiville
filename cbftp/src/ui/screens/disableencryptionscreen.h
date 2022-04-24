#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;

class DisableEncryptionScreen : public UIWindow {
public:
  DisableEncryptionScreen(Ui *);
  ~DisableEncryptionScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  bool active;
  bool mismatch;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
};
