#include "sitesectionsscreen.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "../../path.h"
#include "../../site.h"

namespace {

bool sectionNameCompare(const std::pair<std::string, Path> a, const std::pair<std::string, Path> b) {
  return a.first.compare(b.first) < 0;
}

enum KeyAction {
  KEYACTION_DETAILS,
  KEYACTION_ADD_SECTION
};

}

SiteSectionsScreen::SiteSectionsScreen(Ui* ui) : UIWindow(ui, "SiteSectionsScreen"), table(*vv) {
  keybinds.addBind(10, KEYACTION_DETAILS, "Details");
  keybinds.addBind('A', KEYACTION_ADD_SECTION, "Add section");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete section");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
  keybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
}

SiteSectionsScreen::~SiteSectionsScreen() {

}

void SiteSectionsScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site) {
  this->site = site;
  sitecopy = std::make_shared<Site>(*site.get());
  currentviewspan = 0;
  ypos = 0;
  temphighlightline = false;
  hascontents = false;
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void SiteSectionsScreen::redraw() {
  vv->clear();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  totallistsize = sitecopy->sectionsSize();
  table.reset();
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addSectionTableHeader(y++, table);
  while (ypos && ypos >= totallistsize) {
    --ypos;
  }
  unsigned int pos = 0;
  std::vector<std::pair<std::string, Path>> sections;
  for (auto it = sitecopy->sectionsBegin(); it != sitecopy->sectionsEnd(); ++it) {
    sections.push_back(*it);
  }
  std::sort(sections.begin(), sections.end(), sectionNameCompare);
  for (auto it = sections.begin(); it != sections.end() && y < row; ++it) {
    if (pos >= currentviewspan) {
      addSectionDetails(y++, table, it->first, it->second);
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

bool SiteSectionsScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (temphighlightline) {
    temphighlightline = false;
    ui->redraw();
    if (action == KEYACTION_HIGHLIGHT_LINE) {
      return true;
    }
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
    case KEYACTION_NEXT_PAGE:
    {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos < totallistsize - 1; i++) {
        ypos++;
        table.goDown();
      }
      ui->update();
      return true;
    }
    case KEYACTION_PREVIOUS_PAGE:
    {
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
    case KEYACTION_BOTTOM:
      ypos = totallistsize - 1;
      ui->update();
      return true;
    case KEYACTION_DETAILS:
     if (hascontents) {
       std::shared_ptr<MenuSelectOptionTextButton> elem =
           std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
       ui->goEditSiteSection(sitecopy, elem->getLabelText());
     }
     return true;
    case KEYACTION_ADD_SECTION:
      ui->goAddSiteSection(sitecopy);
      return true;
    case KEYACTION_DELETE:
      if (hascontents) {
        std::shared_ptr<MenuSelectOptionTextButton> elem =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        sitecopy->removeSection(elem->getLabelText());
        ui->redraw();
      }
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DONE:
      site->clearSections();
      for (auto it = sitecopy->sectionsBegin(); it != sitecopy->sectionsEnd(); ++it) {
        site->addSection(it->first, it->second.toString());
      }
      ui->returnToLast();
      return true;
    case KEYACTION_HIGHLIGHT_LINE:
      if (!hascontents) {
        break;
      }
      temphighlightline = true;
      ui->redraw();
      return true;
  }
  return false;
}

std::string SiteSectionsScreen::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::string SiteSectionsScreen::getInfoLabel() const {
  return "SITE SECTIONS: " + sitecopy->getName();
}

std::string SiteSectionsScreen::getInfoText() const {
  return "Total: " + std::to_string(totallistsize);
}

void SiteSectionsScreen::addSectionTableHeader(unsigned int y, MenuSelectOption & mso) {
  addSectionTableRow(y, mso, false, "NAME", "PATH");
}

void SiteSectionsScreen::addSectionDetails(unsigned int y, MenuSelectOption & mso, const std::string & section, const Path & path) {
  addSectionTableRow(y, mso, true, section, path.toString());
}

void SiteSectionsScreen::addSectionTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
    const std::string & name, const std::string & path)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "name", name);
  msal->addElement(msotb, 6, RESIZE_CUTEND);

  msotb = mso.addTextButtonNoContent(y, 1, "path", path);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);
}
