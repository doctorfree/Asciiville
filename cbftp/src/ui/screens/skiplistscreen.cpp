#include "skiplistscreen.h"

#include "../../globalcontext.h"
#include "../../util.h"

#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectoptioncontainer.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptiontextarrow.h"
#include "../resizableelement.h"
#include "../misc.h"

namespace {

enum KeyAction {
  KEYACTION_INSERT_ROW,
  KEYACTION_MOVE_DOWN,
  KEYACTION_MOVE_UP
};

enum KeyScopes {
  KEYSCOPE_IN_TABLE
};
}

SkipListScreen::SkipListScreen(Ui* ui) : UIWindow(ui, "SkipListScreen"), base(*vv), table(*vv, false) {
  keybinds.addScope(KEYSCOPE_IN_TABLE, "In the skiplist table");
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Navigate left");
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Navigate right");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete row", KEYSCOPE_IN_TABLE);
  keybinds.addBind(KEY_IC, KEYACTION_INSERT_ROW, "Add row before", KEYSCOPE_IN_TABLE);
  keybinds.addBind('m', KEYACTION_MOVE_DOWN, "Move down", KEYSCOPE_IN_TABLE);
  keybinds.addBind('o', KEYACTION_MOVE_UP, "Move up", KEYSCOPE_IN_TABLE);
  keybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line", KEYSCOPE_IN_TABLE);
  table.makeLeavableUp();
}

SkipListScreen::~SkipListScreen() {

}

void SkipListScreen::initialize(unsigned int row, unsigned int col, SkipList * skiplist) {
  globalskip = !skiplist;
  this->skiplist = globalskip ? global->getSkipList() : skiplist;
  initialize();
  init(row, col);
}

void SkipListScreen::initialize() {
  active = false;
  table.reset();
  base.reset();
  base.enterFocusFrom(0);
  std::list<SkiplistItem>::const_iterator it;
  testskiplist.clearEntries();
  if (globalskip) {
    testskiplist.setGlobalSkip(NULL);
  }
  else {
    testskiplist.setGlobalSkip(skiplist);
  }
  for (it = skiplist->entriesBegin(); it != skiplist->entriesEnd(); it++) {
    testskiplist.addEntry(it->matchRegex(), it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->getAction());
  }
  focusedarea = &base;
  int y = globalskip ? 4 : 5;
  base.addTextButtonNoContent(y++, 1, "add", "<Add pattern>");
  testpattern = base.addStringField(y, 1, "testpattern", "Test pattern:", "", false, 16, 256);
  testtype = base.addTextArrow(y, 34, "testtype", "Test type:");
  testtype->addOption("File", 0);
  testtype->addOption("Dir", 1);
  testtype->setOption(0);
  testinspreadjob = base.addCheckBox(++y, 1, "testinspreadjob", "Test in spread job:", true);
  currentviewspan = 0;
  temphighlightline = false;
  recreateTable();
  init(row, col);
}

void SkipListScreen::recreateTable() {
  table.clear();
  int y = globalskip ? 8 : 9;
  std::shared_ptr<ResizableElement> re;
  std::shared_ptr<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  re = table.addTextButton(y, 1, "regex", "REGEX");
  re->setSelectable(false);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = table.addTextButton(y, 2, "pattern", "PATTERN");
  re->setSelectable(false);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
  re = table.addTextButton(y, 3, "file", "FILE");
  re->setSelectable(false);
  msal->addElement(re, 3, RESIZE_REMOVE);
  re = table.addTextButton(y, 4, "dir", "DIR");
  re->setSelectable(false);
  msal->addElement(re, 4, RESIZE_REMOVE);
  re = table.addTextButton(y, 5, "action", "ACTION");
  re->setSelectable(false);
  msal->addElement(re, 5, RESIZE_REMOVE);
  re = table.addTextButton(y, 6, "scope", "SCOPE");
  re->setSelectable(false);
  msal->addElement(re, 6, RESIZE_REMOVE);
  std::list<SkiplistItem>::const_iterator it;
  for (it = testskiplist.entriesBegin(); it != testskiplist.entriesEnd(); it++) {
    y++;
    addPatternLine(y, it->matchRegex(), it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->getAction());
  }
}

