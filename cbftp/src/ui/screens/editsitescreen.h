#pragma once

#include <list>
#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Site;
class MenuSelectOptionElement;

class EditSiteScreen : public UIWindow {
public:
  EditSiteScreen(Ui * ui);
  ~EditSiteScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & operation, const std::string & site);
  void redraw() override;
  void command(const std::string & command, const std::string & arg) override;
  bool keyPressed(unsigned int ch) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  void fillPreselectionList(const std::string &, std::list<std::shared_ptr<Site> > *) const;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  std::shared_ptr<Site> site;
  std::shared_ptr<Site> modsite;
  std::string operation;
  bool slotsupdated;
};
