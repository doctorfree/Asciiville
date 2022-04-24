#include "mainscreen.h"

#include <cctype>
#include <memory>

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../race.h"
#include "../../engine.h"
#include "../../sitemanager.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../transferjob.h"
#include "../../util.h"
#include "../../remotecommandhandler.h"
#include "../../settingsloadersaver.h"
#include "../../preparedrace.h"
#include "../../sectionmanager.h"
#include "../../section.h"
#include "../../hourlyalltracking.h"

#include <cassert>

#include "../menuselectoptioncheckbox.h"
#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../misc.h"

#include "allracesscreen.h"
#include "alltransferjobsscreen.h"

namespace {

enum KeyActions {
  KEYACTION_ADD_SITE,
  KEYACTION_EDIT_SITE,
  KEYACTION_GLOBAL_SETTINGS,
  KEYACTION_EVENT_LOG,
  KEYACTION_ALL_SPREAD_JOBS,
  KEYACTION_ALL_TRANSFER_JOBS,
  KEYACTION_BROWSE_LOCAL,
  KEYACTION_BROWSE_SPLIT,
  KEYACTION_SECTIONS,
  KEYACTION_SNAKE,
  KEYACTION_RAW_COMMAND,
  KEYACTION_COPY,
  KEYACTION_ABORT_DELETE_INC,
  KEYACTION_ABORT_DELETE_ALL,
  KEYACTION_LOGIN,
  KEYACTION_SCOREBOARD,
  KEYACTION_METRICS,
  KEYACTION_ALL_TRANSFERS,
  KEYACTION_FORCE_DISCONNECT_ALL_SLOTS,
  KEYACTION_DISCONNECT_ALL_SLOTS,
  KEYACTION_REMOVE_SITE_FROM_ALL_SPREADJOBS
};

enum KeyScopes {
  KEYSCOPE_PREPARED_SPREAD_JOB,
  KEYSCOPE_SPREAD_JOB,
  KEYSCOPE_TRANSFER_JOB,
  KEYSCOPE_SITE
};

void putIfClear(VirtualView* vv, unsigned int row, unsigned int col, int c) {
  if (vv->isClear(row, col)) {
    vv->putChar(row, col, c, false);
  }
}

void drawTree(VirtualView* vv) {
  unsigned int row = vv->getActualRealRows();
  unsigned int col = vv->getActualRealCols();
  if (row < 16 || col < 27) {
    return;
  }
  unsigned int startrow = row - 16;
  unsigned int startcol = col - 27;
  putIfClear(vv, startrow, startcol + 12, '*');
  putIfClear(vv, startrow + 6, startcol + 10, 0x25CF);
  putIfClear(vv, startrow + 4, startcol + 13, 0x25CF);
  putIfClear(vv, startrow + 7, startcol + 16, 0x25CF);
  putIfClear(vv, startrow + 8, startcol + 7, 0x25CF);
  putIfClear(vv, startrow + 9, startcol + 12, 0x25CF);
  putIfClear(vv, startrow + 2, startcol + 11, 0x25CF);
  putIfClear(vv, startrow + 10, startcol + 6, 0x25CF);
  putIfClear(vv, startrow + 11, startcol + 15, 0x25CF);
  putIfClear(vv, startrow + 10, startcol + 19, 0x25CF);
  vv->setColor(startrow, startcol + 12, COLOR_WHITE, COLOR_YELLOW);
  vv->setColor(startrow + 1, startcol + 11, COLOR_RED, COLOR_GREEN, 3);
  vv->setColor(startrow + 2, startcol + 10, COLOR_RED, COLOR_GREEN, 5);
  vv->setColor(startrow + 3, startcol + 9, COLOR_RED, COLOR_GREEN, 7);
  vv->setColor(startrow + 4, startcol + 8, COLOR_RED, COLOR_GREEN, 9);
  vv->setColor(startrow + 5, startcol + 7, COLOR_RED, COLOR_GREEN, 11);
  vv->setColor(startrow + 6, startcol + 6, COLOR_RED, COLOR_GREEN, 13);
  vv->setColor(startrow + 7, startcol + 5, COLOR_RED, COLOR_GREEN, 15);
  if (vv->isClear(startrow + 12, startcol, 25)) {
    vv->putStr(startrow + 12, startcol + 6, "Merry xmas &");
    vv->putStr(startrow + 13, startcol + 10, "happy");
    vv->putStr(startrow + 14, startcol + 11, "new");
    vv->putStr(startrow + 15, startcol + 10, "year!");
  }
  vv->setColor(startrow + 8, startcol + 4, COLOR_RED, COLOR_GREEN, 17);
  vv->setColor(startrow + 9, startcol + 3, COLOR_RED, COLOR_GREEN, 19);
  vv->setColor(startrow + 10, startcol + 2, COLOR_RED, COLOR_GREEN, 21);
  vv->setColor(startrow + 11, startcol + 1, COLOR_RED, COLOR_GREEN, 23);
  vv->setColor(startrow + 12, startcol, COLOR_RED, COLOR_GREEN, 25);
  vv->setColor(startrow + 13, startcol + 10, COLOR_BLACK, COLOR_YELLOW, 5);
  vv->setColor(startrow + 14, startcol + 10, COLOR_BLACK, COLOR_YELLOW, 5);
  vv->setColor(startrow + 15, startcol + 10, COLOR_BLACK, COLOR_YELLOW, 5);
}

} // namespace

