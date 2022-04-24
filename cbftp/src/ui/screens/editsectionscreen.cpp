#include "editsectionscreen.h"

#include <cassert>

#include "../../globalcontext.h"
#include "../../section.h"
#include "../../sectionmanager.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptiontextbutton.h"

EditSectionScreen::EditSectionScreen(Ui* ui) : UIWindow(ui, "EditSectionScreen"), section(nullptr), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind('S', KEYACTION_SKIPLIST, "Skiplist");
}

EditSectionScreen::~EditSectionScreen() {

}

void EditSectionScreen::initialize(unsigned int row, unsigned int col, const std::string & section) {
  active = false;
  if (this->section != nullptr) {
    delete this->section;
  }
  if (section == "") {
    mode = Mode::ADD;
    this->section = new Section();
    oldname = "";
  }
  else {
    mode = Mode::EDIT;
    Section * editsection = global->getSectionManager()->getSection(section);
    assert(editsection != NULL);
    this->section = new Section(*editsection);
    oldname = editsection->getName();
  }
  mso.reset();
  mso.addStringField(1, 1, "name", "Section name:", this->section->getName(), false);
  mso.addTextButtonNoContent(3, 1, "skiplist", "Configure skiplist...");
  std::shared_ptr<MenuSelectOptionNumArrow> hotkeyarrow = mso.addIntArrow(4, 1, "hotkey", "Hotkey:", this->section->getHotKey(), -1, 9);
  hotkeyarrow->setSubstituteText(-1, "None");
  mso.enterFocusFrom(0);
  init(row, col);
}

void EditSectionScreen::redraw() {
  vv->clear();
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

bool EditSectionScreen::keyPressed(unsigned int ch) {
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
  bool activation;
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
    case KEYACTION_ENTER:
      activeelement = mso.getElement(mso.getSelectionPointer());
      activation = activeelement->activate();
      if (!activation) {
        if (activeelement->getIdentifier() == "skiplist") {
          ui->goSkiplist(&section->getSkipList());
          return true;
        }
      }
      active = true;
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DONE:
      done();
      return true;
    case KEYACTION_SKIPLIST:
      ui->goSkiplist(&section->getSkipList());
      return true;
  }
  return false;
}

void EditSectionScreen::done() {
  for(unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    std::string identifier = msoe->getIdentifier();
    if (identifier == "name") {
      std::string newname = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
      section->setName(newname);
    }
    if (identifier == "hotkey") {
      section->setHotKey(std::static_pointer_cast<MenuSelectOptionNumArrow>(msoe)->getData());
    }
  }
  switch (mode) {
    case Mode::ADD:
      if (global->getSectionManager()->addSection(*section)) {
        ui->returnToLast();
      }
      break;
    case Mode::EDIT:
      if (global->getSectionManager()->replaceSection(*section, oldname)) {
        ui->returnToLast();
      }
      break;
  }
}

std::string EditSectionScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string EditSectionScreen::getInfoLabel() const {
  if (mode == Mode::ADD) {
    return "ADD SECTION";
  }
  return "EDIT SECTION: " + oldname;
}
