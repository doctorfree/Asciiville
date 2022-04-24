#include "infoscreen.h"

#include "../../globalcontext.h"
#include "../../statistics.h"
#include "../../util.h"
#include "../../buildinfo.h"
#include "../../hourlyalltracking.h"
#include "../../settingsloadersaver.h"
#include "../../loadmonitor.h"
#include "../../sitemanager.h"

#include "../../core/polling.h"
#include "../../core/sslmanager.h"

#include "../ui.h"

namespace {

enum KeyAction {
  KEYACTION_RESET_HOURLY,
  KEYACTION_RESET_ALL,
  KEYACTION_RESET_SITES_HOURLY,
  KEYACTION_RESET_SITES_ALL
};

}

InfoScreen::InfoScreen(Ui* ui) : UIWindow(ui, "InfoScreen") {
  keybinds.addBind('r', KEYACTION_RESET_HOURLY, "Reset global 24-hour stats");
  keybinds.addBind('R', KEYACTION_RESET_ALL, "Reset global all time stats");
  keybinds.addBind('l', KEYACTION_RESET_SITES_HOURLY, "Reset 24-hour stats for all sites");
  keybinds.addBind('L', KEYACTION_RESET_SITES_ALL, "Reset all time stats for all sites");
}

void InfoScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  init(row, col);
}

void InfoScreen::redraw() {
  vv->clear();
  ui->hideCursor();
  unsigned long long int sizefxpday = global->getStatistics()->getSizeFXP().getLast24Hours();
  unsigned int filesfxpday = global->getStatistics()->getFilesFXP().getLast24Hours();
  unsigned long long int sizefxpall = global->getStatistics()->getSizeFXP().getAll();
  unsigned int filesfxpall = global->getStatistics()->getFilesFXP().getAll();
  unsigned long long int sizeupday = global->getStatistics()->getSizeUp().getLast24Hours();
  unsigned int filesupday = global->getStatistics()->getFilesUp().getLast24Hours();
  unsigned long long int sizeupall = global->getStatistics()->getSizeUp().getAll();
  unsigned int filesupall = global->getStatistics()->getFilesUp().getAll();
  unsigned long long int sizedownday = global->getStatistics()->getSizeDown().getLast24Hours();
  unsigned int filesdownday = global->getStatistics()->getFilesDown().getLast24Hours();
  unsigned long long int sizedownall = global->getStatistics()->getSizeDown().getAll();
  unsigned int filesdownall = global->getStatistics()->getFilesDown().getAll();

  unsigned int i = 1;
  vv->putStr(i++, 1, "Compile time: " + BuildInfo::compileTime());
  vv->putStr(i++, 1, "Version tag: " + BuildInfo::version());
  vv->putStr(i++, 1, "Distribution tag: " + BuildInfo::tag());
  vv->putStr(i++, 1, "OpenSSL version: " + Core::SSLManager::version());
  vv->putStr(i++, 1, "Polling syscall: " + Core::Polling::type());
  vv->putStr(i++, 1, std::string("Data file encryption: ") + (global->getSettingsLoaderSaver()->getState() == DataFileState::EXISTS_DECRYPTED ? "Enabled" : "Disabled"));
  i++;
  vv->putStr(i++, 1, "Traffic measurements");
  vv->putStr(i++, 1, "FXP      last 24 hours: " + util::parseSize(sizefxpday) + ", " +
               std::to_string(filesfxpday) + " files - All time: " + util::parseSize(sizefxpall) + ", " +
               std::to_string(filesfxpall) + " files");
  vv->putStr(i++, 1, "Upload   last 24 hours: " + util::parseSize(sizeupday) + ", " +
                 std::to_string(filesupday) + " files - All time: " + util::parseSize(sizeupall) + ", " +
                 std::to_string(filesupall) + " files");
  vv->putStr(i++, 1, "Download last 24 hours: " + util::parseSize(sizedownday) + ", " +
                 std::to_string(filesdownday) + " files - All time: " + util::parseSize(sizedownall) + ", " +
                 std::to_string(filesdownall) + " files");
  ++i;
  vv->putStr(i++, 1, "All time spread jobs: " + std::to_string(global->getStatistics()->getSpreadJobs()));
  vv->putStr(i++, 1, "All time transfer jobs: " + std::to_string(global->getStatistics()->getTransferJobs()));
  ++i;
  vv->putStr(i++, 1, "Current CPU load total: " + std::to_string(global->getLoadMonitor()->getCurrentCpuUsageAll()) + "%");
  vv->putStr(i++, 1, "Current CPU load worker: " + std::to_string(global->getLoadMonitor()->getCurrentCpuUsageWorker()) + "%");
  vv->putStr(i++, 1, "Current worker queue size: " + std::to_string(global->getLoadMonitor()->getCurrentWorkerQueueSize()));
  vv->putStr(i++, 1, "Current performance level: " + std::to_string(global->getLoadMonitor()->getCurrentRecommendedPerformanceLevel()));
  vv->putStr(i++, 1, "Current file list refresh rate: " + std::to_string(global->getLoadMonitor()->getCurrentFileListRefreshRate()) + "/s");
}

void InfoScreen::command(const std::string& command, const std::string& arg) {
  if (command == "yes") {
    switch (confirmaction) {
      case KEYACTION_RESET_HOURLY:
        global->getStatistics()->resetHourlyStats();
        break;
      case KEYACTION_RESET_ALL:
        global->getStatistics()->resetAllStats();
        break;
      case KEYACTION_RESET_SITES_HOURLY:
        global->getSiteManager()->resetHourlyStats();
        break;
      case KEYACTION_RESET_SITES_ALL:
        global->getSiteManager()->resetAllStats();
        break;
    }
  }
  ui->redraw();
}

bool InfoScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  confirmaction = action;
  switch (action) {
    case KEYACTION_RESET_HOURLY:
      ui->goConfirmation("Do you really wish to reset the global 24-hour stats");
      return true;
    case KEYACTION_RESET_ALL:
      ui->goStrongConfirmation("Do you really wish to reset the global all time stats?");
      return true;
    case KEYACTION_RESET_SITES_HOURLY:
      ui->goConfirmation("Do you really wish to reset the 24-hour stats for all sites");
      return true;
    case KEYACTION_RESET_SITES_ALL:
      ui->goStrongConfirmation("Do you really wish to reset the all time stats for all sites?");
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string InfoScreen::getInfoLabel() const {
  return "GENERAL INFO";
}