MainScreen::MainScreen(Ui* ui) : UIWindow(ui, "MainScreen"), msop(*vv), msosj(*vv), msotj(*vv), msos(*vv) {
  keybinds.addScope(KEYSCOPE_PREPARED_SPREAD_JOB, "When a prepared spread job is selected");
  keybinds.addScope(KEYSCOPE_SPREAD_JOB, "When a spread job is selected");
  keybinds.addScope(KEYSCOPE_TRANSFER_JOB, "When a transfer job is selected");
  keybinds.addScope(KEYSCOPE_SITE, "When a site is selected");
  keybinds.addBind('A', KEYACTION_ADD_SITE, "Add site");
  keybinds.addBind('G', KEYACTION_GLOBAL_SETTINGS, "Global settings");
  keybinds.addBind('l', KEYACTION_EVENT_LOG, "Event log");
  keybinds.addBind('t', KEYACTION_ALL_TRANSFERS, "Transfers");
  keybinds.addBind('r', KEYACTION_ALL_SPREAD_JOBS, "All spread jobs");
  keybinds.addBind('j', KEYACTION_ALL_TRANSFER_JOBS, "All transfer jobs");
  keybinds.addBind('q', KEYACTION_QUICK_JUMP, "Quick jump");
  keybinds.addBind('c', KEYACTION_BROWSE_LOCAL, "Browse local");
  keybinds.addBind('i', KEYACTION_INFO, "General info");
  keybinds.addBind('s', KEYACTION_SECTIONS, "Sections");
  keybinds.addBind('S', KEYACTION_SNAKE, "Snake");
  keybinds.addBind('o', KEYACTION_SCOREBOARD, "Scoreboard");
  keybinds.addBind('m', KEYACTION_METRICS, "Metrics");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Previous page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Next page");
  keybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
  keybinds.addBind({'b', KEY_RIGHT}, KEYACTION_BROWSE, "Browse", KEYSCOPE_SITE);
  keybinds.addBind('E', KEYACTION_EDIT_SITE, "Edit", KEYSCOPE_SITE);
  keybinds.addBind('\t', KEYACTION_BROWSE_SPLIT, "Split browse", KEYSCOPE_SITE);
  keybinds.addBind('w', KEYACTION_RAW_COMMAND, "Raw command", KEYSCOPE_SITE);
  keybinds.addBind('C', KEYACTION_COPY, "Copy", KEYSCOPE_SITE);
  keybinds.addBind({'D', KEY_DC}, KEYACTION_DELETE, "Delete", KEYSCOPE_SITE);
  keybinds.addBind(10, KEYACTION_ENTER, "Status", KEYSCOPE_SITE);
  keybinds.addBind('L', KEYACTION_LOGIN, "Login all slots", KEYSCOPE_SITE);
  keybinds.addBind('T', KEYACTION_TRANSFERS, "Transfers", KEYSCOPE_SITE);
  keybinds.addBind('K', KEYACTION_FORCE_DISCONNECT_ALL_SLOTS, "Kill all slots", KEYSCOPE_SITE);
  keybinds.addBind('k', KEYACTION_DISCONNECT_ALL_SLOTS, "Disconnect all slots", KEYSCOPE_SITE);
  keybinds.addBind('X', KEYACTION_REMOVE_SITE_FROM_ALL_SPREADJOBS, "Remove site from all running spreadjobs", KEYSCOPE_SITE);
  keybinds.addBind(10, KEYACTION_ENTER, "Status", KEYSCOPE_SPREAD_JOB);
  keybinds.addBind('B', KEYACTION_ABORT, "Abort", KEYSCOPE_SPREAD_JOB);
  keybinds.addBind('T', KEYACTION_TRANSFERS, "Transfers", KEYSCOPE_SPREAD_JOB);
  keybinds.addBind('R', KEYACTION_RESET, "Reset", KEYSCOPE_SPREAD_JOB);
  keybinds.addBind('z', KEYACTION_ABORT_DELETE_INC, "Abort and delete own files on incomplete sites", KEYSCOPE_SPREAD_JOB);
  keybinds.addBind('Z', KEYACTION_ABORT_DELETE_ALL, "Abort and delete own files on ALL sites", KEYSCOPE_SPREAD_JOB);
  keybinds.addBind('b', KEYACTION_BROWSE, "Browse", KEYSCOPE_TRANSFER_JOB);
  keybinds.addBind(10, KEYACTION_ENTER, "Status", KEYSCOPE_TRANSFER_JOB);
  keybinds.addBind('B', KEYACTION_ABORT, "Abort", KEYSCOPE_TRANSFER_JOB);
  keybinds.addBind('T', KEYACTION_TRANSFERS, "Transfers", KEYSCOPE_TRANSFER_JOB);
  keybinds.addBind('R', KEYACTION_RESET, "Reset", KEYSCOPE_TRANSFER_JOB);
  keybinds.addBind(10, KEYACTION_ENTER, "Start", KEYSCOPE_PREPARED_SPREAD_JOB);
  keybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete", KEYSCOPE_PREPARED_SPREAD_JOB);
}

void MainScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  gotomode = false;
  currentviewspan = 0;
  sitestartrow = 0;
  currentraces = 0;
  currenttransferjobs = 0;
  sitepos = 0;
  awaitingremovesitefromallspreadjobs = false;
  if (global->getEngine()->preparedRaces()) {
    focusedarea = &msop;
    msop.enterFocusFrom(0);
  }
  else if (global->getEngine()->currentRaces()) {
    focusedarea = &msosj;
    msosj.enterFocusFrom(0);
  }
  else if (global->getEngine()->currentTransferJobs()) {
    focusedarea = &msotj;
    msotj.enterFocusFrom(0);
  }
  else {
    focusedarea = &msos;
    msos.enterFocusFrom(0);
  }
  temphighlightline = false;
  init(row, col);
}

void MainScreen::redraw() {
  vv->clear();
  ui->hideCursor();
  totalsitessize = global->getSiteManager()->getNumSites();
  if (sitepos && sitepos >= totalsitessize) {
    --sitepos;
  }
  numsitestext = "Sites: " + std::to_string(totalsitessize);
  int listpreparedraces = global->getEngine()->preparedRaces();
  int listraces = global->getEngine()->allRaces();
  int listtransferjobs = global->getEngine()->allTransferJobs();

  unsigned int irow = 0;
  msop.clear();
  msosj.clear();
  msotj.clear();
  msos.clear();
  if (listpreparedraces) {
    addPreparedRaceTableHeader(irow++, msop);
    std::list<std::shared_ptr<PreparedRace> >::const_iterator it;
    int i = 0;
    for (it = --global->getEngine()->getPreparedRacesEnd(); it != --global->getEngine()->getPreparedRacesBegin() && i < 3; it--, i++) {
      addPreparedRaceDetails(irow++, msop, *it);
    }
    msop.checkPointer();
    msop.adjustLines(col - 3);
    irow++;
  }
  if (listraces) {

    AllRacesScreen::addRaceTableHeader(irow++, msosj, std::string("SPREAD JOB NAME") + (listraces > 3 ? " (Showing latest 3)" : ""));
    std::list<std::shared_ptr<Race> >::const_iterator it;
    int i = 0;
    for (it = --global->getEngine()->getRacesEnd(); it != --global->getEngine()->getRacesBegin() && i < 3; it--, i++) {
      AllRacesScreen::addRaceDetails(irow++, msosj, *it);
    }
    msosj.checkPointer();
    msosj.adjustLines(col - 3);
    irow++;
  }
  if (listtransferjobs) {

    AllTransferJobsScreen::addJobTableHeader(irow++, msotj, std::string("TRANSFER JOB NAME") + (listtransferjobs > 3 ? " (Showing latest 3)" : ""));
    std::list<std::shared_ptr<TransferJob> >::const_iterator it;
    int i = 0;
    for (it = --global->getEngine()->getTransferJobsEnd(); it != --global->getEngine()->getTransferJobsBegin() && i < 3; it--, i++) {
      AllTransferJobsScreen::addJobDetails(irow++, msotj, *it);
    }
    msotj.checkPointer();
    msotj.adjustLines(col - 3);
    irow++;
  }
  msop.makeLeavableDown(listraces || listtransferjobs || totalsitessize);
  msosj.makeLeavableUp(listpreparedraces);
  msosj.makeLeavableDown(listtransferjobs || totalsitessize);
  msotj.makeLeavableUp(listpreparedraces || listraces);
  msotj.makeLeavableDown(totalsitessize);
  msos.makeLeavableUp(listpreparedraces || listraces || listtransferjobs);

  if (!totalsitessize) {
    vv->putStr(irow, 1, "Press 'A' to add a site");
  }
  else {
    int y = irow;
    addSiteHeader(y++, msos);
    sitestartrow = y;
    adaptViewSpan(currentviewspan, row - sitestartrow, sitepos, totalsitessize);
    for (unsigned int i = currentviewspan; (int)i < global->getSiteManager()->getNumSites() && i - currentviewspan < row - sitestartrow; ++i) {
      std::shared_ptr<Site> site = global->getSiteManager()->getSite(i);
      std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site->getName());
      addSiteDetails(y++, msos, sl);
    }

    msos.setPointer(msos.getAdjustableLine(sitepos + 1 - currentviewspan)->getElement(0));
    msos.checkPointer();
    msos.adjustLines(col - 3);

    printSlider(vv, row, sitestartrow, col - 1, totalsitessize, currentviewspan);
  }
  int currentraces = global->getEngine()->currentRaces();
  int currenttransferjobs = global->getEngine()->currentTransferJobs();
  if (currentraces) {
    activeracestext = "Active spread jobs: " + std::to_string(currentraces) + "  ";
  }
  else {
    activeracestext = "";
  }
  if (currenttransferjobs) {
    activejobstext = "Active transfer jobs: " + std::to_string(currenttransferjobs) + "  ";
  }
  else {
    activejobstext = "";
  }
  if (focusedarea == &msop && !msop.size()) {
    msop.reset();
    focusedarea = &msosj;
    focusedarea->enterFocusFrom(0);
  }
  if (focusedarea == &msosj && !msosj.size()) {
    msosj.reset();
    focusedarea = &msotj;
    focusedarea->enterFocusFrom(0);
  }
  if (focusedarea == &msotj && !msotj.size()) {
    msotj.reset();
    focusedarea = &msos;
    focusedarea->enterFocusFrom(0);
  }
  printTable(msop);
  printTable(msosj);
  printTable(msotj);
  printTable(msos);
  ui->setInfo();
  if (ui->getShowXmasTree() && isYearEnd()) {
    drawTree(vv);
  }
}