void SkipListScreen::redraw() {
  vv->clear();
  int y = 1;
  if (!globalskip) {
    vv->putStr(y++, 1, "This skiplist is local and will fall through to the global skiplist if no match is found.");
  }
  vv->putStr(y++, 1, "Valid wildcard expressions are * and ?, unless regex is used.");
  vv->putStr(y++, 1, "The pattern list is parsed from top to bottom and the first match applies. Case insensitive.");
  y += 5;
  unsigned int listspan = vv->getActualRealRows() - y - 1;
  if (testskiplist.size()) {
    base.makeLeavableDown();
  }
  else {
    base.makeLeavableDown(false);
  }
  table.adjustLines(col - 3);
  table.checkPointer();
  unsigned int ypos = table.getLineIndex(table.getAdjustableLine(table.getElement(table.getSelectionPointer()))) - 1;
  adaptViewSpan(currentviewspan, listspan, ypos, table.linesSize() - 1);
  bool highlight;
  for (unsigned int i = 0; i < base.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = base.getElement(i);
    highlight = base.getSelectionPointer() == i && &base == focusedarea;
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  std::shared_ptr<MenuSelectAdjustableLine> highlightline;
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    unsigned int lineindex = table.getLineIndex(table.getAdjustableLine(re));
    ypos = lineindex - 1;
    if (lineindex > 0 && (ypos < currentviewspan || ypos >= currentviewspan + listspan)) {
      continue;
    }
    highlight = table.getSelectionPointer() == i && &table == focusedarea;
    if (re->isVisible()) {
      vv->putStr(re->getRow() - (lineindex ? currentviewspan : 0), re->getCol(), re->getContentText(), highlight);
    }
    if (highlight && temphighlightline) {
      highlightline = table.getAdjustableLine(re);
    }
  }
  if (highlightline) {
    std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol(true);
    vv->highlightOn(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
  }
  printSlider(vv, vv->getActualRealRows(), y + 1, vv->getActualRealCols() - 1, testskiplist.size(), currentviewspan);
  std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
  if (!!msoe) {
    int cursorpos = msoe->cursorPosition();
    if (active && cursorpos >= 0) {
      ui->showCursor();
      unsigned int cursorcol = msoe->getCol() + cursorpos;
      unsigned int cursorrow = msoe->getRow();
      if (focusedarea == &base) {
        cursorcol += msoe->getLabelText().length() + 1;
      }
      else {
        cursorrow -= currentviewspan;
      }
      vv->moveCursor(cursorrow, cursorcol);
    }
    else {
      ui->hideCursor();
    }
  }

  if (col > testtype->getCol() + 20) {
    std::string empty(col - (testtype->getCol() + 20), ' ');
    vv->putStr(testtype->getRow(), testtype->getCol() + 20, empty);
  }
  if (testpattern->getData().length() > 0) {
    std::string allowstring;
    SkipListMatch match = testskiplist.check(testpattern->getData(), testtype->getData(), testinspreadjob->getData());
    if (match.action == SKIPLIST_ALLOW) {
      allowstring = "ALLOWED";
    }
    else if (match.action == SKIPLIST_DENY) {
      allowstring = "DENIED";
    }
    else if (match.action == SKIPLIST_UNIQUE) {
      allowstring = "UNIQUE";
    }
    else if (match.action == SKIPLIST_SIMILAR) {
      allowstring = "SIMILAR";
    }
    vv->putStr(testtype->getRow(), testtype->getCol() + 20, allowstring);
    std::string matchstring = "Match: " + (match.matched ? match.matchpattern : "no match (allowed)");
    vv->putStr(testtype->getRow(), testtype->getCol() + 30, matchstring);
  }
}

bool SkipListScreen::keyPressed(unsigned int ch) {
  int scope = getCurrentScope();
  int action = keybinds.getKeyAction(ch, scope);
  if (temphighlightline) {
    temphighlightline = false;
    ui->redraw();
    if (action == KEYACTION_HIGHLIGHT_LINE) {
      return true;
    }
  }
  unsigned int pagerows = (row > 8 ? row - 8 : 2) * 0.6;
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      if (activeelement->getIdentifier() == "pattern") {
        std::shared_ptr<MenuSelectAdjustableLine> line = table.getAdjustableLine(activeelement);
        if (line) {
          std::shared_ptr<MenuSelectOptionCheckBox> regexbox = std::static_pointer_cast<MenuSelectOptionCheckBox>(line->getElement(0));
          if (regexbox->getData()) {
            try {
              util::regexParse(std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->getData());
            }
            catch (std::regex_error&) {
              ui->update();
              ui->goInfo("Invalid regular expression.");
              regexbox->setValue(false);
              saveToTempSkipList();
              return true;
            }
          }
        }
      }
      saveToTempSkipList();
      ui->redraw();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->redraw();
    return true;
  }
  bool activation;
  switch(action) {
    case KEYACTION_UP: {
      bool moved = keyUp();
      ui->redraw();
      return moved;
    }
    case KEYACTION_DOWN: {
      bool moved = keyDown();
      ui->redraw();
      return moved;
    }
    case KEYACTION_NEXT_PAGE: {
      bool moved = false;
      for (unsigned int i = 0; i < pagerows; i++) {
        moved = keyDown() || moved;
      }
      ui->redraw();
      return moved;
    }
    case KEYACTION_PREVIOUS_PAGE: {
      bool moved = false;
      for (unsigned int i = 0; i < pagerows; i++) {
        moved = keyUp() || moved;
      }
      ui->redraw();
      return moved;
    }
    case KEYACTION_LEFT: {
      bool moved = focusedarea->goLeft();
      ui->redraw();
      return moved;
    }
    case KEYACTION_RIGHT: {
      bool moved = focusedarea->goRight();
      ui->redraw();
      return moved;
    }
    case KEYACTION_ENTER:
      activation = focusedarea->activateSelected();
      if (!activation) {
        std::shared_ptr<MenuSelectOptionElement> element = focusedarea->getElement(focusedarea->getSelectionPointer());
        if (element->getIdentifier() == "add") {
          addPatternLine(0, false, "", false, false, SCOPE_IN_RACE, SKIPLIST_ALLOW);
          saveToTempSkipList();
          recreateTable();
          ui->redraw();
          return true;
        }
        if (element->getIdentifier() == "regex") {
          std::shared_ptr<MenuSelectOptionCheckBox> regexbox = std::static_pointer_cast<MenuSelectOptionCheckBox>(element);
          if (regexbox->getData()) {
            std::shared_ptr<MenuSelectAdjustableLine> line = table.getAdjustableLine(element);
            if (line) {
              try {
                util::regexParse(std::static_pointer_cast<MenuSelectOptionTextField>(line->getElement(1))->getData());
              }
              catch (std::regex_error&) {
                ui->goInfo("Invalid regular expression.");
                regexbox->setValue(false);
                saveToTempSkipList();
                return true;
              }
            }
          }
        }
        saveToTempSkipList();
        ui->redraw();
        return true;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      ui->redraw();
      ui->setLegend();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DONE:
      skiplist->clearEntries();
      for (std::list<SkiplistItem>::const_iterator it = testskiplist.entriesBegin(); it != testskiplist.entriesEnd(); it++) {
        skiplist->addEntry(it->matchRegex(), it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->getAction());
      }
      ui->returnToLast();
      return true;
    case KEYACTION_DELETE:
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        table.removeAdjustableLine(msal);
        saveToTempSkipList();
        table.checkPointer();
        if (testskiplist.size() == 0) {
          table.goUp();
          focusedarea = &base;
          focusedarea->enterFocusFrom(2);
          ui->setLegend();
        }
        recreateTable();
        ui->redraw();
      }
      return true;
    case KEYACTION_INSERT_ROW:
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        addPatternLine(0, false, "", false, false, SCOPE_IN_RACE, SKIPLIST_ALLOW, msal);
        saveToTempSkipList();
        recreateTable();
        ui->redraw();
      }
      return true;
    case KEYACTION_MOVE_UP:
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        if (table.getLineIndex(msal) > 1 && table.swapLineWithPrevious(msal)) {
          table.goUp();
          saveToTempSkipList();
          recreateTable();
          ui->redraw();
        }
      }
      return true;
    case KEYACTION_MOVE_DOWN:
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        if (table.swapLineWithNext(msal)) {
          table.goDown();
          saveToTempSkipList();
          recreateTable();
          ui->redraw();
        }
      }
      return true;
    case KEYACTION_HIGHLIGHT_LINE:
      if (focusedarea != &table) {
        break;
      }
      temphighlightline = true;
      ui->redraw();
      return true;
  }
  return false;
}

