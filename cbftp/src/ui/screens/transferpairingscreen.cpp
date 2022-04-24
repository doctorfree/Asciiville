#include "transferpairingscreen.h"

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

TransferPairingScreen::TransferPairingScreen(Ui* ui) : UIWindow(ui, "TransferPairingScreen"), table(*vv) {
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

TransferPairingScreen::~TransferPairingScreen() {

}

void TransferPairingScreen::initialize(unsigned int row, unsigned int col, TransferPairing* transferpairing) {
  this->transferpairing = transferpairing;
  initialize();
  init(row, col);
}

void TransferPairingScreen::initialize() {
  active = false;
  table.reset();
  currentviewspan = 0;
  temphighlightline = false;
  init(row, col);
}

void TransferPairingScreen::redraw() {
  vv->clear();
  int y = 1;
  unsigned int listspan = row - y - 1;
  table.clear();
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
  table.adjustLines(col - 3);
  table.checkPointer();
  unsigned int ypos = table.getLineIndex(table.getAdjustableLine(table.getElement(table.getSelectionPointer()))) - 1;
  adaptViewSpan(currentviewspan, listspan, ypos, table.linesSize() - 1);
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = table.getElement(i);
    bool highlight = table.getSelectionPointer() == i;
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
    bool highlight = table.getSelectionPointer() == i;
    if (re->isVisible()) {
      vv->putStr(re->getRow() - (lineindex ? currentviewspan : 0), re->getCol(), re->getContentText(), highlight);
    }
    if (highlight && (temphighlightline ^ ui->getHighlightEntireLine())) {
      highlightline = table.getAdjustableLine(re);
    }
  }
  if (highlightline) {
    std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
    vv->highlightOn(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
  }
  printSlider(vv, row, 1, col - 1, 0, currentviewspan);
  std::shared_ptr<MenuSelectOptionElement> msoe = table.getElement(table.getSelectionPointer());
  if (!!msoe) {
    int cursorpos = msoe->cursorPosition();
    if (active && cursorpos >= 0) {
      ui->showCursor();
      unsigned int cursorcol = msoe->getCol() + cursorpos;
      unsigned int cursorrow = msoe->getRow();
      cursorrow -= currentviewspan;
      vv->moveCursor(cursorrow, cursorcol);
    }
    else {
      ui->hideCursor();
    }
  }
}

bool TransferPairingScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
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
              return true;
            }
          }
        }
      }
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
      table.goUp();
      ui->update();
      return true;
    case KEYACTION_DOWN:
      table.goDown();
      ui->update();
      return true;
    case KEYACTION_NEXT_PAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        table.goDown();
      }
      ui->redraw();
      return true;
    case KEYACTION_PREVIOUS_PAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        table.goUp();
      }
      ui->redraw();
      return true;
    case KEYACTION_LEFT:
      table.goLeft();
      ui->update();
      return true;
    case KEYACTION_RIGHT:
      table.goRight();
      ui->update();
      return true;
    case KEYACTION_ENTER:
      activation = table.activateSelected();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = table.getElement(table.getSelectionPointer());
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DONE:
      transferpairing->clear();
      ui->returnToLast();
      return true;
    case KEYACTION_DELETE: {
      std::shared_ptr<MenuSelectOptionElement> msoe = table.getElement(table.getSelectionPointer());
      std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
      table.removeAdjustableLine(msal);
      table.checkPointer();
      ui->redraw();
      return true;
    }
    case KEYACTION_INSERT_ROW: {
      std::shared_ptr<MenuSelectOptionElement> msoe = table.getElement(table.getSelectionPointer());
      std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
      //addPatternLine(0);
      ui->redraw();
      return true;
    }
    case KEYACTION_MOVE_UP: {
      std::shared_ptr<MenuSelectOptionElement> msoe = table.getElement(table.getSelectionPointer());
      std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
      if (table.getLineIndex(msal) > 1 && table.swapLineWithPrevious(msal)) {
        table.goUp();
        ui->redraw();
      }
      return true;
    }
    case KEYACTION_MOVE_DOWN: {
      std::shared_ptr<MenuSelectOptionElement> msoe = table.getElement(table.getSelectionPointer());
      std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
      if (table.swapLineWithNext(msal)) {
        table.goDown();
        ui->redraw();
      }
      return true;
    }
    case KEYACTION_HIGHLIGHT_LINE:
      temphighlightline = true;
      ui->redraw();
      return true;
  }
  return false;
}

std::string TransferPairingScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string TransferPairingScreen::getInfoLabel() const {
  return "CONFIGURE TRANSFER PAIRING";
}
