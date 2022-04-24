#include "transferstatusscreen.h"

#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../misc.h"

#include "../../transferstatus.h"
#include "../../globalcontext.h"
#include "../../sitelogicmanager.h"
#include "../../sitelogic.h"

TransferStatusScreen::TransferStatusScreen(Ui* ui) : UIWindow(ui, "TransferStatusScreen") {
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind('B', KEYACTION_ABORT, "Abort transfer");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
}

TransferStatusScreen::~TransferStatusScreen() {

}

void TransferStatusScreen::initialize(unsigned int row, unsigned int col, std::shared_ptr<TransferStatus> ts) {
  this->ts = ts;
  autoupdate = true;
  init(row, col);
}

void TransferStatusScreen::redraw() {
  vv->clear();
  unsigned int y = 1;
  int type = ts->getType();
  vv->putStr(y, 1, "Started: " + ts->getTimestamp());
  std::string route = ts->getSource() + " -> " + ts->getTarget();
  vv->putStr(y, 20, "Route: " + route);
  ++y;
  vv->putStr(y, 1, "Job name: " + ts->getJobName());
  ++y;
  vv->putStr(y, 1, "File name: " + ts->getFile());
  ++y;
  unsigned int x = 1;
  if (type != TRANSFERSTATUS_TYPE_UPLOAD) {
    vv->putStr(y, x, "Source slot: " + std::to_string(ts->getSourceSlot()));
    x += 18;
  }
  if (type != TRANSFERSTATUS_TYPE_DOWNLOAD) {
    vv->putStr(y, x, "Target slot: " + std::to_string(ts->getTargetSlot()));
    x += 18;
  }
  bool ssl = ts->getSSL();
  vv->putStr(y, x, std::string("TLS: ") + (ssl ? "Yes" : "No"));
  if (type != TRANSFERSTATUS_TYPE_FXP && ssl) {
    ++y;
    vv->putStr(y, 1, std::string("TLS session reused: ") + (ts->getSSLSessionReused() ? "Yes" : "No"));
    vv->putStr(y, 30, "Cipher: " + ts->getCipher());

  }
  ++y;
  vv->putStr(y, 1, "Source path: " + ts->getSourcePath().toString());
  ++y;
  vv->putStr(y, 1, "Target path: " + ts->getTargetPath().toString());
  ++y;
  TransferDetails td = TransfersScreen::formatTransferDetails(ts);
  std::string passive;
  bool defaultactive = ts->getDefaultActive();
  if (type == TRANSFERSTATUS_TYPE_FXP) {
    passive = defaultactive ? ts->getSource() : ts->getTarget();
  }
  else {
    if (defaultactive) {
     passive = (type == TRANSFERSTATUS_TYPE_DOWNLOAD ? ts->getSource() : ts->getTarget());
    }
    else {
     passive = "local";
    }
  }
  vv->putStr(y, 1, "Passive: " + passive + "     Passive address: " + ts->getPassiveAddress());
  ++y;
  vv->putStr(y, 1, "Size: " + td.transferred);
  vv->putStr(y, 35, "Speed: " + td.speed);
  std::string progress;
  switch (ts->getState()) {
    case TRANSFERSTATUS_STATE_IN_PROGRESS:
      progress = "In progress";
      break;
    case TRANSFERSTATUS_STATE_FAILED:
      progress = "Failed";
      break;
    case TRANSFERSTATUS_STATE_SUCCESSFUL:
      progress = "Completed";
      break;
    case TRANSFERSTATUS_STATE_DUPE:
      progress = "Failed (Dupe)";
      break;
    case TRANSFERSTATUS_STATE_ABORTED:
      progress = "Aborted";
      break;
    case TRANSFERSTATUS_STATE_TIMEOUT:
      progress = "Timeout";
      break;
  }
  vv->putStr(y, 57, "Status: " + progress);
  ++y;
  vv->putStr(y, 1, "Time spent: " + td.timespent);
  vv->putStr(y, 21, "Remaining: " + td.timeremaining);

  int progresspercent = ts->getProgress();
  progress = "....................";
  int charswithhighlight = progress.length() * progresspercent / 100;
  vv->putStr(y, 47, "[");
  vv->putStr(y, 48, progress.substr(0, charswithhighlight), true);
  vv->putStr(y, 48 + charswithhighlight, progress.substr(charswithhighlight));
  vv->putStr(y, 48 + progress.length(), "] " + std::to_string(progresspercent) + "%");
  ++y;
  ++y;
  for (std::list<std::string>::const_iterator it = ts->getLogLines().begin(); it != ts->getLogLines().end(); it++) {
    vv->putStr(y++, 1, *it);
  }
}

bool TransferStatusScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  switch (action) {
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_ABORT:
      abortTransfer(ts);
      return true;
  }
  return false;
}

void TransferStatusScreen::abortTransfer(std::shared_ptr<TransferStatus> ts) {
  if (ts->getState() == TRANSFERSTATUS_STATE_IN_PROGRESS) {
    int type = ts->getType();
    if (type == TRANSFERSTATUS_TYPE_DOWNLOAD || type == TRANSFERSTATUS_TYPE_FXP) {
      global->getSiteLogicManager()->getSiteLogic(ts->getSource())->disconnectConn(ts->getSourceSlot());
    }
    if (type == TRANSFERSTATUS_TYPE_UPLOAD || type == TRANSFERSTATUS_TYPE_FXP) {
      global->getSiteLogicManager()->getSiteLogic(ts->getTarget())->disconnectConn(ts->getTargetSlot());
    }
    ts->setAborted();
  }
}

std::string TransferStatusScreen::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::string TransferStatusScreen::getInfoLabel() const {
  return "TRANSFER STATUS: " + ts->getFile() + " " + ts->getSource() + " -> " + ts->getTarget();
}
