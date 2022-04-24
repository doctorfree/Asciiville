#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class FocusableArea;
class ProxyManager;
class MenuSelectOptionTextArrow;

class ProxyOptionsScreen : public UIWindow {
public:
  ProxyOptionsScreen(Ui *);
  ~ProxyOptionsScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  void command(const std::string &);
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  bool active;
  bool defaultset;
  bool defaultdataset;
  std::string deleteproxy;
  std::string editproxy;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  MenuSelectOption msop;
  ProxyManager * pm;
  std::shared_ptr<MenuSelectOptionTextArrow> useproxy;
  std::shared_ptr<MenuSelectOptionTextArrow> dataproxy;
};
