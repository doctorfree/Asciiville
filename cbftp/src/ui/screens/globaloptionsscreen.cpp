#include "globaloptionsscreen.h"

#include "../../core/iomanager.h"
#include "../../globalcontext.h"
#include "../../remotecommandhandler.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../localstorage.h"
#include "../../settingsloadersaver.h"
#include "../../engine.h"
#include "../../transferprotocol.h"
#include "../../transfermanager.h"
#include "../../httpserver.h"
#include "../../logmanager.h"

#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextarrow.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../misc.h"

namespace {

enum class UdpEnable {
  NO,
  PLAINTEXT,
  ENCRYPTED
};

void addHistoryOptions(std::shared_ptr<MenuSelectOptionTextArrow>& arrow, bool includelow = true) {
  if (includelow) {
    arrow->addOption("10", 10);
    arrow->addOption("25", 25);
    arrow->addOption("50", 50);
  }
  arrow->addOption("100", 100);
  arrow->addOption("250", 250);
  arrow->addOption("500", 500);
  arrow->addOption("1000", 1000);
  arrow->addOption("3000", 3000);
  arrow->addOption("10000", 10000);
  if (!includelow) {
    arrow->addOption("50000", 50000);
    arrow->addOption("100000", 100000);
  }
  arrow->addOption("Unlimited", -1);
}

void addRuntimeOptions(std::shared_ptr<MenuSelectOptionTextArrow>& arrow) {
  arrow->addOption("10s", 10);
  arrow->addOption("30s", 30);
  arrow->addOption("1m", 60);
  arrow->addOption("3m", 180);
  arrow->addOption("5m", 300);
  arrow->addOption("10m", 600);
  arrow->addOption("30m", 1800);
  arrow->addOption("1h", 3600);
  arrow->addOption("2h", 7200);
  arrow->addOption("8h", 28800);
  arrow->addOption("24h", 86400);
  arrow->addOption("Unlimited", 0);
}

}

GlobalOptionsScreen::GlobalOptionsScreen(Ui* ui) : UIWindow(ui, "GlobalOptionsScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind('S', KEYACTION_SKIPLIST, "Skiplist");
}

GlobalOptionsScreen::~GlobalOptionsScreen() {

}

