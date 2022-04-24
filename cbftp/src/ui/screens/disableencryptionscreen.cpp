#include "disableencryptionscreen.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"

#include "../../globalcontext.h"
#include "../../settingsloadersaver.h"

DisableEncryptionScreen::DisableEncryptionScreen(Ui* ui) : UIWindow(ui, "DisableEncryptionScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('d', KEYACTION_DONE, "Disable encryption");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

DisableEncryptionScreen::~DisableEncryptionScreen() {

}

void DisableEncryptionScreen::initialize(unsigned int row, unsigned int col) {
  active = false;
  mismatch = false;
  mso.clear();
  mso.addStringField(5, 1, "newkey", "Passphrase:", "", true);
  init(row, col);
}

void DisableEncryptionScreen::redraw() {
  vv->clear();
  unsigned int y = 1;
  vv->putStr(y, 1, "Warning! Disabling encryption means that anyone with read access to your home directory");
  vv->putStr(y+1, 1, "will be able to read your data file, including your passwords.");
  vv->putStr(y+2, 1, "Please verify this action by entering your current passphrase.");
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
  if (mismatch) {
    error = "Failed: The key did not match.";
  }
  vv->putStr(7, 1, error);

  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    vv->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool DisableEncryptionScreen::keyPressed(unsigned int ch) {
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
    case KEYACTION_ENTER:
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      mismatch = false;
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_DONE: {
      std::shared_ptr<MenuSelectOptionTextField> passfield = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(0));
      std::string key = passfield->getData();
      passfield->clear();
      if (!global->getSettingsLoaderSaver()->setPlain(key)) {
        mismatch = true;
      }
      else {
        ui->returnToLast();
        return true;
      }
      ui->update();
      return true;
    }
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string DisableEncryptionScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string DisableEncryptionScreen::getInfoLabel() const {
  return "DISABLE ENCRYPTION";
}