bool SkipListScreen::keyUp() {
  bool moved = focusedarea->goUp();
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    focusedarea = &base;
    focusedarea->enterFocusFrom(2);
    focusedarea->goLeft();
    ui->setLegend();
  }
  return moved;
}

bool SkipListScreen::keyDown() {
  bool moved = focusedarea->goDown();
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    focusedarea = &table;
    focusedarea->enterFocusFrom(0);
    ui->setLegend();
  }
  return moved;
}

std::string SkipListScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary(getCurrentScope());
}

std::string SkipListScreen::getInfoLabel() const {
  if (globalskip) {
    return "CONFIGURE GLOBAL SKIPLIST";
  }
  else {
    return "CONFIGURE LOCAL SKIPLIST";
  }
}

void SkipListScreen::saveToTempSkipList() {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it;
  testskiplist.clearEntries();
  for (it = table.linesBegin(); it != table.linesEnd(); it++) {
    if (it == table.linesBegin()) {
      continue;
    }
    bool regex = std::static_pointer_cast<MenuSelectOptionCheckBox>((*it)->getElement(0))->getData();
    std::string pattern = std::static_pointer_cast<MenuSelectOptionTextField>((*it)->getElement(1))->getData();
    bool file = std::static_pointer_cast<MenuSelectOptionCheckBox>((*it)->getElement(2))->getData();
    bool dir = std::static_pointer_cast<MenuSelectOptionCheckBox>((*it)->getElement(3))->getData();
    SkipListAction action = static_cast<SkipListAction>(std::static_pointer_cast<MenuSelectOptionTextArrow>((*it)->getElement(4))->getData());
    int scope = std::static_pointer_cast<MenuSelectOptionTextArrow>((*it)->getElement(5))->getData();
    testskiplist.addEntry(regex, pattern, file, dir, scope, action);
  }
}

