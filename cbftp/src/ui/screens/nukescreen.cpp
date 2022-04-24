#include "nukescreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptiontextbutton.h"

namespace {

enum KeyAction {
  KEYACTION_NUKE,
  KEYACTION_NUKE_PROPER,
  KEYACTION_NUKE_REPACK,
  KEYACTION_NUKE_DUPE,
  KEYACTION_NUKE_DUPE_WEB,
  KEYACTION_NUKE_INCOMPLETE
};

}

NukeScreen::NukeScreen(Ui* ui) : UIWindow(ui, "NukeScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind({'d', 'n'}, KEYACTION_NUKE, "Nuke");
  keybinds.addBind('p', KEYACTION_NUKE_PROPER, "Proper");
  keybinds.addBind('r', KEYACTION_NUKE_REPACK, "Repack");
  keybinds.addBind('u', KEYACTION_NUKE_DUPE, "Dupe");
  keybinds.addBind('w', KEYACTION_NUKE_DUPE_WEB, "Dupe web");
  keybinds.addBind('i', KEYACTION_NUKE_INCOMPLETE, "Incomplete");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

NukeScreen::~NukeScreen() {

}

void NukeScreen::initialize(unsigned int row, unsigned int col, const std::string & sitestr, const std::string & items, const Path & path) {
  active = false;
  this->sitestr = sitestr;
  this->items = items;
  this->path = path;
  std::list<std::string> sections = global->getSiteManager()->getSite(sitestr)->getSectionsForPath(path);
  mso.reset();
  mso.addStringField(5, 1, "reason", "Reason:", "", false, col - 10, 512);
  mso.addIntArrow(6, 1, "multiplier", "Multiplier:", 1, 0, 100);
  mso.enterFocusFrom(0);
  init(row, col);
}

void NukeScreen::redraw() {
  vv->clear();
  vv->putStr(1, 1, "Site: " + sitestr);
  vv->putStr(2, 1, "Item: " + items);
  vv->putStr(3, 1, "Path: " + path.toString());
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
  vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    vv->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool NukeScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
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
  switch(action) {
    case KEYACTION_UP:
      if (mso.goUp()) {
        ui->redraw();
        return true;
      }
      return false;
    case KEYACTION_DOWN:
      if (mso.goDown()) {
        ui->redraw();
        return true;
      }
      return false;
    case KEYACTION_ENTER:
      activeelement = mso.getElement(mso.getSelectionPointer());
      activeelement->activate();
      active = true;
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_NUKE:
      nuke();
      return true;
    case KEYACTION_NUKE_PROPER:
      nuke(1, "proper");
      return true;
    case KEYACTION_NUKE_REPACK:
      nuke(1, "repack");
      return true;
    case KEYACTION_NUKE_DUPE:
      nuke(1, "dupe");
      return true;
    case KEYACTION_NUKE_DUPE_WEB:
      nuke(3, "dupe.web");
      return true;
    case KEYACTION_NUKE_INCOMPLETE:
      nuke(3, "incomplete");
      return true;
  }
  return false;
}

void NukeScreen::nuke() {
  int multiplier = 1;
  std::string reason;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    std::string identifier = msoe->getIdentifier();
    if (identifier == "multiplier") {
      multiplier = std::static_pointer_cast<MenuSelectOptionNumArrow>(msoe)->getData();
    }
    else if (identifier == "reason") {
      reason = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
    }
  }
  nuke(multiplier, reason);
}

void NukeScreen::nuke(int multiplier, const std::string & reason) {
  ui->returnNuke(multiplier, reason);
}

std::string NukeScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string NukeScreen::getInfoLabel() const {
  return "NUKE";
}
