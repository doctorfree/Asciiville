#pragma once

#include <list>
#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"
#include "../uifilelist.h"

class MenuSelectOptionElement;
class UIFileList;

class MakeDirScreen : public UIWindow {
public:
  MakeDirScreen(Ui *);
  ~MakeDirScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & site,  UIFileList & filelist);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void tryMakeDir();
  bool active;
  std::string site;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::string release;
  UIFileList filelist;
  bool alreadyexists;
};