void GlobalOptionsScreen::initialize(unsigned int row, unsigned int col) {
  active = false;
  unsigned int y = 1;
  unsigned int x = 1;
  rch = global->getRemoteCommandHandler();
  sm = global->getSiteManager();
  ls = global->getLocalStorage();
  mso.reset();
  bindinterface = mso.addTextArrow(y++, x, "bindinterface", "Bind to network interface:");
  bindinterface->addOption("Unspecified", 0);
  interfacemap[0] = "";
  std::list<std::pair<std::string, std::string> > interfaces = global->getIOManager()->listInterfaces();
  int interfaceid = 1;
  bool hasbind = global->getIOManager()->hasBindInterface();
  std::string bindinterfacename;
  if (hasbind) {
    bindinterfacename = global->getIOManager()->getBindInterface();
  }
  for (std::list<std::pair<std::string, std::string> >::iterator it = interfaces.begin(); it != interfaces.end(); it++) {
    if (it->first == "lo") {
      continue;
    }
    interfacemap[interfaceid] = it->first;
    bindinterface->addOption(it->first + ", " + it->second, interfaceid++);
    if (hasbind && it->first == bindinterfacename) {
      bindinterface->setOption(interfaceid - 1);
    }
  }
  std::shared_ptr<MenuSelectOptionTextArrow> transferproto = mso.addTextArrow(y++, x, "transferprotocol", "Local transfer protocol:");
  transferproto->addOption("IPv4 only", static_cast<int>(TransferProtocol::IPV4_ONLY));
  transferproto->addOption("Prefer IPv4", static_cast<int>(TransferProtocol::PREFER_IPV4));
  transferproto->addOption("Prefer IPv6", static_cast<int>(TransferProtocol::PREFER_IPV6));
  transferproto->addOption("IPv6 only", static_cast<int>(TransferProtocol::IPV6_ONLY));
  transferproto->setOption(static_cast<int>(ls->getTransferProtocol()));
  int firstport = ls->getActivePortFirst();
  int lastport = ls->getActivePortLast();
  std::string portrange = std::to_string(firstport) + ":" + std::to_string(lastport);
  mso.addStringField(y++, x, "activeportrange", "Active mode port range:", portrange, false, 11);
  mso.addCheckBox(y++, x, "useactiveaddress", "Use active mode address:", ls->getUseActiveModeAddress());
  mso.addStringField(y++, x, "activeaddress4", "Active mode address IPv4:", ls->getActiveModeAddress4(), false, 52, 64);
  mso.addStringField(y++, x, "activeaddress6", "Active mode address IPv6:", ls->getActiveModeAddress6(), false, 52, 64);
  y++;
  mso.addCheckBox(y++, x, "tcpenable", "Enable HTTPS/JSON API:", global->getHTTPServer()->getEnabled());
  mso.addStringField(y++, x, "tcpport", "HTTPS/JSON API Port:", std::to_string(global->getHTTPServer()->getPort()), false, 5);
  std::shared_ptr<MenuSelectOptionTextArrow> udpenable = mso.addTextArrow(y++, x, "udpenable", "Enable UDP API:");
  udpenable->addOption("No", static_cast<int>(UdpEnable::NO));
  udpenable->addOption("Plaintext", static_cast<int>(UdpEnable::PLAINTEXT));
  udpenable->addOption("Encrypted", static_cast<int>(UdpEnable::ENCRYPTED));
  UdpEnable udpenabled = UdpEnable::NO;
  if (rch->isEnabled()) {
    udpenabled = rch->isEncrypted() ? UdpEnable::ENCRYPTED : UdpEnable::PLAINTEXT;
  }
  udpenable->setOption(static_cast<int>(udpenabled));
  mso.addStringField(y++, x, "udpport", "UDP API Port:", std::to_string(rch->getUDPPort()), false, 5);
  mso.addStringField(y++, x, "apipass", "API password:", rch->getPassword(), true);
  std::shared_ptr<MenuSelectOptionTextArrow> bell = mso.addTextArrow(y++, x, "udpbell", "Remote command bell:");
  bell->addOption("Disabled", static_cast<int>(RemoteCommandNotify::DISABLED));
  bell->addOption("Action requested", static_cast<int>(RemoteCommandNotify::ACTION_REQUESTED));
  bell->addOption("Jobs added", static_cast<int>(RemoteCommandNotify::JOBS_ADDED));
  bell->addOption("All commands", static_cast<int>(RemoteCommandNotify::ALL_COMMANDS));
  bell->setOption(static_cast<int>(rch->getNotify()));
  mso.addStringField(y++, x, "preparedraceexpirytime", "Prepared spread job expiration time:", std::to_string(global->getEngine()->getPreparedRaceExpiryTime()), false, 5);
  std::shared_ptr<MenuSelectOptionTextArrow> racestarterexpiry = mso.addTextArrow(y++, x, "racestarterexpiry", "Next prepared spread job starter timeout:");
  racestarterexpiry->addOption("30s", 30);
  racestarterexpiry->addOption("1m", 60);
  racestarterexpiry->addOption("3m", 180);
  racestarterexpiry->addOption("5m", 300);
  racestarterexpiry->addOption("10m", 600);
  racestarterexpiry->addOption("30m", 1800);
  racestarterexpiry->addOption("1h", 3600);
  racestarterexpiry->addOption("Unlimited", 0);
  racestarterexpiry->setOption(global->getEngine()->getNextPreparedRaceStarterTimeout());
  y++;
  std::shared_ptr<MenuSelectOptionTextArrow> maxspreadjobtimeseconds = mso.addTextArrow(y++, x, "maxspreadjobtimeseconds", "Max spread job runtime:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxtransfertimeseconds = mso.addTextArrow(y++, x, "maxtransfertimeseconds", "Max transfer runtime:");
  addRuntimeOptions(maxspreadjobtimeseconds);
  addRuntimeOptions(maxtransfertimeseconds);
  maxspreadjobtimeseconds->setOption(global->getEngine()->getMaxSpreadJobTimeSeconds());
  maxtransfertimeseconds->setOption(global->getTransferManager()->getMaxTransferTimeSeconds());
  y++;
  std::shared_ptr<MenuSelectOptionTextArrow> spreadjobhistory = mso.addTextArrow(y++, x, "spreadjobhistory", "Spread job history:");
  std::shared_ptr<MenuSelectOptionTextArrow> transferjobhistory = mso.addTextArrow(y++, x, "transferjobhistory", "Transfer job history:");
  std::shared_ptr<MenuSelectOptionTextArrow> transferhistory = mso.addTextArrow(y++, x, "transferhistory", "Transfer history:");
  addHistoryOptions(spreadjobhistory);
  addHistoryOptions(transferjobhistory);
  addHistoryOptions(transferhistory, false);
  spreadjobhistory->setOption(global->getEngine()->getMaxSpreadJobsHistory());
  transferjobhistory->setOption(global->getEngine()->getMaxTransferJobsHistory());
  transferhistory->setOption(global->getTransferManager()->getMaxTransferHistory());
  std::shared_ptr<MenuSelectOptionTextArrow> logbufferhistory = mso.addTextArrow(y++, x, "logbufferhistory", "Log buffer history:");
  logbufferhistory->addOption("16", 16);
  logbufferhistory->addOption("64", 64);
  logbufferhistory->addOption("256", 256);
  logbufferhistory->addOption("1024", 1024);
  logbufferhistory->addOption("4096", 4096);
  logbufferhistory->addOption("16384", 16384);
  logbufferhistory->addOption("65536", 65536);
  logbufferhistory->addOption("Unlimited", -1);
  logbufferhistory->setOption(global->getLogManager()->getMaxRawbufLines());
  y++;
  std::shared_ptr<MenuSelectOptionTextArrow> legendmode = mso.addTextArrow(y++, x, "legendmode", "Legend bar:");
  legendmode->addOption("Disabled", LEGEND_DISABLED);
  legendmode->addOption("Scrolling", LEGEND_SCROLLING);
  legendmode->addOption("Static", LEGEND_STATIC);
  legendmode->setOption(ui->getLegendMode());
  mso.addCheckBox(y++, x, "highlightentireline", "Highlight entire lines:", ui->getHighlightEntireLine());
  if (isYearEnd()) {
    mso.addCheckBox(y++, x, "xmastree", "Show xmas tree:", ui->getShowXmasTree());
  }
  y++;
  mso.addStringField(y++, x, "defuser", "Default site username:", sm->getDefaultUserName(), false);
  mso.addStringField(y++, x, "defpass", "Default site password:", sm->getDefaultPassword(), true);
  std::shared_ptr<MenuSelectOptionTextArrow> logins = mso.addTextArrow(y++, x, "deflogins", "Default site login slots:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxup = mso.addTextArrow(y++, x, "defmaxup", "Default site upload slots:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxdn = mso.addTextArrow(y++, x, "defmaxdn", "Default site download slots:");
  logins->addOption("Many", -1);
  maxup->addOption("All", -1);
  maxdn->addOption("All", -1);
  for (unsigned int i = 0; i < 100; ++i) {
    std::string num = std::to_string(i);
    if (i > 0) {
      logins->addOption(num, i);
    }
    maxup->addOption(num, i);
    maxdn->addOption(num, i);
  }
  logins->setOption(sm->getDefaultMaxLogins());
  maxup->setOption(sm->getDefaultMaxUp());
  maxdn->setOption(sm->getDefaultMaxDown());
  std::shared_ptr<MenuSelectOptionTextArrow> tlsmode = mso.addTextArrow(y++, x, "tlsmode", "Default TLS mode:");
  tlsmode->addOption("None", static_cast<int>(TLSMode::NONE));
  tlsmode->addOption("AUTH TLS", static_cast<int>(TLSMode::AUTH_TLS));
  tlsmode->addOption("Implicit", static_cast<int>(TLSMode::IMPLICIT));
  tlsmode->setOption(static_cast<int>(sm->getDefaultTLSMode()));
  std::shared_ptr<MenuSelectOptionTextArrow> sslfxp = mso.addTextArrow(y++, x, "tlsfxp", "Default TLS transfers:");
  sslfxp->addOption("Always off", SITE_SSL_ALWAYS_OFF);
  sslfxp->addOption("Prefer off", SITE_SSL_PREFER_OFF);
  sslfxp->addOption("Prefer on", SITE_SSL_PREFER_ON);
  sslfxp->addOption("Always on", SITE_SSL_ALWAYS_ON);
  sslfxp->setOption(sm->getDefaultSSLTransferPolicy());
  mso.addStringField(y++, x, "defidletime", "Default site max idle time (s):", std::to_string(sm->getDefaultMaxIdleTime()), false);
  y++;
  mso.addStringField(y++, x, "dlpath", "Download path:", ls->getDownloadPath().toString(), false, 53, 128);
  y++;
  mso.addTextButtonNoContent(y++, x, "skiplist", "Configure skiplist...");
  mso.addTextButtonNoContent(y++, x, "proxy", "Configure proxy settings...");
  mso.addTextButtonNoContent(y++, x, "fileviewer", "Configure file viewing...");
  mso.addTextButtonNoContent(y++, x, "globalkeybinds", "Configure global keybinds...");
  mso.addTextButtonNoContent(y, x, "disableencryption", "Disable data file encryption...");
  mso.addTextButtonNoContent(y++, x, "enableencryption", "Enable data file encryption...");
  mso.addTextButtonNoContent(y++, x, "changekey", "Change data file encryption key...");
  init(row, col);
}

void GlobalOptionsScreen::redraw() {
  vv->clear();
  std::shared_ptr<MenuSelectOptionElement> disableencryption = mso.getElement("disableencryption");
  std::shared_ptr<MenuSelectOptionElement> enableencryption = mso.getElement("enableencryption");
  std::shared_ptr<MenuSelectOptionElement> changekey = mso.getElement("changekey");
  if (global->getSettingsLoaderSaver()->getState() == DataFileState::EXISTS_DECRYPTED) {
    disableencryption->show();
    changekey->show();
    enableencryption->hide();
    if (mso.getElement(mso.getSelectionPointer()) == enableencryption) {
      mso.setPointer(disableencryption);
    }
  }
  else {
    disableencryption->hide();
    changekey->hide();
    enableencryption->show();
    if (mso.getElement(mso.getSelectionPointer()) == disableencryption) {
      mso.setPointer(enableencryption);
    }
  }
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    if (!msoe->visible()) {
      continue;
    }
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void GlobalOptionsScreen::update() {
  redraw();
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    vv->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool GlobalOptionsScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  std::shared_ptr<MenuSelectOptionElement> msoe;
  switch(action) {
    case KEYACTION_UP: {
      bool moved = mso.goUp();
      ui->update();
      return moved;
    }
    case KEYACTION_DOWN: {
      bool moved = mso.goDown();
      ui->update();
      return moved;
    }
    case KEYACTION_ENTER:
      msoe = mso.getElement(mso.getSelectionPointer());
      if (msoe->getIdentifier() == "skiplist") {
        ui->goSkiplist();
        return true;
      }
      if (msoe->getIdentifier() == "changekey") {
        ui->goChangeKey();
        return true;
      }
      if (msoe->getIdentifier() == "disableencryption") {
        ui->goDisableEncryption();
        return true;
      }
      if (msoe->getIdentifier() == "enableencryption") {
        ui->goEnableEncryption();
        return true;
      }
      if (msoe->getIdentifier() == "proxy") {
        ui->goProxy();
        return true;
      }
      if (msoe->getIdentifier() == "fileviewer") {
        ui->goFileViewerSettings();
        return true;
      }
      if (msoe->getIdentifier() == "globalkeybinds") {
        ui->goGlobalKeyBinds();
        return true;
      }
      activation = msoe->activate();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = msoe;
      ui->setLegend();
      ui->update();
      return true;
    case KEYACTION_BACK_CANCEL:
      global->getSettingsLoaderSaver()->saveSettings();
      ui->returnToLast();
      return true;
    case KEYACTION_SKIPLIST:
      ui->goSkiplist();
      return true;
    case KEYACTION_DONE:
      UdpEnable udpenable = UdpEnable::NO;
      for(unsigned int i = 0; i < mso.size(); i++) {
        std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "bindinterface") {
          std::string interface = interfacemap[std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()];
          global->getIOManager()->setBindInterface(interface);
        }
        else if (identifier == "activeportrange") {
          std::string portrange = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
          size_t splitpos = portrange.find(":");
          if (splitpos != std::string::npos) {
            int portfirst = std::stoi(portrange.substr(0, splitpos));
            int portlast = std::stoi(portrange.substr(splitpos + 1));
            ls->setActivePortFirst(portfirst);
            ls->setActivePortLast(portlast);
          }
        }
        else if (identifier == "transferprotocol") {
          ls->setTransferProtocol(static_cast<TransferProtocol>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "useactiveaddress") {
          ls->setUseActiveModeAddress(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "activeaddress4") {
          ls->setActiveModeAddress4(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "activeaddress6") {
          ls->setActiveModeAddress6(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "tcpenable") {
          global->getHTTPServer()->setEnabled(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "tcpport") {
          global->getHTTPServer()->setPort(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "udpenable") {
          udpenable = static_cast<UdpEnable>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
          if (udpenable == UdpEnable::NO) {
            rch->setEnabled(false);
          }
          else {
            rch->setEncrypted(udpenable == UdpEnable::ENCRYPTED);
          }
        }
        else if (identifier == "udpport") {
          rch->setPort(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "apipass") {
          rch->setPassword(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "udpbell") {
          rch->setNotify(static_cast<RemoteCommandNotify>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "defuser") {
          sm->setDefaultUserName(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "defpass") {
          sm->setDefaultPassword(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "deflogins") {
          sm->setDefaultMaxLogins(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "defmaxup") {
          sm->setDefaultMaxUp(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "defmaxdn") {
          sm->setDefaultMaxDown(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "tlsmode") {
          sm->setDefaultTLSMode(static_cast<TLSMode>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "tlsfxp") {
          sm->setDefaultSSLTransferPolicy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "defidletime") {
          sm->setDefaultMaxIdleTime(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "legendmode") {
          ui->setLegendMode((LegendMode)std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "highlightentireline") {
          ui->setHighlightEntireLine(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "xmastree") {
          ui->setShowXmasTree(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "dlpath") {
          ls->setDownloadPath(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "preparedraceexpirytime") {
          global->getEngine()->setPreparedRaceExpiryTime(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "racestarterexpiry") {
          global->getEngine()->setNextPreparedRaceStarterTimeout(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "spreadjobhistory") {
          global->getEngine()->setMaxSpreadJobsHistory(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "transferjobhistory") {
          global->getEngine()->setMaxTransferJobsHistory(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "transferhistory") {
          global->getTransferManager()->setMaxTransferHistory(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "logbufferhistory") {
          global->getLogManager()->setMaxRawbufLines(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "maxspreadjobtimeseconds") {
          global->getEngine()->setMaxSpreadJobTimeSeconds(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "maxtransfertimeseconds") {
          global->getTransferManager()->setMaxTransferTimeSeconds(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
      }
      rch->setEnabled(udpenable != UdpEnable::NO);
      global->getSettingsLoaderSaver()->saveSettings();
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string GlobalOptionsScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string GlobalOptionsScreen::getInfoLabel() const {
  return "GLOBAL SETTINGS";
}
