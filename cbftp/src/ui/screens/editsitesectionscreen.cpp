#include "editsitesectionscreen.h"

#include <list>

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../settingsloadersaver.h"
#include "../../path.h"
#include "../../util.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"

EditSiteSectionScreen::EditSiteSectionScreen(Ui* ui) : UIWindow(ui, "EditSiteSectionScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

EditSiteSectionScreen::~EditSiteSectionScreen() {

}

void EditSiteSectionScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const Path & path) {
  mode = Mode::ADD;
  initialize(row, col, site, "", path);
}

void EditSiteSectionScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const std::string & section) {
  mode = Mode::EDIT;
  oldsection = section;
  Path path = site->getSectionPath(section);
  initialize(row, col, site, section, path);
}

void EditSiteSectionScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const std::string & section, const Path & path) {
  active = false;
  modsite = site;
  exists = false;
  unsigned int y = 1;
  unsigned int x = 1;
  mso.reset();
  mso.addStringField(y++, x, "names", (mode == Mode::EDIT ? "Name:" : "Name(s):"), section, false, 64, 128);
  mso.addTextButtonNoContent(y++, x, "select", "Select name from list...");
  mso.addStringField(y++, x, "path", "Path:", path.toString(), false, 64);
  mso.enterFocusFrom(0);
  init(row, col);
}

void EditSiteSectionScreen::redraw() {
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
  exists = false;
  unsigned int y = 5;
  std::shared_ptr<MenuSelectOptionElement> path = mso.getElement("path");
  std::list<std::string> alreadyboundsections = modsite->getSectionsForPath(std::static_pointer_cast<MenuSelectOptionTextField>(path)->getData());
  if (!alreadyboundsections.empty()) {
    vv->putStr(y++, 1, "Sections currently bound to this path: " + util::join(alreadyboundsections, ", "));
  }
  if (mode == Mode::ADD) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement("names");
    std::list<std::string> sections = util::split(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData(), ",");
    for (const std::string& section : sections) {
      if (modsite->hasSection(section)) {
        std::string path = modsite->getSectionPath(section).toString();
        vv->putStr(y++, 1, "Warning: Section " + section + " exists and will be updated. Old path: " + path);
        exists = true;
      }
    }
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

bool EditSiteSectionScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      ui->setLegend();
      ui->redraw();
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
    case KEYACTION_ENTER: {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      activation = msoe->activate();
      if (msoe->getIdentifier() == "select") {
        std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement("names");
        std::list<std::string> preselected = util::split(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData(), ",");
        ui->goSelectSection(preselected);
        return true;
      }
      if (!activation) {
        ui->redraw();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->setLegend();
      ui->update();
      return true;
    }
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DONE:
      if (exists) {
        ui->goConfirmation("Warning: one or more specified sections already exist and will be updated. Are you sure?");
        return true;
      }
      if (setSections()) {
        ui->returnToLast();
        return true;
      }
      return false;
  }
  return false;
}

bool EditSiteSectionScreen::setSections() {
  std::shared_ptr<MenuSelectOptionTextField> nameelem = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("names"));
  std::shared_ptr<MenuSelectOptionTextField> pathelem = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("path"));
  std::list<std::string> names = util::split(nameelem->getData(), ",");
  std::string path = pathelem->getData();
  if (names.empty() || names.front().empty() || path.empty()) {
    return false;
  }
  if (mode == Mode::EDIT && names.front() != oldsection) {
    modsite->removeSection(oldsection);
  }
  for (const std::string& name : names) {
    modsite->addSection(name, path);
  }
  global->getSettingsLoaderSaver()->saveSettings();
  return true;
}

void EditSiteSectionScreen::command(const std::string & command, const std::string & arg) {
  if (command == "returnselectitems") {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement("names");
    std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->setText(arg);
    mso.setPointer(msoe);
  }
  else if (command == "yes") {
    if (setSections()) {
      ui->returnToLast();
      return;
    }
  }
  ui->redraw();
}

std::string EditSiteSectionScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string EditSiteSectionScreen::getInfoLabel() const {
  if (mode == Mode::EDIT) {
    return "EDIT SECTION ON " + modsite->getName() + ": " + oldsection;
  }
  return "ADD SECTION ON " + modsite->getName();
}
