#include "rawdatascreen.h"

#include "../../ftpconn.h"
#include "../../rawbuffer.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../globalcontext.h"
#include "../../encoding.h"
#include "../../util.h"

#include "../ui.h"
#include "../termint.h"

namespace {

enum KeyAction {
  KEYACTION_CLEAR,
  KEYACTION_CONNECT,
  KEYACTION_DISCONNECT,
  KEYACTION_RAW_COMMAND
};

enum KeyScopes {
  KEYSCOPE_INPUT,
  KEYSCOPE_NOT_INPUT
};

}

RawDataScreen::RawDataScreen(Ui* ui) : UIWindow(ui, "RawDataScreen") {
  keybinds.addScope(KEYSCOPE_INPUT, "When entering command");
  keybinds.addScope(KEYSCOPE_NOT_INPUT, "When not entering command");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Scroll up");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Scroll down");
  keybinds.addBind(TERMINT_CTRL_L, KEYACTION_CLEAR, "Clear log");
  keybinds.addBind(10, KEYACTION_ENTER, "Send command", KEYSCOPE_INPUT);
  keybinds.addBind(27, KEYACTION_BACK_CANCEL, "Clear/Cancel", KEYSCOPE_INPUT);
  keybinds.addBind(KEY_UP, KEYACTION_UP, "History up", KEYSCOPE_INPUT);
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "History down", KEYSCOPE_INPUT);
  keybinds.addBind('w', KEYACTION_RAW_COMMAND, "Raw command", KEYSCOPE_NOT_INPUT);
  keybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Previous screen", KEYSCOPE_NOT_INPUT);
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Next screen", KEYSCOPE_NOT_INPUT);
  keybinds.addBind(10, KEYACTION_BACK_CANCEL, "Return", KEYSCOPE_NOT_INPUT);
  keybinds.addBind('c', KEYACTION_CONNECT, "Connect", KEYSCOPE_NOT_INPUT);
  keybinds.addBind('d', KEYACTION_DISCONNECT, "Disconnect", KEYSCOPE_NOT_INPUT);
  keybinds.addBind('f', KEYACTION_FILTER, "Toggle filtering", KEYSCOPE_NOT_INPUT);
  keybinds.addBind('F', KEYACTION_FILTER_REGEX, "Toggle regex filtering", KEYSCOPE_NOT_INPUT);
}

void RawDataScreen::initialize(unsigned int row, unsigned int col, std::string sitename, int connid) {
  this->sitename = sitename;
  this->connid = connid;
  expectbackendpush = true;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitename);
  threads = sitelogic->getConns()->size();
  this->rawbuf = sitelogic->getConn(connid)->getRawBuffer();
  rawbuf->setUiWatching(true);
  readfromcopy = false;
  rawcommandmode = false;
  rawcommandswitch = false;
  copyreadpos = 0;
  filtermodeinput = false;
  filtermodeinputregex = false;
  init(row, col);
}

void RawDataScreen::redraw() {
  vv->clear();
  unsigned int rownum = row;
  if (rawcommandmode) {
    --rownum;
    std::string oldtext = rawcommandfield.getData();
    rawcommandfield = MenuSelectOptionTextField("rawcommand", row-1, 10, "", oldtext, col-15, 65536, false);
  }
  fixCopyReadPos();
  std::string oldtext = filterfield.getData();
  filterfield = MenuSelectOptionTextField("filter", row - 1, 1, "", oldtext, col - 20, 512, false);
  update();
}

void RawDataScreen::update() {
  if (rawcommandswitch) {
    if (rawcommandmode) {
      rawcommandmode = false;
      ui->hideCursor();
    }
    else {
      rawcommandmode = true;
      ui->showCursor();
    }
    rawcommandswitch = false;
    redraw();
    return;
  }
  vv->clear();
  unsigned int rownum = row;
  if (rawcommandmode) {
    rownum = row - 1;
  }
  else if (filtermodeinput || filtermodeinputregex) {
    rownum = row - 2;
  }
  printRawBufferLines(vv, rawbuf, rownum, col, 0, readfromcopy, copyreadpos);
  if (rawcommandmode) {
    std::string pretag = "[Raw command]: ";
    vv->putStr(rownum, 0, pretag + rawcommandfield.getContentText());
    vv->moveCursor(rownum, pretag.length() + rawcommandfield.cursorPosition());
  }
  if (filtermodeinput || filtermodeinputregex) {
    ui->showCursor();
    std::string pretag = filtermodeinput ? "[Filter(s)]: " : "[Regex filter]: ";
    vv->putStr(filterfield.getRow(), filterfield.getCol(), pretag + filterfield.getContentText());
    vv->moveCursor(filterfield.getRow(), filterfield.getCol() + pretag.length() + filterfield.cursorPosition());
  }
}

