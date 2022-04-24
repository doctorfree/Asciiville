#include "allracesscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../misc.h"

#include "../../race.h"
#include "../../engine.h"
#include "../../globalcontext.h"
#include "../../util.h"

namespace {

enum KeyAction {
  KEYACTION_RESET_HARD,
  KEYACTION_ABORT_DELETE_INC,
  KEYACTION_ABORT_DELETE_ALL
};

}

AllRacesScreen::AllRacesScreen(Ui* ui) : UIWindow(ui, "AllRacesScreen"), table(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Details");
  keybinds.addBind('r', KEYACTION_RESET, "Reset job");
  keybinds.addBind('R', KEYACTION_RESET_HARD, "Hard reset job");
  keybinds.addBind('B', KEYACTION_ABORT, "Abort job");
  keybinds.addBind('t', KEYACTION_TRANSFERS, "Transfers for job");
  keybinds.addBind('z', KEYACTION_ABORT_DELETE_INC, "Abort and delete own files on incomplete sites");
  keybinds.addBind('Z', KEYACTION_ABORT_DELETE_ALL, "Abort and delete own files on ALL sites");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
  keybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
}

void AllRacesScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  hascontents = false;
  currentviewspan = 0;
  ypos = 0;
  temphighlightline = false;
  engine = global->getEngine();
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void AllRacesScreen::redraw() {
  vv->clear();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  unsigned int totallistsize = engine->allRaces();
  table.reset();
  while (ypos && ypos >= engine->allRaces()) {
    --ypos;
  }
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addRaceTableHeader(y, table, "RELEASE");
  y++;
  unsigned int pos = 0;
  for (std::list<std::shared_ptr<Race> >::const_iterator it = --engine->getCurrentRacesEnd(); it != --engine->getCurrentRacesBegin() && y < row; it--) {
    if (pos >= currentviewspan) {
      addRaceDetails(y++, table, *it);
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
  }
  for (std::list<std::shared_ptr<Race> >::const_iterator it = --engine->getFinishedRacesEnd(); it != --engine->getFinishedRacesBegin() && y < row; it--) {
    if (pos >= currentviewspan) {
      addRaceDetails(y++, table, *it);
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

void AllRacesScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    if (!!abortrace) {
      global->getEngine()->abortRace(abortrace);
    }
    else if (!!abortdeleteraceinc) {
      global->getEngine()->deleteOnAllSites(abortdeleteraceinc, false, false);
    }
    else if (!!abortdeleteraceall) {
      global->getEngine()->deleteOnAllSites(abortdeleteraceall, false, true);
    }
    ui->update();
  }
  abortrace.reset();
  abortdeleteraceinc.reset();
  abortdeleteraceall.reset();
}

bool AllRacesScreen::keyPressed(unsigned int ch) {
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
      }
      return true;
    case KEYACTION_DOWN:
      if (hascontents && ypos < engine->allRaces() - 1) {
        ++ypos;
        table.goDown();
        ui->update();
      }
      return true;
    case KEYACTION_NEXT_PAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos < engine->allRaces() - 1; i++) {
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
    case KEYACTION_BOTTOM:
      ypos = engine->allRaces() - 1;
      ui->update();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_ENTER:
      if (hascontents) {
        ui->goRaceStatus(table.getElement(table.getSelectionPointer())->getId());
      }
      return true;
    case KEYACTION_ABORT:
      if (hascontents) {
        abortrace = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!abortrace && abortrace->getStatus() == RaceStatus::RUNNING) {
          ui->goConfirmation("Do you really want to abort the spread job " + abortrace->getName());
        }
      }
      return true;
    case KEYACTION_ABORT_DELETE_INC:
      if (hascontents) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!race && race->getStatus() == RaceStatus::RUNNING) {
          abortdeleteraceinc = race;
          ui->goConfirmation("Do you really want to abort the race " + abortdeleteraceinc->getName() + " and delete your own files on all incomplete sites?");
        }
      }
      return true;
    case KEYACTION_ABORT_DELETE_ALL:
      if (hascontents) {
        abortdeleteraceall = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!abortdeleteraceall) {
          if (abortdeleteraceall->getStatus() == RaceStatus::RUNNING) {
            ui->goConfirmation("Do you really want to abort the race " + abortdeleteraceall->getName() + " and delete your own files on ALL involved sites?");
          }
          else {
            ui->goConfirmation("Do you really want to delete your own files in " + abortdeleteraceall->getName() + " on ALL involved sites?");
          }
        }
      }
      return true;
    case KEYACTION_RESET:
    case KEYACTION_RESET_HARD:
      if (hascontents) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!race) {
          global->getEngine()->resetRace(race, action == KEYACTION_RESET_HARD);
        }
      }
      return true;
    case KEYACTION_TRANSFERS:
      if (hascontents) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!race) {
          ui->goTransfersFilterSpreadJob(race->getName());
        }
      }
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

