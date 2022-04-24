#pragma once

#include <list>
#include <map>
#include <memory>

#include "../menuselectoption.h"
#include "../uiwindow.h"

class SiteManager;
class RemoteCommandHandler;
class LocalStorage;
class MenuSelectOptionElement;
class MenuSelectOptionTextArrow;

class GlobalOptionsScreen : public UIWindow {
public:
  GlobalOptionsScreen(Ui *);
  ~GlobalOptionsScreen();
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  MenuSelectOption mso;
  RemoteCommandHandler * rch;
  SiteManager * sm;
  LocalStorage * ls;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  std::map<int, std::string> interfacemap;
  std::shared_ptr<MenuSelectOptionTextArrow> bindinterface;
};
