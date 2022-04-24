#include "rawcommandscreen.h"

#include "../../ftpconn.h"
#include "../../rawbuffer.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../globalcontext.h"
#include "../../encoding.h"
#include "../../site.h"

#include "../ui.h"
#include "../termint.h"

#include "rawdatascreen.h"

namespace {

enum KeyAction {
  KEYACTION_INSERT_SELECTION,
  KEYACTION_CLEAR
};

enum KeyScopes {
  KEYSCOPE_INPUT
};

}

RawCommandScreen::RawCommandScreen(Ui* ui) : UIWindow(ui, "RawCommandScreen") {
  keybinds.addScope(KEYSCOPE_INPUT, "If input is accepted");
  keybinds.addBind(27, KEYACTION_BACK_CANCEL, "Clear/Return");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Scroll up");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Scroll down");
  keybinds.addBind(TERMINT_CTRL_L, KEYACTION_CLEAR, "Clear log");
  keybinds.addBind(10, KEYACTION_ENTER, "Send command", KEYSCOPE_INPUT);
  keybinds.addBind(KEY_UP, KEYACTION_UP, "History up", KEYSCOPE_INPUT);
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "History down", KEYSCOPE_INPUT);
  keybinds.addBind(KEY_IC, KEYACTION_INSERT_SELECTION, "Insert selection (if available)", KEYSCOPE_INPUT);
  keybinds.useAlternateKeybindsButton();
}

void RawCommandScreen::initialize(unsigned int row, unsigned int col, const std::string & sitename, const Path & path, const std::string & selection) {
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitename);
  this->label = "RAW COMMAND INPUT: " + sitename;
  this->selection = selection;
  this->path = path;
  if (this->path == "") {
    this->path = "/";
  }
  hasselection = false;
  if (selection.length()) {
    hasselection = true;
  }
  expectbackendpush = true;

  this->rawbuf = sitelogic->getRawCommandBuffer();
  rawbuf->setUiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  allowinput = true;
  init(row, col);
}

void RawCommandScreen::initialize(unsigned int row, unsigned int col, const std::string & sitename) {
  std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(sitename);
  initialize(row, col, sitename, sl->getSite()->getBasePath(), "");
}

void RawCommandScreen::initialize(unsigned int row, unsigned int col, RawBuffer * rawbuffer, const std::string & label, const std::string & infotext) {
  this->label = label;
  this->path = infotext;
  selection = "";
  hasselection = false;
  expectbackendpush = false;
  this->rawbuf = rawbuffer;
  readfromcopy = false;
  copyreadpos = 0;
  allowinput = false;
  init(row, col);
}

void RawCommandScreen::redraw() {
  vv->clear();
  std::string oldtext = rawcommandfield.getData();
  rawcommandfield = MenuSelectOptionTextField("rawcommand", row-1, 10, "", oldtext, col-15, 65536, false);
  fixCopyReadPos();
  update();
}

void RawCommandScreen::update() {
  vv->clear();
  unsigned int rownum = row;
  if (allowinput) {
    rownum--;
    ui->showCursor();
    std::string pretag = "[Raw command]: ";
    vv->putStr(rownum, 0, pretag + rawcommandfield.getContentText());
    vv->moveCursor(rownum, pretag.length() + rawcommandfield.cursorPosition());
  }
  RawDataScreen::printRawBufferLines(vv, rawbuf, rownum, col, 0, readfromcopy, copyreadpos);
}

bool RawCommandScreen::keyPressed(unsigned int ch) {
  int scope = getCurrentScope();
  int action = keybinds.getKeyAction(ch, scope);
  unsigned int rownum = allowinput ? row - 1 : row;
  std::string command;
  switch(action) {
    case KEYACTION_BACK_CANCEL:
      if (rawcommandfield.getData() != "") {
        rawcommandfield.clear();
        ui->update();
      }
      else {
        rawbuf->setUiWatching(false);
        ui->returnToLast();
      }
      return true;
    case KEYACTION_PREVIOUS_PAGE:
      if (!readfromcopy) {
        rawbuf->freezeCopy();
        copyreadpos = 0;
        readfromcopy = true;
      }
      else {
        copyreadpos = copyreadpos + rownum / 2;
        fixCopyReadPos();
      }
      ui->update();
      return true;
    case KEYACTION_NEXT_PAGE:
      if (readfromcopy) {
        if (copyreadpos == 0) {
          readfromcopy = false;
        }
        else if (copyreadpos < rownum / 2) {
          copyreadpos = 0;
        }
        else {
          copyreadpos = copyreadpos - rownum / 2;
        }
      }
      ui->update();
      return true;
    case KEYACTION_CLEAR:
      rawbuf->clear();
      readfromcopy = false;
      ui->redraw();
      return true;
  }
  if (!allowinput) {
    return false;
  }
  if (rawcommandfield.inputChar(ch)) {
    ui->update();
    return true;
  }
  switch (action) {
    case KEYACTION_ENTER:
      command = rawcommandfield.getData();
      if (command != "") {
        readfromcopy = false;
        history.push(command);
        sitelogic->requestRawCommand(ui, path, command);
        rawcommandfield.clear();
        ui->update();
      }
      return true;
    case KEYACTION_UP:
      if (history.canBack()) {
        if (history.current()) {
          history.setCurrent(rawcommandfield.getData());
        }
        history.back();
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    case KEYACTION_DOWN:
      if (history.forward()) {
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    case KEYACTION_INSERT_SELECTION:
      if (!allowinput) {
        return false;
      }
      for (unsigned int i = 0; i < selection.length(); i++) {
        rawcommandfield.inputChar(selection[i]);
      }
      ui->update();
      return true;
  }
  return false;
}

std::string RawCommandScreen::getLegendText() const {
  return keybinds.getLegendSummary(getCurrentScope());
}

std::string RawCommandScreen::getInfoLabel() const {
  return label;
}

std::string RawCommandScreen::getInfoText() const {
  return path.toString();
}

int RawCommandScreen::getCurrentScope() const {
  if (allowinput) {
    return KEYSCOPE_INPUT;
  }
  return KEYSCOPE_ALL;
}

void RawCommandScreen::fixCopyReadPos() {
  unsigned int rownum = allowinput ? row - 1 : row;
  unsigned int copysize = rawbuf->getCopySize();
  if (rownum >= copysize) {
    copyreadpos = 0;
  }
  else if (copyreadpos + rownum > copysize) {
    copyreadpos = copysize - rownum;
  }
}