void SkipListScreen::addPatternLine(int y, bool regex, std::string pattern, bool file, bool dir,
                                    int scope, SkipListAction action,
                                    std::shared_ptr<MenuSelectAdjustableLine> before)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal;
  if (!before) {
    msal = table.addAdjustableLine();
  }
  else {
    msal = table.addAdjustableLineBefore(before);
  }
  std::shared_ptr<ResizableElement> re = table.addCheckBox(y, 1, "regex", "", regex);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = table.addStringField(y, 2, "pattern", "", pattern, false, 256);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
  re = table.addCheckBox(y, 3, "filebox", "", file);
  msal->addElement(re, 3, RESIZE_REMOVE);
  re = table.addCheckBox(y, 4, "dirbox", "", dir);
  msal->addElement(re, 4, RESIZE_REMOVE);
  std::shared_ptr<MenuSelectOptionTextArrow> msota = table.addTextArrow(y, 5, "actionarrow", "");
  msota->addOption("Allow", SKIPLIST_ALLOW);
  msota->addOption("Deny", SKIPLIST_DENY);
  msota->addOption("Unique", SKIPLIST_UNIQUE);
  msota->addOption("Similar", SKIPLIST_SIMILAR);
  msota->setOption(static_cast<int>(action));
  msal->addElement(msota, 5, RESIZE_REMOVE);
  msota = table.addTextArrow(y, 6, "scope", "");
  msota->addOption("In spread job", SCOPE_IN_RACE);
  msota->addOption("Allround", SCOPE_ALL);
  msota->setOption(scope);
  msal->addElement(msota, 6, RESIZE_REMOVE);
}

int SkipListScreen::getCurrentScope() const {
  if (focusedarea == &table) {
    return KEYSCOPE_IN_TABLE;
  }
  return KEYSCOPE_ALL;
}
