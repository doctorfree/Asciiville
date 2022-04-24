#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../transferpairing.h"

class Ui;
class MenuSelectAdjustableLine;
class MenuSelectOptionTextField;
class MenuSelectOptionTextArrow;
class MenuSelectOptionElement;

class TransferPairingScreen : public UIWindow {
public:
  TransferPairingScreen(Ui* ui);
  ~TransferPairingScreen();
  void initialize(unsigned int row, unsigned int col, TransferPairing* transferpairing);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  void initialize();
  //void addPatternLine(int y);
  TransferPairing* transferpairing;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption table;
  unsigned int currentviewspan;
  bool temphighlightline;
};