void RawDataScreen::printRawBufferLines(VirtualView* vv, RawBuffer * rawbuf, unsigned int rownum, unsigned int col, unsigned int coloffset) {
  printRawBufferLines(vv, rawbuf, rownum, col, coloffset, false, 0);
}

void RawDataScreen::printRawBufferLines(VirtualView* vv, RawBuffer * rawbuf, unsigned int rownum, unsigned int col, unsigned int coloffset, bool readfromcopy, unsigned int copyreadpos) {
  bool cutfirst5 = false;
  bool skiptag = false;
  bool skiptagchecked = false;
  unsigned int size = rawbuf->isFiltered() ? rawbuf->getFilteredSize() : rawbuf->getSize();
  if (readfromcopy) {
    size = rawbuf->isFiltered() ? rawbuf->getFilteredCopySize() : rawbuf->getCopySize();
  }
  unsigned int numlinestoprint = size < rownum ? size : rownum;
  for (unsigned int i = 0; i < numlinestoprint; ++i) {
    unsigned int pos = numlinestoprint - i - 1;
    const std::pair<std::string, std::string>& line = readfromcopy ?
        (rawbuf->isFiltered() ? rawbuf->getFilteredLineCopy(pos + copyreadpos) :
        rawbuf->getLineCopy(pos + copyreadpos))
     : (rawbuf->isFiltered() ? rawbuf->getFilteredLine(pos) : rawbuf->getLine(pos));
    if (!skiptagchecked) {
      if (col <= 80 + line.first.length()) {
        skiptag = true;
      }
      skiptagchecked = true;
    }
    if (!cutfirst5 && line.second.length() > col && skipCodePrint(line.second)) {
      cutfirst5 = true;
    }
    if (skiptagchecked && cutfirst5) {
      break;
    }
  }
  for (unsigned int i = 0; i < numlinestoprint; i++) {
    unsigned int pos = numlinestoprint - i - 1;
    const std::pair<std::string, std::string>& line = readfromcopy ?
        (rawbuf->isFiltered() ? rawbuf->getFilteredLineCopy(pos + copyreadpos) :
        rawbuf->getLineCopy(pos + copyreadpos))
     : (rawbuf->isFiltered() ? rawbuf->getFilteredLine(pos) : rawbuf->getLine(pos));
    unsigned int startprintsecond = 0;
    if (!skiptag) {
      unsigned int length = line.first.length();
      vv->putStr(i, coloffset, line.first, false, col - coloffset);
      startprintsecond = length + 1;
    }
    unsigned int start = 0;
    if (cutfirst5 && skipCodePrint(line.second)) {
      start = 5;
    }
    vv->putStr(i, startprintsecond + coloffset, encoding::cp437toUnicode(line.second.substr(start)), false, col - startprintsecond);
  }
}

bool RawDataScreen::skipCodePrint(const std::string & line) {
  return line.length() >= 5 && (line.substr(0, 5) == "230- " || line.substr(0, 5) == "200- ");
}