std::string AllRacesScreen::getInfoLabel() const {
  return "ALL SPREAD JOBS";
}

std::string AllRacesScreen::getInfoText() const {
  return "Active: " + std::to_string(engine->currentRaces()) + "  Total: " + std::to_string(engine->allRaces());
}

void AllRacesScreen::addRaceTableHeader(unsigned int y, MenuSelectOption & mso, const std::string & release) {
  addRaceTableRow(y, mso, -1, false, "STARTED", "USE", "SECTION", release, "SIZE", "WORST", "AVG", "BEST", "STATUS", "DONE", "SITES");
}

void AllRacesScreen::addRaceTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id, bool selectable,
    const std::string & timestamp, const std::string & timespent, const std::string & section, const std::string & release,
    const std::string & size, const std::string & worst, const std::string & avg, const std::string & best, const std::string & status, const std::string & done,
    const std::string & sites)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "timestamp", timestamp);
  msotb->setSelectable(false);
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msotb->setSelectable(false);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "section", section);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "release", release);
  msotb->setSelectable(selectable);
  msotb->setId(id);
  msal->addElement(msotb, 12, 1, RESIZE_CUTEND, true);

  msotb = mso.addTextButtonNoContent(y, 1, "size", size);
  msotb->setSelectable(false);
  msal->addElement(msotb, 10, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "worst", worst);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "avg", avg);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "best", best);
  msotb->setSelectable(false);
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "status", status);
  msotb->setSelectable(false);
  msal->addElement(msotb, 11, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "done", done);
  msotb->setSelectable(false);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sites", sites);
  msotb->setSelectable(false);
  msal->addElement(msotb, 0, RESIZE_WITHDOTS);
}

void AllRacesScreen::addRaceDetails(unsigned int y, MenuSelectOption & mso, std::shared_ptr<Race> race) {
  std::string done = std::to_string(race->numSitesDone()) + "/" + std::to_string(race->numSites());
  std::string timespent = util::simpleTimeFormat(race->getTimeSpent());
  std::string status;
  std::string worst = std::to_string(race->getWorstCompletionPercentage()) + "%";
  std::string avg = std::to_string(race->getAverageCompletionPercentage()) + "%";
  std::string best = std::to_string(race->getBestCompletionPercentage()) + "%";
  std::string size = util::parseSize(race->estimatedTotalSize());
  switch (race->getStatus()) {
    case RaceStatus::RUNNING:
      status = "running";
      break;
    case RaceStatus::DONE:
      status = "done";
      break;
    case RaceStatus::ABORTED:
      status = "aborted";
      break;
    case RaceStatus::TIMEOUT:
      status = "timeout";
      break;
  }
  addRaceTableRow(y, mso, race->getId(), true, race->getTimeStamp(), timespent, race->getSection(),
                  race->getName(), size, worst, avg, best, status, done,
                  race->getSiteListText());
}
