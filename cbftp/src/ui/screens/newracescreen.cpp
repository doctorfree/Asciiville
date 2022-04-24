#include "newracescreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../engine.h"
#include "../../race.h"

#include "../ui.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptiontextarrow.h"
#include "../menuselectoptionelement.h"
#include "../focusablearea.h"

namespace {

enum KeyAction {
  KEYACTION_START,
  KEYACTION_START_RETURN,
};

}

NewRaceScreen::NewRaceScreen(Ui * ui) : UIWindow(ui, "NewRaceScreen"), msos(*vv), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('s', KEYACTION_START, "Start spread job");
  keybinds.addBind('S', KEYACTION_START_RETURN, "Start spread job and return to browsing");
  keybinds.addBind('t', KEYACTION_TOGGLE_ALL, "Toggle all");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Navigate left");
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Navigate right");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Page up");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Page down");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

NewRaceScreen::~NewRaceScreen() {

}

void NewRaceScreen::initialize(unsigned int row, unsigned int col, const std::string & site, const std::list<std::string> & sections, const std::list<std::pair<std::string, bool> > & items) {
  active = false;
  toggleall = false;
  unsigned int y = 2;
  unsigned int x = 1;
  infotext = "";
  bool sectionset = false;
  int sectx = x + std::string("Section: ").length();
  msos.reset();
  msota = msos.addTextArrow(y++, 1, "profile", "Profile:");
  msota->addOption("Race", SPREAD_RACE);
  msota->addOption("Distribute", SPREAD_DISTRIBUTE);
  msota->setId(1);
  for (std::list<std::string>::const_iterator it = sections.begin(); it != sections.end(); it++) {
    if (!sectionset) {
      this->section = *it;
      sectionset = true;
    }
    std::string buttontext = " " + *it + " ";
    std::shared_ptr<MenuSelectOptionTextButton> msotb = msos.addTextButton(y, sectx, *it, buttontext);
    msotb->setId(0);
    sectx = sectx + buttontext.length();
  }
  y = y + 2;
  this->items = items;
  startsite = global->getSiteManager()->getSite(site);
  focusedarea = &msos;
  msos.makeLeavableDown();
  mso.reset();
  mso.makeLeavableUp();
  msos.enterFocusFrom(0);
  populateSiteList();
  init(row, col);
}

void NewRaceScreen::populateSiteList() {
  std::vector<std::shared_ptr<Site> >::const_iterator it;
  mso.clear();
  if (!tempsites.size()) {
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      const std::shared_ptr<Site> & site = *it;
      bool allowed = true;
      for (std::list<std::pair<std::string, bool> >::const_iterator itemit = items.begin(); itemit != items.end(); itemit++) {
        if (site->getSkipList().check((site->getSectionPath(section) / itemit->first).toString(), true, false).action == SKIPLIST_DENY) {
          allowed = false;
          break;
        }
      }
      if (site->hasSection(section) && !site->getDisabled() && allowed) {
        tempsites.push_back(std::pair<std::string, bool>(site->getName(), toggleall || site == startsite));
      }
    }
  }
  unsigned int y = 6;
  unsigned int x = 1;
  unsigned int longestsitenameinline = 0;
  std::list<std::pair<std::string, bool> >::iterator it2;
  for (it2 = tempsites.begin(); it2 != tempsites.end(); it2++) {
    if (it2->first.length() > longestsitenameinline) {
      longestsitenameinline = it2->first.length();
    }
    if (y >= row - 1) {
      y = 6;
      x += longestsitenameinline + 7;
      longestsitenameinline = 0;
    }
    mso.addCheckBox(y++, x, it2->first, it2->first, it2->second);
  }
  tempsites.clear();
}

void NewRaceScreen::redraw() {
  vv->clear();
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionCheckBox> msocb = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement(i));
    tempsites.push_back(std::pair<std::string, bool>(msocb->getIdentifier(), msocb->getData()));
  }
  populateSiteList();
  std::string item = items.front().first;
  if (items.size() > 1) {
    item = std::to_string(static_cast<int>(items.size())) + " items";
  }
  vv->putStr(1, 1, "Item: " + item);
  vv->putStr(3, 1, "Section: ");
  bool highlight;
  for (unsigned int i = 0; i < msos.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = msos.getElement(i);
    highlight = false;
    if (msos.isFocused() && msos.getSelectionPointer() == i) {
      highlight = true;
    }
    if (msoe->getId() == 0) {
      vv->putStr(msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe), highlight);
    }
    else {
      vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
      vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
    }
  }
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
  }
}

