#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionTextArrow;
class LocalStorage;

class FileViewerSettingsScreen : public UIWindow {
public:
  FileViewerSettingsScreen(Ui *);
  ~FileViewerSettingsScreen();
  void initialize(unsigned int, unsigned int);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  LocalStorage * ls;
};