bool RawDataScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch, rawcommandmode ? KEYSCOPE_INPUT : KEYSCOPE_NOT_INPUT);
  if (filtermodeinput || filtermodeinputregex) {
    if (filterfield.inputChar(ch)) {
      ui->update();
      return true;
    }
    else if (ch == 10) {
      ui->hideCursor();
      filtertext = filterfield.getData();
      if (filtertext.length()) {
        if (filtermodeinput) {
          rawbuf->setWildcardFilters(util::trim(util::split(filtertext)));
        }
        else {
          try {
            std::regex regexfilter = util::regexParse(filtertext);
            rawbuf->setRegexFilter(regexfilter);
          }
          catch (std::regex_error& e) {
            ui->goInfo("Invalid regular expression.");
            return true;
          }
        }
        if (readfromcopy) {
          copyreadpos = 0;
        }
      }
      filtermodeinput = false;
      filtermodeinputregex = false;
      ui->redraw();
      ui->setLegend();
      return true;
    }
    else if (ch == 27) {
      if (!filterfield.getData().empty()) {
        filterfield.clear();
        ui->update();
      }
      else {
        filtermodeinput = false;
        filtermodeinputregex = false;
        ui->hideCursor();
        ui->redraw();
        ui->setLegend();
      }
      return true;
    }
    else if (ch == '\t') {
      filtermodeinput = !filtermodeinput;
      filtermodeinputregex = !filtermodeinputregex;
      ui->update();
      ui->setLegend();
      return true;
    }
  }
  unsigned int rownum = row;
  if (action == KEYACTION_CLEAR) {
    rawbuf->clear();
    readfromcopy = false;
    ui->redraw();
    return true;
  }
  if (rawcommandmode) {
    rownum = row - 1;
    if (rawcommandfield.inputChar(ch))
    {
      ui->update();
      return true;
    }
    else if (action == KEYACTION_ENTER) {
      std::string command = rawcommandfield.getData();
      if (command == "") {
        rawcommandswitch = true;
        ui->update();
        ui->setLegend();
        return true;
      }
      else {
        readfromcopy = false;
        history.push(command);
        sitelogic->issueRawCommand(connid, command);
        rawcommandfield.clear();
      }
      ui->update();
      return true;
    }
    else if (action == KEYACTION_BACK_CANCEL) {
      if (rawcommandfield.getData() != "") {
        rawcommandfield.clear();
      }
      else {
        rawcommandswitch = true;
        ui->update();
        ui->setLegend();
        return true;
      }
      ui->update();
      return true;
    }
    else if (action == KEYACTION_UP) {
      if (history.canBack()) {
        if (history.current()) {
          history.setCurrent(rawcommandfield.getData());
        }
        history.back();
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    }
    else if (action == KEYACTION_DOWN) {
      if (history.forward()) {
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    }
  }
  switch(action) {
    case KEYACTION_RIGHT:
      if (connid + 1 < threads) {
        rawbuf->setUiWatching(false);
        rawbuf->unsetFilters();
        ui->goRawDataJump(sitename, connid + 1);
      }
      return true;
    case KEYACTION_LEFT:
      rawbuf->unsetFilters();
      rawbuf->setUiWatching(false);
      if (connid == 0) {
        ui->returnToLast();
      }
      else {
        ui->goRawDataJump(sitename, connid - 1);
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
    case KEYACTION_CONNECT:
      sitelogic->connectConn(connid);
      return true;
    case KEYACTION_DISCONNECT:
      sitelogic->disconnectConn(connid);
      return true;
    case KEYACTION_RAW_COMMAND:
      rawcommandswitch = true;
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_FILTER:
      if (rawbuf->isFiltered()) {
        rawbuf->unsetFilters();
        ui->setInfo();
        if (readfromcopy) {
          copyreadpos = 0;
        }
      }
      else {
        filtermodeinput = true;
        ui->setLegend();
      }
      ui->redraw();
      return true;
    case KEYACTION_FILTER_REGEX:
      if (rawbuf->isFiltered()) {
        rawbuf->unsetFilters();
        ui->setInfo();
        if (readfromcopy) {
          copyreadpos = 0;
        }
      }
      else {
        filtermodeinputregex = true;
        ui->setLegend();
      }
      ui->redraw();
      return true;
    case KEYACTION_BACK_CANCEL: // esc
      rawbuf->setUiWatching(false);
      rawbuf->unsetFilters();
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string RawDataScreen::getLegendText() const {
  if (filtermodeinput) {
    return "[Any] Enter space separated filters. Valid operators are !, *, ?. Must match all negative filters and at least one positive if given. Case insensitive. - [Tab] switch mode - [Esc] Cancel";
  }
  if (filtermodeinputregex) {
    return "[Any] Enter regex input - [Tab] switch mode - [Esc] Cancel";
  }
  return keybinds.getLegendSummary(rawcommandmode ? KEYSCOPE_INPUT : KEYSCOPE_NOT_INPUT);
}

std::string RawDataScreen::getInfoLabel() const {
  return "RAW DATA: " + sitename + " #" + std::to_string(connid);
}

std::string RawDataScreen::getInfoText() const {
  if (rawbuf->isFiltered()) {
    return "FILTERING ON: " + filtertext;
  }
  return "";
}

void RawDataScreen::fixCopyReadPos() {
  unsigned int rownum = rawcommandmode ? row - 1 : row;
  unsigned int copysize = rawbuf->isFiltered() ? rawbuf->getFilteredCopySize() : rawbuf->getCopySize();
  if (rownum >= copysize) {
    copyreadpos = 0;
  }
  else if (copyreadpos + rownum > copysize) {
    copyreadpos = copysize - rownum;
  }
}