void MainScreen::printTable(MenuSelectOption & table) {
  std::shared_ptr<MenuSelectAdjustableLine> highlightline;
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    bool highlight = table.isFocused() && table.getSelectionPointer() == i;
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
}

void MainScreen::command(const std::string & command) {
  if (command == "yes") {
    if (!!abortrace) {
      global->getEngine()->abortRace(abortrace);
    }
    else if (!!abortjob) {
      global->getEngine()->abortTransferJob(abortjob);
    }
    else if (!!abortdeleteraceinc) {
      global->getEngine()->deleteOnAllSites(abortdeleteraceinc, false, false);
    }
    else if (!!abortdeleteraceall) {
      global->getEngine()->deleteOnAllSites(abortdeleteraceall, false, true);
    }
    else if (awaitingremovesitefromallspreadjobs) {
      global->getEngine()->removeSiteFromAllRunningSpreadJobs(removesite);
    }
    else {
      global->getSiteLogicManager()->deleteSiteLogic(deletesite);
      global->getSiteManager()->deleteSite(deletesite);
      global->getSettingsLoaderSaver()->saveSettings();
    }
  }
  awaitingremovesitefromallspreadjobs = false;
  abortrace.reset();
  abortjob.reset();
  abortdeleteraceinc.reset();
  abortdeleteraceall.reset();
  ui->redraw();
  ui->setInfo();
}

bool MainScreen::keyUp() {
  if (focusedarea == &msos) {
    if (sitepos) {
      --sitepos;
    }
    else if (msotj.size() || msosj.size() || msop.size()) {
      msos.defocus();
    }
  }
  else {
    focusedarea->goUp();
  }
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    if ((defocusedarea == &msos && !msotj.size() && !msosj.size()) ||
        (defocusedarea == &msotj && !msosj.size()) ||
        defocusedarea == &msosj)
    {
      focusedarea = &msop;
    }
    else if ((defocusedarea == &msos && !msotj.size()) ||
             defocusedarea == &msotj)
    {
      focusedarea = &msosj;
    }
    else {
      focusedarea = &msotj;
    }
    focusedarea->enterFocusFrom(2);
    ui->setLegend();
  }
  return true;
}

