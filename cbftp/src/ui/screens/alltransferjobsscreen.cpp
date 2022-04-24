#include "alltransferjobsscreen.h"

#include "transferjobstatusscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../misc.h"

#include "../../globalcontext.h"
#include "../../util.h"
#include "../../engine.h"
#include "../../transferjob.h"
#include "../../site.h"
#include "../../sitelogic.h"

AllTransferJobsScreen::AllTransferJobsScreen(Ui* ui) : UIWindow(ui, "AllTransferJobsScreen"), table(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Details");
  keybinds.addBind('b', KEYACTION_BROWSE, "Browse");
  keybinds.addBind('B', KEYACTION_ABORT, "Abort job");
  keybinds.addBind('t', KEYACTION_TRANSFERS, "Transfers for job");
  keybinds.addBind('r', KEYACTION_RESET, "Reset");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
  keybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
}

void AllTransferJobsScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  hascontents = false;
  currentviewspan = 0;
  ypos = 1;
  temphighlightline = false;
  engine = global->getEngine();
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void AllTransferJobsScreen::redraw() {
  vv->clear();
  unsigned int y = 0;
  unsigned int totallistsize = engine->allTransferJobs() + 1;
  table.reset();
  while (ypos > 1 && ypos > engine->allTransferJobs()) {
    --ypos;
  }
  adaptViewSpan(currentviewspan, row, ypos, totallistsize);

  if (!currentviewspan) {
    addJobTableHeader(y, table, "NAME");
    y++;
  }
  unsigned int pos = 1;
  for (std::list<std::shared_ptr<TransferJob> >::const_iterator it = --engine->getTransferJobsEnd(); it != --engine->getTransferJobsBegin() && y < row; it--) {
    if (pos >= currentviewspan) {
      addJobDetails(y++, table, *it);
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
  printSlider(vv, row, col - 1, totallistsize, currentviewspan);
}

void AllTransferJobsScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    global->getEngine()->abortTransferJob(abortjob);
  }
  ui->redraw();
}

bool AllTransferJobsScreen::keyPressed(unsigned int ch) {
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
      if (hascontents && ypos > 1) {
        --ypos;
        table.goUp();
        ui->update();
      }
      else if (ypos == 1) {
        currentviewspan = 0;
        ui->update();
      }
      return true;
    case KEYACTION_DOWN:
      if (hascontents && ypos < engine->allTransferJobs()) {
        ++ypos;
        table.goDown();
        ui->update();
      }
      return true;
    case KEYACTION_NEXT_PAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows; i++) {
        if (ypos >= engine->allTransferJobs()) {
          break;
        }
        ypos++;
        table.goDown();
      }
      ui->update();
      return true;
    }
    case KEYACTION_PREVIOUS_PAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows; i++) {
        if (ypos == 1) {
          currentviewspan = 0;
          break;
        }
        ypos--;
        table.goUp();
      }
      ui->update();
      return true;
    }
    case KEYACTION_TOP:
      ypos = 1;
      currentviewspan = 0;
      ui->update();
      return true;
    case KEYACTION_BOTTOM:
      ypos = engine->allTransferJobs();
      ui->update();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_ENTER:
      if (hascontents) {
        std::shared_ptr<MenuSelectOptionTextButton> msotb =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        ui->goTransferJobStatus(msotb->getId());
      }
      return true;
    case KEYACTION_BROWSE:
      if (hascontents) {
        std::shared_ptr<TransferJob> tj = global->getEngine()->getTransferJob(table.getElement(table.getSelectionPointer())->getId());
        if (!!tj) {
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
        }
      }
      return true;
    case KEYACTION_ABORT:
      if (hascontents) {
        abortjob = global->getEngine()->getTransferJob(table.getElement(table.getSelectionPointer())->getId());
        if (!!abortjob && !abortjob->isDone()) {
          ui->goConfirmation("Do you really want to abort the transfer job " + abortjob->getName());
        }
      }
      return true;
    case KEYACTION_TRANSFERS:
      if (hascontents) {
        std::shared_ptr<TransferJob> tj = global->getEngine()->getTransferJob(table.getElement(table.getSelectionPointer())->getId());
        if (!!tj) {
          ui->goTransfersFilterTransferJob(tj->getName());
        }
      }
      return true;
    case KEYACTION_RESET:
      if (hascontents) {
        std::shared_ptr<TransferJob> tj = global->getEngine()->getTransferJob(table.getElement(table.getSelectionPointer())->getId());
        if (!!tj) {
          global->getEngine()->resetTransferJob(tj);
          ui->redraw();
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

std::string AllTransferJobsScreen::getInfoLabel() const {
  return "ALL TRANSFER JOBS";
}

std::string AllTransferJobsScreen::getInfoText() const {
  return "Active: " + std::to_string(engine->currentTransferJobs()) + "  Total: " + std::to_string(engine->allTransferJobs());
}

void AllTransferJobsScreen::addJobTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id, bool selectable,
    const std::string & queuetime, const std::string & starttime, const std::string & timespent, const std::string & type,
    const std::string & name, const std::string & route, const std::string & sizeprogress, const std::string & filesprogress,
    const std::string & timeremaining, const std::string & speed, const std::string & progress) {
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "queuetime", queuetime);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "starttime", starttime);
  msotb->setSelectable(false);
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msotb->setSelectable(false);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "type", type);
  msotb->setSelectable(false);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "name", name);
  msotb->setSelectable(selectable);
  msotb->setId(id);
  msal->addElement(msotb, 11, 0, RESIZE_CUTEND, true);

  msotb = mso.addTextButtonNoContent(y, 1, "route", route);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sizeprogress", sizeprogress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "filesprogress", filesprogress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 1, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timeremaining", timeremaining);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "speed", speed);
  msotb->setSelectable(false);
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "progress", progress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 10, RESIZE_REMOVE);
}

