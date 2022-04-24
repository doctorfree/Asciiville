#include "selectsitesscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncheckbox.h"

#include "../../sitemanager.h"
#include "../../site.h"
#include "../../globalcontext.h"

namespace {

enum KeyAction {
  KEYACTION_SELECT
};

}

SelectSitesScreen::SelectSitesScreen(Ui* ui) : UIWindow(ui, "SelectSitesScreen"), mso(*vv) {
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind({10, ' '}, KEYACTION_SELECT, "Select");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind('t', KEYACTION_TOGGLE_ALL, "Toggle all");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Navigate left");
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Navigate right");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
}

void SelectSitesScreen::initializeExclude(unsigned int row, unsigned int col, const std::string & purpose, std::list<std::shared_ptr<Site> > preselectedsites, std::list<std::shared_ptr<Site> > excludedsites) {
  sm = global->getSiteManager();
  this->purpose = purpose;
  std::set<std::shared_ptr<Site> > preselected;
  std::set<std::shared_ptr<Site> > excluded;
  for (std::list<std::shared_ptr<Site> >::iterator it = preselectedsites.begin(); it != preselectedsites.end(); it++) {
    preselected.insert(*it);
  }
  for (std::list<std::shared_ptr<Site> >::iterator it = excludedsites.begin(); it != excludedsites.end(); it++) {
    excluded.insert(*it);
  }
  mso.clear();
  mso.enterFocusFrom(0);
  std::vector<std::shared_ptr<Site> >::const_iterator it;
  for (it = sm->begin(); it != sm->end(); it++) {
    if (excluded.find(*it) != excluded.end()) {
      continue;
    }
    bool selected = preselected.find(*it) != preselected.end();
    tempsites.push_back(std::pair<std::string, bool>((*it)->getName(), selected));
  }
  togglestate = false;
  init(row, col);
}

void SelectSitesScreen::initializeSelect(unsigned int row, unsigned int col, const std::string & purpose, std::list<std::shared_ptr<Site> > preselectedsites, std::list<std::shared_ptr<Site> > sites) {
  sm = global->getSiteManager();
  this->purpose = purpose;
  std::set<std::shared_ptr<Site> > preselected;
  for (std::list<std::shared_ptr<Site> >::iterator it = preselectedsites.begin(); it != preselectedsites.end(); it++) {
    preselected.insert(*it);
  }
  for (std::list<std::shared_ptr<Site> >::iterator it = sites.begin(); it != sites.end(); it++) {
    bool selected = preselected.find(*it) != preselected.end();
    tempsites.push_back(std::pair<std::string, bool>((*it)->getName(), selected));
  }
  mso.clear();
  mso.enterFocusFrom(0);
  togglestate = false;
  init(row, col);
}

void SelectSitesScreen::redraw() {
  vv->clear();
  unsigned int y = 1;
  unsigned int x = 1;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionCheckBox> msocb = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement(i));
    tempsites.push_back(std::pair<std::string, bool>(msocb->getIdentifier(), msocb->getData()));
  }
  mso.clear();
  unsigned int longestsitenameinline = 0;
  std::list<std::pair<std::string, bool> >::iterator it;
  for (it = tempsites.begin(); it != tempsites.end(); it++) {
    std::string sitename = it->first;
    if (sitename.length() > longestsitenameinline) {
      longestsitenameinline = sitename.length();
    }
    if (y >= row - 1) {
      y = 1;
      x += longestsitenameinline + 7;
      longestsitenameinline = 0;
    }
    mso.addCheckBox(y++, x, sitename, sitename, it->second);
  }
  tempsites.clear();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
  }
  if (!mso.size()) {
    vv->putStr(1, 1, "(no sites available)");
  }
}

bool SelectSitesScreen::keyPressed(unsigned int ch) {
  unsigned int pagerows = (unsigned int) row * 0.6;
  bool activation;
  int action = keybinds.getKeyAction(ch);
  switch(action) {
    case KEYACTION_UP:
      if (mso.goUp()) {
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_DOWN:
      if (mso.goDown()) {
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_LEFT:
      if (mso.goLeft()) {
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_RIGHT:
      if (mso.goRight()) {
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_NEXT_PAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (!mso.goDown()) {
          break;
        }
      }
      ui->redraw();
      return true;
    case KEYACTION_PREVIOUS_PAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (!mso.goUp()) {
          break;
        }
      }
      ui->redraw();
      return true;
    case KEYACTION_SELECT:
    {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      if (!!msoe) {
        activation = msoe->activate();
        if (!activation) {
          ui->update();
          return true;
        }
        ui->update();
        ui->setLegend();
      }
      return true;
    }
    case KEYACTION_DONE:
    {
      std::string sites = "";
      for (unsigned int i = 0; i < mso.size(); i++) {
        std::shared_ptr<MenuSelectOptionCheckBox> msocb = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement(i));
        if (msocb->getData()) {
          sites += msocb->getIdentifier() + ",";
        }
      }
      if (sites.length() > 0) {
        sites = sites.substr(0, sites.length() - 1);
      }
      ui->returnSelectItems(sites);
      return true;
    }
    case KEYACTION_TOGGLE_ALL:
    {
      bool triggered = false;
      while (!triggered && mso.size()) {
        for (unsigned int i = 0; i < mso.size(); i++) {
          std::shared_ptr<MenuSelectOptionCheckBox> msocb = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement(i));
          if (togglestate == msocb->getData()) {
            msocb->activate();
            triggered = true;
          }
        }
        togglestate = !togglestate;
      }
      ui->redraw();
      break;
    }
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string SelectSitesScreen::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::string SelectSitesScreen::getInfoLabel() const {
  if (purpose.length()) {
    return "SELECT SITES - " + purpose;
  }
  return "SELECT SITES";
}
