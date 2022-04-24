#include "sitestatusscreen.h"

#include "../../sitemanager.h"
#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../ftpconn.h"
#include "../../connstatetracker.h"
#include "../../util.h"
#include "../../hourlyalltracking.h"

#include "../ui.h"

namespace {

enum KeyAction {
  KEYACTION_RAW_COMMAND,
  KEYACTION_EDIT_SITE,
  KEYACTION_FORCE_DISCONNECT_ALL_SLOTS,
  KEYACTION_DISCONNECT_ALL_SLOTS,
  KEYACTION_LOGIN_ALL_SLOTS,
  KEYACTION_RESET_HOURLY,
  KEYACTION_RESET_ALL
};

}

SiteStatusScreen::SiteStatusScreen(Ui* ui) : UIWindow(ui, "SiteStatusScreen") {
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Raw data screens");
  keybinds.addBind(10, KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind('b', KEYACTION_BROWSE, "Browse");
  keybinds.addBind('w', KEYACTION_RAW_COMMAND, "Raw command");
  keybinds.addBind('E', KEYACTION_EDIT_SITE, "Edit site");
  keybinds.addBind('t', KEYACTION_TRANSFERS, "Transfers");
  keybinds.addBind('K', KEYACTION_FORCE_DISCONNECT_ALL_SLOTS, "Kill all slots");
  keybinds.addBind('k', KEYACTION_DISCONNECT_ALL_SLOTS, "Disconnect all slots");
  keybinds.addBind('L', KEYACTION_LOGIN_ALL_SLOTS, "Login all slots");
  keybinds.addBind('r', KEYACTION_RESET_HOURLY, "Reset 24-hour stats");
  keybinds.addBind('R', KEYACTION_RESET_ALL, "Reset all time stats");
}

void SiteStatusScreen::initialize(unsigned int row, unsigned int col, std::string sitename) {
  this->sitename = sitename;
  site = global->getSiteManager()->getSite(sitename);
  autoupdate = true;
  st = global->getSiteLogicManager()->getSiteLogic(site->getName());
  init(row, col);
}

void SiteStatusScreen::redraw() {
  vv->clear();
  ui->hideCursor();
  std::string loginslots = "Login slots:    " + std::to_string(st->getCurrLogins());
  if (!site->unlimitedLogins()) {
    loginslots += "/" + std::to_string(site->getMaxLogins());
  }
  std::string upslots = "Upload slots:   " + std::to_string(st->getCurrUp());
  if (!site->unlimitedUp()) {
    upslots += "/" + std::to_string(site->getMaxUp());
  }
  std::string downslots = "Download slots: " + std::to_string(st->getCurrDown());
  if (!site->unlimitedDown()) {
    downslots += "/" + std::to_string(site->getMaxDown());
  }
  vv->putStr(1, 1, loginslots);
  vv->putStr(2, 1, upslots);
  vv->putStr(3, 1, downslots);
  vv->putStr(5, 1, "Login threads:");
  int i = 8;
  for(unsigned int j = 0; j < st->getConns()->size(); j++) {
    std::string status = st->getStatus(j);
    std::string llstate = st->getConnStateTracker(j)->isListLocked()
        ? "Y" : "N";
    std::string hlstate = st->getConnStateTracker(j)->isHardLocked()
        ? "Y" : "N";
    std::string tstate = st->getConnStateTracker(j)->hasFileTransfer()
        ? "Y" : "N";
    vv->putStr(i++, 1, "#" + std::to_string((int)j) + " - LL:" + llstate +
        " - HL:" + hlstate + " - T:" + tstate + " - " + status);
  }
  ++i;
  unsigned long long int sizeupday = site->getSizeUp().getLast24Hours();
  unsigned int filesupday = site->getFilesUp().getLast24Hours();
  unsigned long long int sizeupall = site->getSizeUp().getAll();
  unsigned int filesupall = site->getFilesUp().getAll();
  unsigned long long int sizedownday = site->getSizeDown().getLast24Hours();
  unsigned int filesdownday = site->getFilesDown().getLast24Hours();
  unsigned long long int sizedownall = site->getSizeDown().getAll();
  unsigned int filesdownall = site->getFilesDown().getAll();
  vv->putStr(i++, 1, "Traffic measurements");
  vv->putStr(i++, 1, "Upload   last 24 hours: " + util::parseSize(sizeupday) + ", " +
                 std::to_string(filesupday) + " files - All time: " + util::parseSize(sizeupall) + ", " +
                 std::to_string(filesupall) + " files");
  vv->putStr(i++, 1, "Download last 24 hours: " + util::parseSize(sizedownday) + ", " +
                 std::to_string(filesdownday) + " files - All time: " + util::parseSize(sizedownall) + ", " +
                 std::to_string(filesdownall) + " files");
}

bool SiteStatusScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  confirmaction = action;
  switch(action) {
    case KEYACTION_RIGHT:
      ui->goRawData(site->getName());
      return true;
    case KEYACTION_EDIT_SITE:
      ui->goEditSite(site->getName());
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_BROWSE:
      ui->goBrowse(site->getName());
      return true;
    case KEYACTION_TRANSFERS:
      ui->goTransfersFilterSite(site->getName());
      return true;
    case KEYACTION_RAW_COMMAND:
      ui->goRawCommand(site->getName());
      return true;
    case KEYACTION_FORCE_DISCONNECT_ALL_SLOTS:
      st->disconnectAll(true);
      return true;
    case KEYACTION_DISCONNECT_ALL_SLOTS:
      st->disconnectAll();
      return true;
    case KEYACTION_LOGIN_ALL_SLOTS:
      for(unsigned int j = 0; j < st->getConns()->size(); j++) {
        st->connectConn(j);
      }
      return true;
    case KEYACTION_RESET_HOURLY:
      ui->goConfirmation("Do you really wish to reset the 24-hour stats for " + site->getName());
      return true;
    case KEYACTION_RESET_ALL:
      ui->goStrongConfirmation("Do you really wish to reset the all time stats for " + site->getName() + "?");
      return true;
  }
  return false;
}

std::string SiteStatusScreen::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::string SiteStatusScreen::getInfoLabel() const {
  return "DETAILED STATUS: " + site->getName();
}

void SiteStatusScreen::command(const std::string& command, const std::string& arg) {
  if (command == "yes") {
    switch (confirmaction) {
      case KEYACTION_RESET_HOURLY:
        st->getSite()->resetHourlyStats();
        break;
      case KEYACTION_RESET_ALL:
        st->getSite()->resetAllStats();
        break;
    }
  }
  ui->redraw();
}