void AllTransferJobsScreen::addJobTableHeader(unsigned int y, MenuSelectOption & mso, const std::string & name) {
  addJobTableRow(y, mso, -1, false, "QUEUED", "STARTED", "USE", "TYPE", name, "ROUTE", "SIZE", "FILES", "LEFT", "SPEED", "DONE");
}

void AllTransferJobsScreen::addJobDetails(unsigned int y, MenuSelectOption & mso, std::shared_ptr<TransferJob> tj) {
  std::string timespent = util::simpleTimeFormat(tj->timeSpent());
  bool running = tj->getStatus() == TRANSFERJOB_RUNNING;
  bool started = tj->getStatus() != TRANSFERJOB_QUEUED;
  std::string timeremaining = running ? util::simpleTimeFormat(tj->timeRemaining()) : "-";
  std::string route = TransferJobStatusScreen::getRoute(tj);
  std::string sizeprogress = util::parseSize(tj->sizeProgress()) +
                             " / " + util::parseSize(tj->totalSize());
  std::string filesprogress = std::to_string(tj->filesProgress()) + "/" +
                              std::to_string(tj->filesTotal());
  std::string speed = started ? util::parseSize(tj->getSpeed() * SIZEPOWER) + "/s" : "-";
  std::string progress = std::to_string(tj->getProgress()) + "%";
  std::string status;
  switch (tj->getStatus()) {
    case TRANSFERJOB_QUEUED:
      status = "queue";
      break;
    case TRANSFERJOB_RUNNING:
      status = progress;
      break;
    case TRANSFERJOB_DONE:
      status = "done";
      break;
    case TRANSFERJOB_ABORTED:
      status = "abor";
      break;
  }
  std::string type = "FXP";
  switch (tj->getType()) {
    case TRANSFERJOB_DOWNLOAD:
      type = "DL";
      break;
    case TRANSFERJOB_UPLOAD:
      type = "UL";
      break;
  }
  addJobTableRow(y, mso, tj->getId(), true, tj->timeQueued(), tj->timeStarted(), timespent, type, tj->getSrcFileName(), route, sizeprogress, filesprogress, timeremaining, speed, status);
}