bool MainScreen::keyDown() {
  if (focusedarea == &msos) {
    if (sitepos + 1 < totalsitessize) {
      ++sitepos;
    }
  }
  else {
    focusedarea->goDown();
  }
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    if ((defocusedarea == &msop && !msosj.size() && !msotj.size()) ||
        (defocusedarea == &msosj && !msotj.size()) ||
        defocusedarea == &msotj)
    {
      focusedarea = &msos;
    }
    else if ((defocusedarea == &msop && !msosj.size()) ||
             defocusedarea == &msosj)
    {
      focusedarea = &msotj;
    }
    else {
      focusedarea = &msos;
    }
    focusedarea->enterFocusFrom(0);
    ui->setLegend();
  }
  return true;
}

bool MainScreen::keyPressed(unsigned int ch) {
  int scope = getCurrentScope();
  int action = keybinds.getKeyAction(ch, scope);
  if (temphighlightline) {
    temphighlightline = false;
    ui->redraw();
    if (action == KEYACTION_HIGHLIGHT_LINE) {
      return true;
    }
  }
  unsigned int pagerows = (unsigned int) (row - sitestartrow) * 0.6;
  if (gotomode) {
    bool matched = false;
    if (ch >= 32 && ch <= 126) {
      for (int i = 0; i < global->getSiteManager()->getNumSites(); i++) {
        std::shared_ptr<Site> site = global->getSiteManager()->getSite(i);
        if (toupper(ch) == toupper(site->getName()[0])) {
          sitepos = i;
          matched = true;
          break;
        }
      }
    }
    gotomode = false;
    if (matched && !msos.isFocused()) {
      defocusedarea = focusedarea;
      defocusedarea->defocus();
      focusedarea = &msos;
      focusedarea->enterFocusFrom(0);
    }
    ui->update();
    ui->setLegend();
    return true;
  }
  switch(action) {
    case KEYACTION_HIGHLIGHT_LINE:
      temphighlightline = true;
      ui->redraw();
      break;
    case KEYACTION_ENTER:
      if (msos.isFocused()) {
        if (!msos.linesSize()) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
        ui->goSiteStatus(site->getName());
      }
      else if (msosj.isFocused() && msosj.size() > 0) {
        unsigned int id = msosj.getElement(msosj.getSelectionPointer())->getId();
        ui->goRaceStatus(id);
      }
      else if (msotj.isFocused()) {
        unsigned int id = msotj.getElement(msotj.getSelectionPointer())->getId();
        ui->goTransferJobStatus(id);
      }
      else if (msop.isFocused()) {
        unsigned int id = msop.getElement(msop.getSelectionPointer())->getId();
        global->getEngine()->startPreparedRace(id);
        ui->redraw();
      }
      return true;
    case KEYACTION_GLOBAL_SETTINGS:
      ui->goGlobalOptions();
      return true;
    case KEYACTION_INFO:
      ui->goInfo();
      return true;
    case KEYACTION_EVENT_LOG:
      ui->goEventLog();
      return true;
    case KEYACTION_SCOREBOARD:
      ui->goScoreBoard();
      return true;
    case KEYACTION_METRICS:
      ui->goMetrics();
      return true;
    case KEYACTION_ALL_SPREAD_JOBS:
      ui->goAllRaces();
      return true;
    case KEYACTION_ADD_SITE:
      ui->goAddSite();
      return true;
    case KEYACTION_SECTIONS:
      ui->goSections();
      return true;
    case KEYACTION_SNAKE:
      ui->goSnake();
      return true;
    case KEYACTION_ALL_TRANSFER_JOBS:
      ui->goAllTransferJobs();
      return true;
    case KEYACTION_BROWSE_LOCAL:
      ui->goBrowseLocal();
      return true;
    case KEYACTION_DELETE:
      if (msos.isFocused()) {
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
        if (!site) break;
        deletesite = site->getName();
        ui->goConfirmation("Do you really want to delete site " + deletesite);
      }
      else if (msop.isFocused()) {
        unsigned int id = msop.getElement(msop.getSelectionPointer())->getId();
        global->getEngine()->deletePreparedRace(id);
        ui->redraw();
      }
      return true;
    case KEYACTION_ABORT:
      if (msosj.isFocused() && msosj.size() > 0) {
        abortrace = global->getEngine()->getRace(msosj.getElement(msosj.getSelectionPointer())->getId());
        if (!!abortrace && abortrace->getStatus() == RaceStatus::RUNNING) {
          ui->goConfirmation("Do you really want to abort the spread job " + abortrace->getName());
        }
      }
      else if (msotj.isFocused() && msotj.size() > 0) {
        abortjob = global->getEngine()->getTransferJob(msotj.getElement(msotj.getSelectionPointer())->getId());
        if (!!abortjob && !abortjob->isDone()) {
          ui->goConfirmation("Do you really want to abort the transfer job " + abortjob->getName());
        }
      }
      return true;
    case KEYACTION_ABORT_DELETE_INC:
      if (msosj.isFocused() && msosj.size() > 0) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(msosj.getElement(msosj.getSelectionPointer())->getId());
        if (!!race && race->getStatus() == RaceStatus::RUNNING) {
          abortdeleteraceinc = race;
          ui->goConfirmation("Do you really want to abort the race " + abortdeleteraceinc->getName() + " and delete your own files on all incomplete sites?");
        }
      }
      return true;
    case KEYACTION_ABORT_DELETE_ALL:
      if (msosj.isFocused() && msosj.size() > 0) {
        abortdeleteraceall = global->getEngine()->getRace(msosj.getElement(msosj.getSelectionPointer())->getId());
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
      if (msosj.isFocused() && msosj.size() > 0) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(msosj.getElement(msosj.getSelectionPointer())->getId());
        if (!!race) {
          global->getEngine()->resetRace(race, false);
          ui->redraw();
        }
      }
      else if (msotj.isFocused() && msotj.size() > 0) {
        std::shared_ptr<TransferJob> tj = global->getEngine()->getTransferJob(msotj.getElement(msotj.getSelectionPointer())->getId());
        if (!!tj) {
          global->getEngine()->resetTransferJob(tj);
          ui->redraw();
        }
      }
      return true;
    case KEYACTION_ALL_TRANSFERS:
      ui->goTransfers();
      return true;
    case KEYACTION_TRANSFERS:
      if (msosj.isFocused() && msosj.size() > 0) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(msosj.getElement(msosj.getSelectionPointer())->getId());
        if (!!race) {
          ui->goTransfersFilterSpreadJob(race->getName());
        }
      }
      else if (msotj.isFocused() && msotj.size() > 0) {
        std::shared_ptr<TransferJob> tj = global->getEngine()->getTransferJob(msotj.getElement(msotj.getSelectionPointer())->getId());
        if (!!tj) {
          ui->goTransfersFilterTransferJob(tj->getName());
        }
      }
      else if (msos.isFocused() && msos.size() > 0) {
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
        if (site) {
          ui->goTransfersFilterSite(sitename);
        }
      }
      return true;
    case KEYACTION_NEXT_PAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        keyDown();
      }
      ui->redraw();
      return true;
    case KEYACTION_PREVIOUS_PAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        keyUp();
      }
      ui->redraw();
      return true;
    case KEYACTION_QUICK_JUMP:
      gotomode = true;
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_BROWSE:
      if (scope == KEYSCOPE_TRANSFER_JOB) {
        std::shared_ptr<TransferJob> tj = global->getEngine()->getTransferJob(msotj.getElement(msotj.getSelectionPointer())->getId());
        Path srcpath = tj->getSrcPath();
        Path dstpath = tj->getDstPath();
        if (tj->isDirectory()) {
          srcpath = srcpath / tj->getSrcFileName();
          dstpath = dstpath / tj->getDstFileName();
        }
        if (tj->getType() == TRANSFERJOB_FXP) {
          ui->goBrowseSplit(tj->getSrc()->getSite()->getName(), tj->getDst()->getSite()->getName(), srcpath, dstpath);
        }
        else if (tj->getType() == TRANSFERJOB_DOWNLOAD) {
          ui->goBrowseSplit(tj->getSrc()->getSite()->getName(), srcpath, dstpath);
        }
        else if (tj->getType() == TRANSFERJOB_UPLOAD) {
          ui->goBrowseSplit(tj->getDst()->getSite()->getName(), dstpath, srcpath);
        }
        return true;
      }
      break;
    case KEYACTION_BACK_CANCEL:
      ui->goContinueBrowsing();
      return true;
  }
  if (msos.isFocused()) {
    switch(action) {
      case KEYACTION_EDIT_SITE: {
        if (!msos.linesSize()) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        ui->goEditSite(sitename);
        return true;
      }
      case KEYACTION_COPY: {
        if (!msos.linesSize()) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        std::shared_ptr<Site> oldsite = global->getSiteManager()->getSite(sitename);
        std::shared_ptr<Site> site = std::make_shared<Site>(*oldsite);
        int i;
        for (i = 0; !!global->getSiteManager()->getSite(site->getName() + "-" + std::to_string(i)); i++);
        site->setName(site->getName() + "-" + std::to_string(i));
        global->getSiteManager()->addSite(site);
        global->getSettingsLoaderSaver()->saveSettings();
        ui->redraw();
        ui->setInfo();
        return true;
      }
      case KEYACTION_BROWSE: {
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        ui->goBrowse(sitename);
        return true;
      }
      case KEYACTION_LOGIN: {
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        global->getSiteLogicManager()->getSiteLogic(sitename)->activateAll();
        return true;
      }
      case KEYACTION_FORCE_DISCONNECT_ALL_SLOTS: {
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        global->getSiteLogicManager()->getSiteLogic(sitename)->disconnectAll(true);
        return true;
      }
      case KEYACTION_DISCONNECT_ALL_SLOTS: {
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        global->getSiteLogicManager()->getSiteLogic(sitename)->disconnectAll();
        return true;
      }
      case KEYACTION_BROWSE_SPLIT: {
        if (!msos.linesSize()) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        ui->goBrowseSplit(sitename);
        return true;
      }
      case KEYACTION_RAW_COMMAND: {
        if (!msos.linesSize()) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
        ui->goRawCommand(sitename);
        return true;
      }
      case KEYACTION_REMOVE_SITE_FROM_ALL_SPREADJOBS: {
        if (!msos.linesSize()) break;
        removesite = msos.getElement(msos.getSelectionPointer())->getExtraData();
        awaitingremovesitefromallspreadjobs = true;
        ui->goConfirmation("Do you really want to remove " + removesite + " from ALL running spreadjobs?");
        return true;
      }
      case KEYACTION_0:
        jumpSectionHotkey(0);
        return true;
      case KEYACTION_1:
        jumpSectionHotkey(1);
        return true;
      case KEYACTION_2:
        jumpSectionHotkey(2);
        return true;
      case KEYACTION_3:
        jumpSectionHotkey(3);
        return true;
      case KEYACTION_4:
        jumpSectionHotkey(4);
        return true;
      case KEYACTION_5:
        jumpSectionHotkey(5);
        return true;
      case KEYACTION_6:
        jumpSectionHotkey(6);
        return true;
      case KEYACTION_7:
        jumpSectionHotkey(7);
        return true;
      case KEYACTION_8:
        jumpSectionHotkey(8);
        return true;
      case KEYACTION_9:
        jumpSectionHotkey(9);
        return true;
    }
  }
  return false;
}

