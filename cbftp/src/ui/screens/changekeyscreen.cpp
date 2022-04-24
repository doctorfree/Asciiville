#include "changekeyscreen.h"

#include "../../globalcontext.h"
#include "../../settingsloadersaver.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"

ChangeKeyScreen::ChangeKeyScreen(Ui* ui) : UIWindow(ui, "ChangeKeyScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

ChangeKeyScreen::~ChangeKeyScreen() {

}

void ChangeKeyScreen::initialize(unsigned int row, unsigned int col) {
  active = false;
  mismatch = false;
  oldmismatch = false;
  tooshort = false;
  unsigned int y = 4;
  unsigned int x = 1;
  mso.clear();
  mso.addStringField(y++, x, "oldkey", "Old passphrase:", "", true);
  mso.addStringField(y++, x, "newkey", "New passphrase:", "", true);
  mso.addStringField(y++, x, "newkey2", "Verify new:", "", true);
  init(row, col);
}

void ChangeKeyScreen::redraw() {
  vv->clear();
  unsigned int y = 1;
  vv->putStr(y, 1, "Please verify with your old encryption key.");
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
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
  std::string error = "                                                          ";
  if (tooshort) {
    error = "Failed: The passphrase must be at least " + std::to_string(SHORTESTKEY) + " characters long.";
  }
  else if (mismatch) {
    error = "Failed: The new keys did not match.";
  }
  else if (oldmismatch) {
    error = "Failed: the old key was not correct.";
  }
  vv->putStr(8, 1, error);

  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    vv->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool ChangeKeyScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      ui->setLegend();
      ui->update();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(action) {
    case KEYACTION_UP: {
      bool moved = mso.goUp();
      ui->update();
      return moved;
    }
    case KEYACTION_DOWN: {
      bool moved = mso.goDown();
      ui->update();
      return moved;
    }
    case KEYACTION_ENTER:
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      tooshort = false;
      mismatch = false;
      oldmismatch = false;
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->setLegend();
      ui->update();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DONE: {
      std::shared_ptr<MenuSelectOptionTextField> field1 = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(0));
      std::shared_ptr<MenuSelectOptionTextField> field2 = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(1));
      std::shared_ptr<MenuSelectOptionTextField> field3 = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(2));
      std::string oldkey = field1->getData();
      std::string newkey = field2->getData();
      std::string newkey2 = field3->getData();
      field1->clear();
      field2->clear();
      field3->clear();
      if (newkey == newkey2) {
        if (newkey.length() >= SHORTESTKEY) {
          if (global->getSettingsLoaderSaver()->changeKey(oldkey, newkey)) {
            ui->returnToLast();
            return true;
          }
          else {
            oldmismatch = true;
          }
        }
        else {
          tooshort = true;
        }
      }
      else {
        mismatch = true;
      }
      ui->update();
      return true;
    }
  }
  return false;
}

std::string ChangeKeyScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string ChangeKeyScreen::getInfoLabel() const {
  return "CHANGE ENCRYPTION KEY";
}
