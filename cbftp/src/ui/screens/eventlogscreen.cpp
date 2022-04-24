#include "eventlogscreen.h"

#include "../../eventlog.h"
#include "../../rawbuffer.h"
#include "../../globalcontext.h"
#include "../../util.h"

#include "../ui.h"

EventLogScreen::EventLogScreen(Ui* ui) : UIWindow(ui, "EventLogScreen") {
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Scroll up");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Scroll down");
  keybinds.addBind(10, KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind('f', KEYACTION_FILTER, "Toggle filtering");
  keybinds.addBind('F', KEYACTION_FILTER_REGEX, "Toggle regex filtering");
}

void EventLogScreen::initialize(unsigned int row, unsigned int col) {
  expectbackendpush = true;
  this->rawbuf = global->getEventLog()->getRawBuffer();
  rawbuf->setUiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  filtermodeinput = false;
  filtermodeinputregex = false;
  init(row, col);
}

void EventLogScreen::redraw() {
  rawbuf->bookmark();
  vv->clear();
  unsigned int rows = (filtermodeinput || filtermodeinputregex) ? vv->getActualRealRows() - 2 : vv->getActualRealRows();
  while (true) {
    vv->clear();
    std::list<std::string> printlines;
    fixCopyReadPos();
    unsigned int size = rawbuf->isFiltered() ? rawbuf->getFilteredSize() : rawbuf->getSize();
    if (readfromcopy) {
      size = rawbuf->isFiltered() ? rawbuf->getFilteredCopySize() : rawbuf->getCopySize();
    }
    unsigned int numlinestoprint = size < rows ? size : rows;
    for (unsigned int i = 0; i < numlinestoprint; ++i) {
      unsigned int pos = numlinestoprint - i - 1;
      const std::pair<std::string, std::string> & entry = readfromcopy ?
          (rawbuf->isFiltered() ? rawbuf->getFilteredLineCopy(pos + copyreadpos) :
          rawbuf->getLineCopy(pos + copyreadpos))
       : (rawbuf->isFiltered() ? rawbuf->getFilteredLine(pos) : rawbuf->getLine(pos));
      std::string line = entry.first + " " + entry.second;
      vv->putStr(i, 0, line);
    }
    unsigned int newrows = (filtermodeinput || filtermodeinputregex) ? vv->getActualRealRows() - 2 : vv->getActualRealRows();
    if (rows <= newrows) { // check if the rendering wrote a too long line and therefore caused the horizontal slider to show up,
                           // which reduces the number of available rows by 1
      break;
    }
    rows = newrows;
  }
  std::string oldtext = filterfield.getData();
  filterfield = MenuSelectOptionTextField("filter", vv->getActualRealRows() - 1, 1, "", oldtext, vv->getActualRealCols() - 20, 512, false);
  update();
}

void EventLogScreen::update() {
  if (rawbuf->linesSinceBookmark()) {
    redraw();
    return;
  }
  if (filtermodeinput || filtermodeinputregex) {
    ui->showCursor();
    std::string pretag = filtermodeinput ? "[Filter(s)]: " : "[Regex filter]: ";
    vv->putStr(filterfield.getRow(), filterfield.getCol(), pretag + filterfield.getContentText());
    vv->moveCursor(filterfield.getRow(), filterfield.getCol() + pretag.length() + filterfield.cursorPosition());
  }
}

bool EventLogScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
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
  switch(action) {
    case KEYACTION_PREVIOUS_PAGE:
      if (!readfromcopy) {
        rawbuf->freezeCopy();
        copyreadpos = 0;
        readfromcopy = true;
      }
      else {
        copyreadpos = copyreadpos + vv->getActualRealRows() / 2;
        fixCopyReadPos();
      }
      ui->redraw();
      return true;
    case KEYACTION_NEXT_PAGE:
      if (readfromcopy) {
        if (copyreadpos == 0) {
          readfromcopy = false;
        }
        else if (copyreadpos < vv->getActualRealRows() / 2) {
          copyreadpos = 0;
        }
        else {
          copyreadpos = copyreadpos - vv->getActualRealRows() / 2;
        }
      }
      ui->redraw();
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
    case KEYACTION_BACK_CANCEL:
      rawbuf->setUiWatching(false);
      rawbuf->unsetFilters();
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string EventLogScreen::getLegendText() const {
  if (filtermodeinput) {
    return "[Any] Enter space separated filters. Valid operators are !, *, ?. Must match all negative filters and at least one positive if given. Case insensitive. - [Tab] switch mode - [Esc] Cancel";
  }
  if (filtermodeinputregex) {
    return "[Any] Enter regex input - [Tab] switch mode - [Esc] Cancel";
  }
  return keybinds.getLegendSummary();
}

std::string EventLogScreen::getInfoLabel() const {
  return "EVENT LOG";
}

std::string EventLogScreen::getInfoText() const {
  if (rawbuf->isFiltered()) {
    return "FILTERING ON: " + filtertext;
  }
  return "";
}

void EventLogScreen::fixCopyReadPos() {
  unsigned int copysize = rawbuf->isFiltered() ? rawbuf->getFilteredCopySize() : rawbuf->getCopySize();
  if (vv->getActualRealRows() >= copysize) {
    copyreadpos = 0;
  }
  else if (copyreadpos + vv->getActualRealRows() > copysize) {
    copyreadpos = copysize - vv->getActualRealRows();
  }
}