std::string MainScreen::getLegendText() const {
  if (gotomode) {
    return "[Any] Go to matching first letter in site list - [Esc] Cancel";
  }
  return keybinds.getLegendSummary(getCurrentScope());
}

std::string MainScreen::getInfoLabel() const {
  return "CBFTP MAIN";
}

std::string MainScreen::getInfoText() const {
  std::string text;
  if (global->getRemoteCommandHandler()->isEnabled()) {
    text += "Remote commands enabled  ";
  }
  if (global->getEngine()->getNextPreparedRaceStarterEnabled()) {
    text += "Next spread job starter: ";
    if (global->getEngine()->getNextPreparedRaceStarterTimeout() != 0) {
      text += util::simpleTimeFormat(global->getEngine()->getNextPreparedRaceStarterTimeRemaining());
    }
    else {
      text += "Active";
    }
    text += "  ";
  }
  return text + activeracestext + activejobstext + numsitestext;
}

void MainScreen::addPreparedRaceTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id,
    bool selectable, const std::string & section, const std::string & name, const std::string & ttl,
    const std::string & sites)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "section", section);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButton(y, 1, "name", name);
  if (!selectable) {
    msotb->setSelectable(false);
  }
  msotb->setId(id);
  msal->addElement(msotb, 4, 1, RESIZE_CUTEND, true);

  msotb = mso.addTextButtonNoContent(y, 1, "ttl", ttl);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sites", sites);
  msotb->setSelectable(false);
  msal->addElement(msotb, 0, RESIZE_WITHDOTS);
}

