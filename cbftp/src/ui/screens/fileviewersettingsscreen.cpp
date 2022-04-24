#include "fileviewersettingsscreen.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../externalfileviewing.h"

#include "../../globalcontext.h"
#include "../../localstorage.h"

FileViewerSettingsScreen::FileViewerSettingsScreen(Ui* ui) : UIWindow(ui, "FileViewerSettingsScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

FileViewerSettingsScreen::~FileViewerSettingsScreen() {

}

void FileViewerSettingsScreen::initialize(unsigned int row, unsigned int col) {
  active = false;
  ls = global->getLocalStorage();
  unsigned int y = 1;
  unsigned int x = 1;
  mso.clear();
  mso.addStringField(y++, x, "temppath", "Temporary download path:", ls->getTempPath().toString(), false, 128, 128);
  y++;
  mso.addStringField(y++, x, "video", "Video viewer:", ui->getExternalFileViewing().getVideoViewer(), false);
  mso.addStringField(y++, x, "audio", "Audio viewer:", ui->getExternalFileViewing().getAudioViewer(), false);
  mso.addStringField(y++, x, "image", "Image viewer:", ui->getExternalFileViewing().getImageViewer(), false);
  mso.addStringField(y++, x, "pdf", "PDF viewer:", ui->getExternalFileViewing().getPDFViewer(), false);
  mso.makeLeavableDown();
  mso.enterFocusFrom(0);
  init(row, col);
}

void FileViewerSettingsScreen::redraw() {
  vv->clear();
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
  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    vv->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool FileViewerSettingsScreen::keyPressed(unsigned int ch) {
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
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_DONE:
      for(unsigned int i = 0; i < mso.size(); i++) {
        std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "temppath") {
          ls->setTempPath(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "video") {
          ui->getExternalFileViewing().setVideoViewer(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "audio") {
          ui->getExternalFileViewing().setAudioViewer(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "image") {
          ui->getExternalFileViewing().setImageViewer(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "pdf") {
          ui->getExternalFileViewing().setPDFViewer(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
      }
      ui->returnToLast();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string FileViewerSettingsScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string FileViewerSettingsScreen::getInfoLabel() const {
  return "EXTERNAL FILE VIEWER SETTINGS";
}
