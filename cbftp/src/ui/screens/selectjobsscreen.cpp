#include "selectjobsscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptioncheckbox.h"
#include "../misc.h"

#include "../../commandowner.h"
#include "../../race.h"
#include "../../transferjob.h"
#include "../../engine.h"
#include "../../globalcontext.h"
#include "../../util.h"

namespace {

enum KeyAction {
  KEYACTION_SELECT
};

}

SelectJobsScreen::SelectJobsScreen(Ui* ui) : UIWindow(ui, "SelectJobsScreen"), table(*vv) {
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind({10, ' '}, KEYACTION_SELECT, "Select");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
}

void SelectJobsScreen::initialize(unsigned int row, unsigned int col, JobType type) {
  hascontents = false;
  currentviewspan = 0;
  ypos = 0;
  numselected = 0;
  this->type = type;

  table.reset();
  unsigned int y = 0;
  addJobTableHeader(y++, table, "JOB NAME");
  if (type == JOBTYPE_SPREADJOB) {
    totallistsize = global->getEngine()->allRaces();
    for (std::list<std::shared_ptr<Race> >::const_iterator it = --global->getEngine()->getRacesEnd();
        it != --global->getEngine()->getRacesBegin(); --it, ++y)
    {
      addJobDetails(y, table, *it);
    }
  }
  else {
    totallistsize = global->getEngine()->allTransferJobs();
    for (std::list<std::shared_ptr<TransferJob> >::const_iterator it = --global->getEngine()->getTransferJobsEnd();
        it != --global->getEngine()->getTransferJobsBegin(); --it, ++y)
    {
      addJobDetails(y, table, *it);
    }
  }
  table.checkPointer();
  init(row, col);
}

void SelectJobsScreen::redraw() {
  vv->clear();
  unsigned int listspan = row - 1;
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);
  table.checkPointer();
  hascontents = table.linesSize() > 1;
  table.adjustLines(col - 3);
  bool highlight;
  int y = 0;
  for (unsigned int i = 0; i < table.linesSize(); i++) {
    if (i != 0 && (i < currentviewspan + 1 || i > currentviewspan + listspan)) {
      continue;
    }
    std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(i);
    for (unsigned int j = 0; j < msal->size(); ++j) {
      std::shared_ptr<ResizableElement> re = msal->getElement(j);
      highlight = false;
      if (j == 1 && ypos + 1 == currentviewspan + y && hascontents) {
        highlight = true;
      }
      if (re->isVisible()) {
        vv->putStr(y, re->getCol(), re->getLabelText(), highlight);
      }
    }
    y++;
  }
  printSlider(vv, row, 1, col - 1, totallistsize, currentviewspan);
}

bool SelectJobsScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
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
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_SELECT:
      if (hascontents) {
        std::shared_ptr<MenuSelectOptionTextButton> msotb =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        std::shared_ptr<MenuSelectAdjustableLine> line = table.getAdjustableLine(msotb);
        std::shared_ptr<MenuSelectOptionCheckBox> checkbox = std::static_pointer_cast<MenuSelectOptionCheckBox>(line->getElement(0));
        checkbox->activate();
        if (checkbox->getData()) {
          numselected++;
          checkbox->setLabel("[X]");
        }
        else {
          numselected--;
          checkbox->setLabel("[ ]");
        }
      }
      ui->setInfo();
      ui->update();
      return true;
    case KEYACTION_DONE:
    {
      std::list<std::string> items;
      for (unsigned int i = 0; i < table.size(); i++) {
        std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
        if (re->getIdentifier() == "selected" && std::static_pointer_cast<MenuSelectOptionCheckBox>(re)->getData()) {
          std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(re);
          std::shared_ptr<MenuSelectOptionTextButton> name = std::static_pointer_cast<MenuSelectOptionTextButton>(msal->getElement(1));
          items.push_back(std::to_string(name->getId()));
        }
      }
      ui->returnSelectItems(util::join(items, ","));
      return true;
    }
  }
  return false;
}

std::string SelectJobsScreen::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::string SelectJobsScreen::getInfoLabel() const {
  return std::string("SELECT ") + (type == JOBTYPE_SPREADJOB ? "SPREAD" : "TRANSFER") + " JOBS";
}

std::string SelectJobsScreen::getInfoText() const {
  return "Selected: " + std::to_string(numselected);
}

void SelectJobsScreen::addJobTableHeader(unsigned int y, MenuSelectOption & mso, const std::string & release) {
  addTableRow(y, mso, -1, false, "JOB NAME", "USE");
}

void SelectJobsScreen::addTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id, bool selectable,
    const std::string & name, const std::string & checkboxtext)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<ResizableElement> re;

  re = mso.addCheckBox(y,  1, "selected",  checkboxtext, false);
  re->setSelectable(false);
  msal->addElement(re, 2, 1, RESIZE_REMOVE, false);

  re = mso.addTextButton(y, 1, "name", name);
  re->setSelectable(selectable);
  re->setId(id);
  msal->addElement(re, 1, 1, RESIZE_CUTEND, true);
}

void SelectJobsScreen::addJobDetails(unsigned int y, MenuSelectOption & mso, std::shared_ptr<Race> job) {
  addTableRow(y, mso, job->getId(), true, job->getName(), "[ ]");
}

void SelectJobsScreen::addJobDetails(unsigned int y, MenuSelectOption & mso, std::shared_ptr<TransferJob> job) {
  addTableRow(y, mso, job->getId(), true, job->getName(), "[ ]");
}