bool NewRaceScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  infotext = "";
  unsigned int pagerows = (unsigned int) (row - 6) * 0.6;
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(action) {
    case KEYACTION_UP:
      if (focusedarea->goUp() || focusedarea->goPrevious()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &msos;
          focusedarea->enterFocusFrom(2);
        }
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_DOWN:
      if (focusedarea->goDown() || focusedarea->goNext()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
          focusedarea->enterFocusFrom(0);
        }
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_LEFT:
      if (focusedarea->goLeft()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_RIGHT:
      if (focusedarea->goRight()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_NEXT_PAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (focusedarea->goDown()) {
          if (!focusedarea->isFocused()) {
            defocusedarea = focusedarea;
            focusedarea = &mso;
            focusedarea->enterFocusFrom(0);
          }
        }
      }
      ui->redraw();
      return true;
    case KEYACTION_PREVIOUS_PAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (focusedarea->goUp()) {
          if (!focusedarea->isFocused()) {
            defocusedarea = focusedarea;
            focusedarea = &msos;
            focusedarea->enterFocusFrom(2);
          }
        }
      }
      ui->redraw();
      return true;
    case KEYACTION_ENTER:
      activation = focusedarea->getElement(focusedarea->getSelectionPointer())->activate();
      if (!activation) {
        if (focusedarea == &msos) {
          std::shared_ptr<MenuSelectOptionElement> msoe = msos.getElement(msos.getSelectionPointer());
          if (msoe->getId() == 0) {
            section = msoe->getIdentifier();
            tempsites.clear();
            mso.clear();
          }
        }
        ui->redraw();
        return true;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_START:
    {
      bool goracestatus = items.size() == 1;
      std::shared_ptr<Race> race = startRace(!goracestatus);
      if (!!race) {
        if (goracestatus) {
          ui->returnRaceStatus(race->getId());
        }
        else {
          ui->returnToLast();
        }
      }
      return true;
    }
    case KEYACTION_START_RETURN:
    {
      std::shared_ptr<Race> race = startRace(true);
      if (!!race) {
        ui->returnToLast();
      }
      return true;
    }
    case KEYACTION_TOGGLE_ALL:
      toggleall = !toggleall;
      populateSiteList();
      ui->redraw();
      return true;
  }
  return false;
}

std::string NewRaceScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string NewRaceScreen::getInfoLabel() const {
  return "START NEW SPREAD JOB";
}

std::string NewRaceScreen::getInfoText() const {
  return infotext;
}

std::string NewRaceScreen::getSectionButtonText(std::shared_ptr<MenuSelectOptionElement> msoe) const {
  std::string buttontext = msoe->getLabelText();
  if (msoe->getIdentifier() == section) {
    buttontext[0] = '[';
    buttontext[buttontext.length()-1] = ']';
  }
  return buttontext;
}

std::shared_ptr<Race> NewRaceScreen::startRace(bool addtemplegend) {
  msota->getData();
  std::list<std::string> sites;
  std::shared_ptr<Race> lastrace;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionCheckBox> msocb = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement(i));
    if (msocb->getData()) {
      sites.push_back(msocb->getIdentifier());
    }
  }
  if (sites.size() < 2) {
    ui->goInfo("Cannot start spread job with less than 2 sites!");
    return lastrace;
  }

  for (std::list<std::pair<std::string, bool> >::const_iterator itemit = items.begin(); itemit != items.end(); itemit++) {
    if (msota->getData() == SPREAD_RACE) {
      std::shared_ptr<Race> race = global->getEngine()->newRace(itemit->first, section, sites, false);
      if (!!race) {
        lastrace = race;
      }
    }
    else {
      std::shared_ptr<Race> race = global->getEngine()->newDistribute(itemit->first, section, sites, false);
      if (!!race) {
        lastrace = race;
      }
    }
  }
  if (addtemplegend && !!lastrace) {
    ui->addTempLegendSpreadJob(lastrace->getId());
  }
  return lastrace;
}
