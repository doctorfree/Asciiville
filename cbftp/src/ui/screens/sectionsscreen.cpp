#include "sectionsscreen.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "../../globalcontext.h"
#include "../../section.h"
#include "../../sectionmanager.h"
#include "../../skiplist.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../util.h"

namespace {

bool sectionNameCompare(const Section * a, const Section * b) {
  return a->getName().compare(b->getName()) < 0;
}

enum KeyAction {
  KEYACTION_SELECT,
  KEYACTION_ADD_SECTION,
  KEYACTION_DETAILS,
  KEYACTION_RETURN_SELECT,
  KEYACTION_RETURN_SELECT2
};

enum KeyScope {
  KEYSCOPE_SELECT,
  KEYSCOPE_EDIT
};

}

SectionsScreen::SectionsScreen(Ui* ui) : UIWindow(ui, "SectionsScreen"), table(*vv) {
  keybinds.addScope(KEYSCOPE_SELECT, "When selecting sections");
  keybinds.addScope(KEYSCOPE_EDIT, "When editing sections");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
  keybinds.addBind(10, KEYACTION_RETURN_SELECT, "Return selected items", KEYSCOPE_SELECT);
  keybinds.addBind('d', KEYACTION_RETURN_SELECT2, "Return selected items", KEYSCOPE_SELECT);
  keybinds.addBind(' ', KEYACTION_SELECT, "Select", KEYSCOPE_SELECT);
  keybinds.addBind('t', KEYACTION_TOGGLE_ALL, "Toggle all", KEYSCOPE_SELECT);
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel", KEYSCOPE_SELECT);
  keybinds.addBind('A', KEYACTION_ADD_SECTION, "Add section", KEYSCOPE_EDIT);
  keybinds.addBind(10, KEYACTION_DETAILS, "Details", KEYSCOPE_EDIT);
  keybinds.addBind('d', KEYACTION_DONE, "Return", KEYSCOPE_EDIT);
  keybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete section", KEYSCOPE_EDIT);
}

SectionsScreen::~SectionsScreen() {

}

void SectionsScreen::initialize(unsigned int row, unsigned int col, bool selectsection, const std::list<std::string>& preselected) {
  mode = selectsection ? Mode::SELECT : Mode::EDIT;
  currentviewspan = 0;
  ypos = 0;
  temphighlightline = false;
  hascontents = false;
  table.reset();
  table.enterFocusFrom(0);
  selected.clear();
  for (const std::string& section : preselected) {
    if (global->getSectionManager()->getSection(section)) {
      selected.insert(section);
    }
  }
  togglestate = false;
  init(row, col);
}

void SectionsScreen::redraw() {
  vv->clear();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  totallistsize = global->getSectionManager()->size();
  table.reset();
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addSectionTableHeader(y++, table);
  while (ypos && ypos >= totallistsize) {
    --ypos;
  }
  unsigned int pos = 0;
  std::vector<const Section *> sections;
  for (auto it = global->getSectionManager()->begin(); it != global->getSectionManager()->end(); ++it) {
    sections.push_back(&it->second);
  }
  std::sort(sections.begin(), sections.end(), sectionNameCompare);
  for (auto it = sections.begin(); it != sections.end() && y < row; ++it) {
    if (pos >= currentviewspan) {
      addSectionDetails(y++, table, **it);
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
  }
  table.checkPointer();
  hascontents = table.linesSize() > 1;
  table.adjustLines(col - 3);
  std::shared_ptr<MenuSelectAdjustableLine> highlightline;
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    bool highlight = hascontents && table.getSelectionPointer() == i;
    if (re->isVisible()) {
      vv->putStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
    if (highlight && (temphighlightline ^ ui->getHighlightEntireLine())) {
      highlightline = table.getAdjustableLine(re);
    }
  }
  if (highlightline) {
    std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
    vv->highlightOn(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
  }
  printSlider(vv, row, 1, col - 1, totallistsize, currentviewspan);
}

bool SectionsScreen::keyPressed(unsigned int ch) {
  int scope = getCurrentScope();
  int action = keybinds.getKeyAction(ch, scope);
  if (temphighlightline) {
    temphighlightline = false;
    ui->redraw();
    if (action == KEYACTION_HIGHLIGHT_LINE) {
      return true;
    }
  }
  if (hascontents && mode == Mode::SELECT && (action == KEYACTION_RETURN_SELECT || action == KEYACTION_RETURN_SELECT2)) {
    std::string selectedstr;
    if (selected.empty()) {
      std::shared_ptr<MenuSelectOptionTextButton> elem =
          std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
      selectedstr = elem->getLabelText();
    }
    else {
      selectedstr = util::join(selected, ",");
    }
    ui->returnSelectItems(selectedstr);
    return true;
  }
  switch (action) {
    case KEYACTION_UP:
      if (hascontents && ypos > 0) {
        --ypos;
        table.goUp();
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_DOWN:
      if (hascontents && ypos < totallistsize - 1) {
        ++ypos;
        table.goDown();
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_NEXT_PAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos < totallistsize - 1; i++) {
        ypos++;
        table.goDown();
      }
      ui->update();
      return true;
    }
    case KEYACTION_PREVIOUS_PAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos > 0; i++) {
        ypos--;
        table.goUp();
      }
      ui->update();
      return true;
    }
    case KEYACTION_TOP:
      ypos = 0;
      ui->update();
      return true;
    case KEYACTION_SELECT:
      if (hascontents && mode == Mode::SELECT) {
        std::shared_ptr<MenuSelectOptionTextButton> elem =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        std::string section = elem->getLabelText();
        if (selected.find(section) == selected.end()) {
          selected.insert(section);
        }
        else {
          selected.erase(section);
        }
        ui->redraw();
        ui->setLegend();
        return true;
      }
      break;
    case KEYACTION_BOTTOM:
      ypos = totallistsize - 1;
      ui->update();
      return true;
    case KEYACTION_DETAILS:
     if (hascontents && mode == Mode::EDIT) {
       std::shared_ptr<MenuSelectOptionTextButton> elem =
           std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
       ui->goEditSection(elem->getLabelText());
     }
     return true;
    case KEYACTION_ADD_SECTION:
      if (mode == Mode::EDIT) {
        ui->goAddSection();
      }
      return true;
    case KEYACTION_DELETE:
      if (hascontents && mode == Mode::EDIT) {
        std::shared_ptr<MenuSelectOptionTextButton> elem =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        ui->goConfirmation("Are you sure that you wish to remove this section: " + elem->getLabelText());
      }
      return true;
    case KEYACTION_DONE:
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_HIGHLIGHT_LINE:
      if (!hascontents) {
        break;
      }
      temphighlightline = true;
      ui->redraw();
      return true;
    case KEYACTION_TOGGLE_ALL:
      if (selected.empty() || !togglestate) {
        selected.clear();
        for (auto it = global->getSectionManager()->begin(); it != global->getSectionManager()->end(); ++it) {
          selected.insert(it->first);
        }
        togglestate = true;
      }
      else if (selected.size() == global->getSectionManager()->size() || togglestate) {
        selected.clear();
        togglestate = false;
      }
      ui->redraw();
      ui->setLegend();
      return true;
  }
  return false;
}

void SectionsScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    std::shared_ptr<MenuSelectOptionTextButton> elem =
        std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
    global->getSectionManager()->removeSection(elem->getLabelText());
  }
  ui->redraw();
}

std::string SectionsScreen::getLegendText() const {
  return keybinds.getLegendSummary(getCurrentScope());
}

std::string SectionsScreen::getInfoLabel() const {
  if (mode == Mode::SELECT) {
    return "SELECT SECTIONS";
  }
  return "SECTIONS";
}

std::string SectionsScreen::getInfoText() const {
  return "Total: " + std::to_string(totallistsize);
}

void SectionsScreen::addSectionTableHeader(unsigned int y, MenuSelectOption & mso) {
  addSectionTableRow(y, mso, false, "SEL", "NAME", "SKIPLIST", "HOTKEY", "#JOBS", "#SITES", "SITES");
}

void SectionsScreen::addSectionDetails(unsigned int y, MenuSelectOption & mso, const Section & section) {
  std::string skiplist = section.getSkipList().size() ? "[X]" : "[ ]";
  std::string selectedstr = (selected.find(section.getName()) != selected.end()) ? "[X]" : "[ ]";
  std::list<std::string> sites;
  for (auto it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); ++it) {
    if ((*it)->hasSection(section.getName())) {
      sites.push_back((*it)->getName());
    }
  }
  int hotkey = section.getHotKey();
  std::string hotkeystr = hotkey != -1 ? std::to_string(hotkey) : "None";
  addSectionTableRow(y, mso, true, selectedstr, section.getName(), skiplist, hotkeystr, std::to_string(section.getNumJobs()),
                     std::to_string(sites.size()), util::join(sites, ","));
}

void SectionsScreen::addSectionTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
    const std::string& selected, const std::string & name, const std::string & skiplist, const std::string & hotkey, const std::string & numjobs,
    const std::string & numsites, const std::string & sites)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  if (mode == Mode::SELECT) {
    msotb = mso.addTextButtonNoContent(y, 1, "selected", selected);
    msal->addElement(msotb, 6, RESIZE_CUTEND);
  }

  msotb = mso.addTextButtonNoContent(y, 1, "name", name);
  msal->addElement(msotb, 7, RESIZE_CUTEND);

  msotb = mso.addTextButtonNoContent(y, 1, "skiplist", skiplist);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "hotkey", hotkey);
  msotb->setSelectable(false);
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "numjobs", numjobs);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "numsites", numsites);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sites", sites);
  msotb->setSelectable(false);
  msal->addElement(msotb, 1, RESIZE_WITHDOTS);
}

int SectionsScreen::getCurrentScope() const {
  return mode == Mode::EDIT ? KEYSCOPE_EDIT : KEYSCOPE_SELECT;
}
