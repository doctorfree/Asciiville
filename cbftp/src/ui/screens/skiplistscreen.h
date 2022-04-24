#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../skiplist.h"

class SkipList;
class Ui;
class FocusableArea;
class MenuSelectAdjustableLine;
class MenuSelectOptionTextField;
class MenuSelectOptionTextArrow;
class MenuSelectOptionElement;

class SkipListScreen : public UIWindow {
public:
  SkipListScreen(Ui *);
  ~SkipListScreen();
  void initialize(unsigned int row, unsigned int col, SkipList * skiplist);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void initialize();
  void saveToTempSkipList();
  void addPatternLine(int y, bool regex, std::string pattern, bool file, bool dir,
                      int scope, SkipListAction action,
                      std::shared_ptr<MenuSelectAdjustableLine> before = nullptr);
  bool keyUp() override;
  bool keyDown() override;
  int getCurrentScope() const;
  void recreateTable();
  SkipList * skiplist;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption base;
  MenuSelectOption table;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  SkipList testskiplist;
  std::shared_ptr<MenuSelectOptionTextField> testpattern;
  std::shared_ptr<MenuSelectOptionTextArrow> testtype;
  std::shared_ptr<MenuSelectOptionCheckBox> testinspreadjob;
  unsigned int currentviewspan;
  bool globalskip;
  bool temphighlightline;
};
