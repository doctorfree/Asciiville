#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Path;
class Ui;
class FocusableArea;
class MenuSelectOptionElement;
class ExternalScripts;

class ExternalScriptsScreen : public UIWindow {
public:
  ExternalScriptsScreen(Ui *);
  ~ExternalScriptsScreen();
  void initialize(unsigned int row, unsigned int col, ExternalScripts* externalscripts, const std::string& description);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void addScriptLine(int y, const std::string& name, const Path& path, int id = -1,
      const std::shared_ptr<MenuSelectAdjustableLine>& before = nullptr);
  bool keyUp() override;
  bool keyDown() override;
  int getCurrentScope() const;
  void recreateTable();
  void saveToTempScriptList();
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption base;
  MenuSelectOption table;
  FocusableArea* focusedarea;
  FocusableArea* defocusedarea;
  bool temphighlightline;
  std::shared_ptr<ExternalScripts> tempexternalscripts;
  ExternalScripts* externalscripts;
  std::list<std::string> description;
};