void MainScreen::addPreparedRaceTableHeader(unsigned int y, MenuSelectOption & mso) {
  addPreparedRaceTableRow(y, mso, -1, false, "SECTION", "PREPARED SPREAD JOB NAME", "EXPIRES", "SITES");
}

void MainScreen::addPreparedRaceDetails(unsigned int y, MenuSelectOption & mso, const std::shared_ptr<PreparedRace> & preparedrace) {
  unsigned int id = preparedrace->getId();
  std::string section = preparedrace->getSection();
  std::string name = preparedrace->getName();
  std::string ttl = util::simpleTimeFormat(preparedrace->getRemainingTime());
  std::list<std::pair<std::string, bool>> sites = preparedrace->getSites();
  std::string sitestr;
  for (const std::pair<std::string, bool>& site : sites) {
    sitestr += site.first + ",";
  }
  if (sitestr.length() > 0) {
    sitestr = sitestr.substr(0, sitestr.length() - 1);
  }
  addPreparedRaceTableRow(y, mso, id, true, section, name, ttl, sitestr);
}

void MainScreen::addSiteHeader(unsigned int y, MenuSelectOption & mso) {
  addSiteRow(y, mso, false, "SITE           ", "LOGINS", "UPLOADS", "DOWNLOADS", "UP", "DOWN", "DISABLED", "UP 24HR", "DOWN 24HR", "ALLUP", "ALLDOWN", "PRIORITY");
}

