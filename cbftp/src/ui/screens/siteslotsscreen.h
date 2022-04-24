#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Site;
class MenuSelectOptionElement;

class SiteSlotsScreen : public UIWindow {
public:
  SiteSlotsScreen(Ui *);
  ~SiteSlotsScreen();
  void initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  std::shared_ptr<Site> modsite;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
};
