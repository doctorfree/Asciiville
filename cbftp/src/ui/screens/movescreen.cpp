#include "movescreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../../path.h"

MoveScreen::MoveScreen(Ui* ui) : UIWindow(ui, "MoveScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('m', KEYACTION_DONE, "Move");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

MoveScreen::~MoveScreen() {

}

void MoveScreen::initialize(unsigned int row, unsigned int col, const std::string & site, const std::string& items, const Path& srcpath, const std::string& dstpath, const std::string& firstitem) {
  active = false;
  this->site = site;
  this->srcpath = srcpath.toString();
  this->items = items;
  this->firstitem = firstitem;
  mso.reset();
  int y = 4;
  if (!site.empty()) {
    y++;
  }
  mso.addStringField(y, 1, "dstpath", "Target path/name:", dstpath, false, col - 22, 512);
  mso.enterFocusFrom(0);
  init(row, col);
}

void MoveScreen::redraw() {
  vv->clear();
  int y = 1;
  if (!site.empty()) {
    vv->putStr(y++, 1, "Site: " + site);
  }
  vv->putStr(y++, 1, "Source path: " + srcpath);
  vv->putStr(y++, 1, "Item: " + items);
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
  if (active && activeelement->cursorPosition() >= 0) {
    ui->showCursor();
    vv->moveCursor(activeelement->getRow(), activeelement->getCol() + activeelement->getLabelText().length() + 1 + activeelement->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

void MoveScreen::deactivate() {
  activeelement->deactivate();
  active = false;
  ui->update();
  ui->setLegend();
}

bool MoveScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      deactivate();
      return true;
    }
    else if (ch == KEY_IC && items == firstitem) {
      for (size_t i = 0; i < firstitem.length(); ++i) {
        std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("dstpath"))->inputChar(firstitem[i]);
      }
    }
    else if (ch == 27) { // esc
      std::shared_ptr<MenuSelectOptionTextField> dstpath = std::static_pointer_cast<MenuSelectOptionTextField>(activeelement);
      if (!dstpath->getData().empty()) {
        dstpath->clear();
      }
      else {
        deactivate();
      }
    }
    else {
      activeelement->inputChar(ch);
    }
    ui->update();
    return true;
  }
  switch(action) {
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
    case KEYACTION_DONE: {
      std::string dstpath = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("dstpath"))->getData();
      if (!dstpath.empty()) {
        ui->returnMove(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("dstpath"))->getData());
      }
      return true;
    }
  }
  return false;
}

std::string MoveScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText() + (items == firstitem ? " - [Insert] insert item name" : "");
  }
  return keybinds.getLegendSummary();
}

std::string MoveScreen::getInfoLabel() const {
  return "MOVE/RENAME";
}
