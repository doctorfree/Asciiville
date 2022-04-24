#include "makedirscreen.h"

#include "../../globalcontext.h"
#include "../../filelist.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"

MakeDirScreen::MakeDirScreen(Ui* ui) : UIWindow(ui, "MakeDirScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('m', KEYACTION_DONE, "Make directory");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

MakeDirScreen::~MakeDirScreen() {

}

void MakeDirScreen::initialize(unsigned int row, unsigned int col, const std::string & site, UIFileList & filelist) {
  active = false;
  this->site = site;
  this->filelist = filelist;
  alreadyexists = false;
  mso.reset();
  int y = 3;
  if (!site.empty()) {
    y++;
  }
  mso.addStringField(y, 1, "name", "Name:", "", false, col - 10, 512);
  mso.enterFocusFrom(0);
  init(row, col);
}

void MakeDirScreen::redraw() {
  vv->clear();
  int y = 1;
  if (!site.empty()) {
    vv->putStr(y++, 1, "Site: " + site);
  }
  vv->putStr(y, 1, "Path: " + filelist.getPath().toString());
  if (alreadyexists) {
    vv->putStr(y + 4, 1, "ERROR: an item with that name already exists!");
    alreadyexists = false;
  }
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

bool MakeDirScreen::keyPressed(unsigned int ch) {
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
    case KEYACTION_DONE:
      tryMakeDir();
      return true;
  }
  return false;
}

void MakeDirScreen::tryMakeDir() {
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement("name");
  std::string dirname = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
  if (dirname.empty()) {
    return;
  }
  if (filelist.contains(dirname)) {
    alreadyexists = true;
    ui->redraw();
    return;
  }
  ui->returnMakeDir(dirname);
  return;
}

std::string MakeDirScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string MakeDirScreen::getInfoLabel() const {
  return "MAKE NEW DIRECTORY";
}
