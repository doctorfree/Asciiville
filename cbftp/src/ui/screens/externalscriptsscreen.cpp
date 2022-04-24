#include "externalscriptsscreen.h"

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

#include "../../externalscripts.h"

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

ExternalScriptsScreen::ExternalScriptsScreen(Ui* ui) : UIWindow(ui, "ExternalScriptsScreen"), base(*vv), table(*vv),
    tempexternalscripts(std::make_shared<ExternalScripts>())
{
  keybinds.addScope(KEYSCOPE_IN_TABLE, "In the command table");
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

ExternalScriptsScreen::~ExternalScriptsScreen() {

}

void ExternalScriptsScreen::initialize(unsigned int row, unsigned int col, ExternalScripts* externalscripts, const std::string& description) {
  this->externalscripts = externalscripts;
  this->description = util::split(description, "\n");
  active = false;
  table.reset();
  base.reset();
  base.enterFocusFrom(0);
  std::list<ExternalScript>::const_iterator it;
  tempexternalscripts->clear();
  for (it = externalscripts->begin(); it != externalscripts->end(); it++) {
    tempexternalscripts->addScript(it->name, it->path, it->id);
  }
  focusedarea = &base;
  int y = this->description.size() + 2;
  base.addTextButtonNoContent(y++, 1, "add", "<Add script>");
  temphighlightline = false;
  recreateTable();
  init(row, col);
}

void ExternalScriptsScreen::recreateTable() {
  table.clear();
  int y = description.size() + 4;
  std::shared_ptr<ResizableElement> re;
  std::shared_ptr<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  re = table.addTextButton(y, 1, "name", "NAME");
  re->setSelectable(false);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = table.addTextButton(y, 2, "path", "PATH");
  re->setSelectable(false);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
  std::list<ExternalScript>::const_iterator it;
  for (it = tempexternalscripts->begin(); it != tempexternalscripts->end(); it++) {
    y++;
    addScriptLine(y, it->name, it->path, it->id);
  }
}

void ExternalScriptsScreen::redraw() {
  vv->clear();
  int y = 1;
  for (const std::string& line : description) {
    vv->putStr(y++, 1, line);
  }
  if (tempexternalscripts->size()) {
    base.makeLeavableDown();
  }
  else {
    base.makeLeavableDown(false);
  }
  table.adjustLines(col - 3);
  table.checkPointer();
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
    highlight = table.getSelectionPointer() == i && &table == focusedarea;
    if (re->isVisible()) {
      vv->putStr(re->getRow(), re->getCol(), re->getContentText(), highlight);
    }
    if (highlight && temphighlightline) {
      highlightline = table.getAdjustableLine(re);
    }
  }
  if (highlightline) {
    std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol(true);
    vv->highlightOn(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
  }
  std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
  if (!!msoe) {
    int cursorpos = msoe->cursorPosition();
    if (active && cursorpos >= 0) {
      ui->showCursor();
      unsigned int cursorcol = msoe->getCol() + cursorpos;
      unsigned int cursorrow = msoe->getRow();
      vv->moveCursor(cursorrow, cursorcol);
    }
    else {
      ui->hideCursor();
    }
  }
}

bool ExternalScriptsScreen::keyPressed(unsigned int ch) {
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
          addScriptLine(0, "example", "scripts/example.sh");
          saveToTempScriptList();
          recreateTable();
        }
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
      saveToTempScriptList();
      externalscripts->clear();
      for (std::list<ExternalScript>::const_iterator it = tempexternalscripts->begin(); it != tempexternalscripts->end(); it++) {
        externalscripts->addScript(it->name, it->path, it->id);
      }
      ui->returnToLast();
      return true;
    case KEYACTION_DELETE:
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        table.removeAdjustableLine(msal);
        table.checkPointer();
        if (tempexternalscripts->size() == 0) {
          table.goUp();
          focusedarea = &base;
          focusedarea->enterFocusFrom(2);
          ui->setLegend();
        }
        saveToTempScriptList();
        recreateTable();
        ui->redraw();
      }
      return true;
    case KEYACTION_INSERT_ROW:
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        addScriptLine(0, "example", "scripts/example.sh", -1, msal);
        saveToTempScriptList();
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
          saveToTempScriptList();
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
          saveToTempScriptList();
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

bool ExternalScriptsScreen::keyUp() {
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

bool ExternalScriptsScreen::keyDown() {
  bool moved = focusedarea->goDown();
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    focusedarea = &table;
    focusedarea->enterFocusFrom(0);
    ui->setLegend();
  }
  return moved;
}

std::string ExternalScriptsScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary(getCurrentScope());
}

std::string ExternalScriptsScreen::getInfoLabel() const {
  return "CONFIGURE EXTERNAL SCRIPTS FOR: " + externalscripts->getName();
}

void ExternalScriptsScreen::saveToTempScriptList() {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it;
  tempexternalscripts->clear();
  for (it = table.linesBegin(); it != table.linesEnd(); it++) {
    if (it == table.linesBegin()) {
      continue;
    }
    std::string name = std::static_pointer_cast<MenuSelectOptionTextField>((*it)->getElement(0))->getData();
    Path path = std::static_pointer_cast<MenuSelectOptionTextField>((*it)->getElement(1))->getData();
    int id = (*it)->getElement(0)->getId();
    tempexternalscripts->addScript(name, path, id);
  }
}

void ExternalScriptsScreen::addScriptLine(int y, const std::string& name, const Path& path, int id,
                                    const std::shared_ptr<MenuSelectAdjustableLine>& before)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal;
  if (!before) {
    msal = table.addAdjustableLine();
  }
  else {
    msal = table.addAdjustableLineBefore(before);
  }
  std::shared_ptr<ResizableElement> re = table.addStringField(y, 1, "name", "", name, false, 8, 32);
  re->setId(id);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = table.addStringField(y, 2, "path", "", path.toString(), false, 64, 256);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
}

int ExternalScriptsScreen::getCurrentScope() const {
  if (focusedarea == &table) {
    return KEYSCOPE_IN_TABLE;
  }
  return KEYSCOPE_ALL;
}
