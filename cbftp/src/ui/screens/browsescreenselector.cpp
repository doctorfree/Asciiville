#include "browsescreenselector.h"

#include <memory>

#include "../../sitemanager.h"
#include "../../site.h"
#include "../../globalcontext.h"
#include "../../localstorage.h"

#include "../termint.h"
#include "../ui.h"
#include "../menuselectoptiontextbutton.h"
#include "../resizableelement.h"
#include "../misc.h"
#include "../keybinds.h"

#include "browsescreenaction.h"

BrowseScreenSelector::BrowseScreenSelector(Ui * ui, KeyBinds& keybinds) :
  BrowseScreenSub(keybinds),
  ui(ui),
  focus(true),
  table(ui->getVirtualView()),
  pointer(0),
  currentviewspan(0),
  gotomode(false)
{
  vv = &ui->getVirtualView();
  SiteManager * sm = global->getSiteManager();
  std::vector<std::shared_ptr<Site> >::const_iterator it;
  entries.push_back(std::pair<std::string, std::string>(BROWSESCREENSELECTOR_HOME,
                                                        global->getLocalStorage()->getDownloadPath().toString()));
  entries.push_back(std::pair<std::string, std::string>("", ""));
  for (it = sm->begin(); it != sm->end(); it++) {
    entries.push_back(std::pair<std::string, std::string>((*it)->getName(), (*it)->getName()));
  }
}

BrowseScreenSelector::~BrowseScreenSelector() {

}

BrowseScreenType BrowseScreenSelector::type() const {
  return BrowseScreenType::SELECTOR;
}

void BrowseScreenSelector::redraw(unsigned int row, unsigned int col, unsigned int coloffset) {
  this->row = row;
  this->col = col;
  this->coloffset = coloffset;
  table.clear();
  adaptViewSpan(currentviewspan, row, pointer, entries.size());

  int y = 0;
  for (unsigned int i = currentviewspan; i < currentviewspan + row && i < entries.size(); i++) {
    std::shared_ptr<MenuSelectOptionTextButton> msotb =
        table.addTextButtonNoContent(y++, coloffset + 1, entries[i].first, entries[i].second);
    if (entries[i].first == "") {
      msotb->setSelectable(false);
    }
    if (i == pointer) {
      table.setPointer(msotb);
    }
  }
  table.checkPointer();
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(i));
    bool highlight = false;
    if (table.getSelectionPointer() == i) {
      highlight = true;
    }
    vv->putStr(msotb->getRow(), msotb->getCol(), msotb->getLabelText(), highlight && focus);
  }
  printSlider(vv, row, coloffset + col - 1, entries.size(), currentviewspan);
}

void BrowseScreenSelector::update() {
  if (table.size()) {
    if (pointer < currentviewspan || pointer >= currentviewspan + row) {
      ui->redraw();
      return;
    }
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(table.getLastSelectionPointer()));
    vv->putStr(re->getRow(), re->getCol(), re->getLabelText());
    re = std::static_pointer_cast<ResizableElement>(table.getElement(table.getSelectionPointer()));
    vv->putStr(re->getRow(), re->getCol(), re->getLabelText(), focus);
  }
}

BrowseScreenAction BrowseScreenSelector::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  unsigned int pagerows = (unsigned int) row * 0.6;
  if (gotomode) {
    if (ch >= 32 && ch <= 126) {
      for (unsigned int i = 0; i < entries.size(); i++) {
        const std::string & label = entries[i].second;
        if (!label.empty() && toupper(ch) == toupper(label[0])) {
          pointer = i;
          ui->redraw();
          break;
        }
      }
    }
    gotomode = false;
    ui->update();
    ui->setLegend();
    return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
  }
  switch (action) {
    case KEYACTION_BACK_CANCEL: // esc
      ui->returnToLast();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    case KEYACTION_CLOSE:
      return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
    case KEYACTION_UP:
      if (table.goUp()) {
        pointer = table.getElement(table.getSelectionPointer())->getRow() + currentviewspan;
        ui->update();
      }
      else if (pointer > 0) {
        pointer--;
        ui->redraw();
      }
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    case KEYACTION_DOWN:
      if (table.goDown()) {
        pointer = table.getElement(table.getSelectionPointer())->getRow() + currentviewspan;
        ui->update();
      }
      else if (pointer < entries.size()) {
        pointer++;
        ui->redraw();
      }
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    case KEYACTION_TOP:
      pointer = 0;
      ui->redraw();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    case KEYACTION_BOTTOM:
      pointer = entries.size() - 1;
      ui->redraw();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    case KEYACTION_NEXT_PAGE:
      if (pagerows < entries.size() && pointer < entries.size() - 1 - pagerows) {
        pointer += pagerows;
      }
      else {
        pointer = entries.size() - 1;
      }
      ui->redraw();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    case KEYACTION_PREVIOUS_PAGE:
      if (pointer > pagerows) {
        pointer -= pagerows;
      }
      else {
        pointer = 0;
      }
      ui->redraw();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    case KEYACTION_RIGHT:
    case KEYACTION_ENTER:
    case KEYACTION_BROWSE:
    {
      std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
      if (msotb->getIdentifier() == BROWSESCREENSELECTOR_HOME) {
        return BrowseScreenAction(BROWSESCREENACTION_HOME);
      }
      else {
        return BrowseScreenAction(BROWSESCREENACTION_SITE, msotb->getIdentifier());
      }
      break;
    }
    case KEYACTION_QUICK_JUMP:
      gotomode = true;
      ui->update();
      ui->setLegend();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
  }
  return BrowseScreenAction();
}

std::string BrowseScreenSelector::getLegendText(int scope) const {
  if (gotomode) {
    return "[Any] Go to matching first letter in site list - [Esc] Cancel";
  }
  return keybinds.getLegendSummary(scope);
}

std::string BrowseScreenSelector::getInfoLabel() const {
  return "SELECT SITE";
}

std::string BrowseScreenSelector::getInfoText() const {
  return "select site or local";
}

void BrowseScreenSelector::setFocus(bool focus) {
  this->focus = focus;
}

BrowseScreenAction BrowseScreenSelector::tryJumpSection(const std::string& section) {
  std::shared_ptr<MenuSelectOptionTextButton> msotb = std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
  if (msotb->getIdentifier() != BROWSESCREENSELECTOR_HOME) {
    return BrowseScreenAction(BROWSESCREENACTION_SITE, msotb->getIdentifier());
  }
  return BrowseScreenAction(BROWSESCREENACTION_NOOP);
}