void MainScreen::addSiteRow(unsigned int y, MenuSelectOption & mso, bool selectable, const std::string & site,
    const std::string & logins, const std::string & uploads, const std::string & downloads,
    const std::string & allowup, const std::string & allowdown, const std::string & disabled,
    const std::string & dayup, const std::string & daydn, const std::string & alup, const std::string & aldn,
    const std::string & prio)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "site", site);
  if (!selectable) {
    msotb->setSelectable(false);
  }
  msotb->setExtraData(site);
  msal->addElement(msotb, 13, 6, RESIZE_CUTEND, false);

  msotb = mso.addTextButtonNoContent(y, 1, "logins", logins);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 12, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "uploads", uploads);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 10, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "downloads", downloads);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 11, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "up", allowup);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "down", allowdown);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "disabled", disabled);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "dayup", dayup);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "daydn", daydn);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "alup", alup);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "aldn", aldn);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "prio", prio);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 1, RESIZE_REMOVE);
}

void MainScreen::addSiteDetails(unsigned int y, MenuSelectOption & mso, const std::shared_ptr<SiteLogic> & sl) {
  std::shared_ptr<Site> site = sl->getSite();
  std::string sitename = site->getName();
  std::string logins = std::to_string(sl->getCurrLogins());
  if (!site->unlimitedLogins()) {
    logins += "/" + std::to_string(site->getMaxLogins());
  }
  std::string uploads = std::to_string(sl->getCurrUp());
  if (!site->unlimitedUp()) {
    uploads += "/" + std::to_string(site->getMaxUp());
  }
  std::string downloads = std::to_string(sl->getCurrDown());
  if (!site->unlimitedDown()) {
    downloads += "/" + std::to_string(site->getMaxDown());
  }
  std::string up;
  switch (site->getAllowUpload()) {
    case SITE_ALLOW_TRANSFER_NO:
      up = "[ ]";
      break;
    case SITE_ALLOW_TRANSFER_YES:
      up = "[X]";
      break;
    default:
      assert(false);
      break;
  }
  std::string down;
  switch (site->getAllowDownload()) {
    case SITE_ALLOW_TRANSFER_NO:
      down = "[ ]";
      break;
    case SITE_ALLOW_TRANSFER_YES:
      down = "[X]";
      break;
    case SITE_ALLOW_DOWNLOAD_MATCH_ONLY:
      down = "[A]";
      break;
  }
  std::string disabled = site->getDisabled()? "[X]" : "[ ]";
  std::string up24 = util::parseSize(site->getSizeUp().getLast24Hours());
  std::string down24 = util::parseSize(site->getSizeDown().getLast24Hours());
  std::string allup = util::parseSize(site->getSizeUp().getAll());
  std::string alldown = util::parseSize(site->getSizeDown().getAll());
  std::string prio = Site::getPriorityText(site->getPriority());
  addSiteRow(y, mso, true, sitename, logins, uploads, downloads, up, down, disabled, up24, down24, allup, alldown, prio);
}

void MainScreen::jumpSectionHotkey(int hotkey) {
  if (!msos.linesSize()) {
    return;
  }
  Section * section = global->getSectionManager()->getSection(hotkey);
  if (section == nullptr) {
    return;
  }
  std::string sitename = msos.getElement(msos.getSelectionPointer())->getExtraData();
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
  if (site->hasSection(section->getName())) {
    ui->goBrowseSection(sitename, section->getName());
  }
}

int MainScreen::getCurrentScope() const {
  int scope = KEYSCOPE_ALL;
  if (msos.isFocused() && msos.size() > 0) {
    scope = KEYSCOPE_SITE;
  }
  else if (msosj.isFocused()) {
    scope = KEYSCOPE_SPREAD_JOB;
  }
  else if (msotj.isFocused()) {
    scope = KEYSCOPE_TRANSFER_JOB;
  }
  else if (msop.isFocused()) {
    scope = KEYSCOPE_PREPARED_SPREAD_JOB;
  }
  return scope;
}
