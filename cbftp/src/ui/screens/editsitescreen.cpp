#include "editsitescreen.h"

#include <set>

#include "../../globalcontext.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../proxymanager.h"
#include "../../proxy.h"
#include "../../util.h"
#include "../../settingsloadersaver.h"
#include "../../transferprotocol.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncontainer.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextarrow.h"

namespace {

enum KeyAction {
  KEYACTION_SECTIONS
};

void addRuntimeOptions(std::shared_ptr<MenuSelectOptionTextArrow>& arrow) {
  arrow->addOption("Default", -1);
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

EditSiteScreen::EditSiteScreen(Ui* ui) : UIWindow(ui, "EditSiteScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind('S', KEYACTION_SKIPLIST, "Skiplist");
  keybinds.addBind('s', KEYACTION_SECTIONS, "Sections");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Navigate left");
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Navigate right");
}

EditSiteScreen::~EditSiteScreen() {

}

void EditSiteScreen::initialize(unsigned int row, unsigned int col, const std::string & operation, const std::string & site) {
  active = false;
  slotsupdated = false;
  this->operation = operation;
  std::string exceptsrc = "";
  std::string exceptdst = "";
  if (operation == "add") {
    this->site = global->getSiteManager()->createNewSite();
    std::vector<std::shared_ptr<Site> >::const_iterator it;
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      if ((*it)->getTransferTargetPolicy() == SITE_TRANSFER_POLICY_BLOCK) {
        exceptsrc += (*it)->getName() + ",";
      }
      if ((*it)->getTransferSourcePolicy() == SITE_TRANSFER_POLICY_BLOCK) {
        exceptdst += (*it)->getName() + ",";
      }
    }
  }
  else if (operation == "edit") {
    this->site = global->getSiteManager()->getSite(site);
    std::set<std::shared_ptr<Site> >::const_iterator it;
    for (it = this->site->exceptSourceSitesBegin(); it != this->site->exceptSourceSitesEnd(); it++) {
      exceptsrc += (*it)->getName() + ",";
    }
    for (it = this->site->exceptTargetSitesBegin(); it != this->site->exceptTargetSitesEnd(); it++) {
      exceptdst += (*it)->getName() + ",";
    }
  }
  this->modsite = std::make_shared<Site>(*this->site.get());
  if (exceptsrc.length()) {
    exceptsrc = exceptsrc.substr(0, exceptsrc.length() - 1);
  }
  if (exceptdst.length()) {
    exceptdst = exceptdst.substr(0, exceptdst.length() - 1);
  }
  std::string affilstr = "";
  std::set<std::string, util::naturalComparator>::const_iterator it;
  for (it = this->site->affilsBegin(); it != this->site->affilsEnd(); it++) {
    affilstr += *it + " ";
  }
  if (affilstr.length() > 0) {
    affilstr = affilstr.substr(0, affilstr.length() - 1);
  }

  unsigned int y = 1;
  unsigned int x = 1;

  mso.reset();
  mso.addStringField(y, x, "name", "Name:", this->site->getName(), false, 10, 64);
  std::shared_ptr<MenuSelectOptionTextField> msotf = mso.addStringField(y++, x + 17, "addr", "Address:", this->site->getAddressesAsString(), false, 48, 512);
  msotf->setExtraLegendText("Multiple sets of address:port separated by space or semicolon");
  std::shared_ptr<MenuSelectOptionTextArrow> tlsmode = mso.addTextArrow(y, x, "tlsmode", "TLS mode:");
  tlsmode->addOption("None", static_cast<int>(TLSMode::NONE));
  tlsmode->addOption("AUTH TLS", static_cast<int>(TLSMode::AUTH_TLS));
  tlsmode->addOption("Implicit", static_cast<int>(TLSMode::IMPLICIT));
  tlsmode->setOption(static_cast<int>(this->site->getTLSMode()));
  mso.addStringField(y, x + 23, "user", "Username:", this->site->getUser(), false, 10, 64);
  mso.addStringField(y++, x + 45, "pass", "Password:", this->site->getPass(), true, 22, 128);
  std::shared_ptr<MenuSelectOptionTextArrow> logins = mso.addTextArrow(y, x, "logins", "Login slots:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxup = mso.addTextArrow(y, x + 22, "maxup", "Upload slots:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxdn = mso.addTextArrow(y++, x + 44, "maxdn", "Download slots:");
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
  logins->setOption(this->site->getInternMaxLogins());
  maxup->setOption(this->site->getInternMaxUp());
  maxdn->setOption(this->site->getInternMaxDown());
  mso.addTextButtonNoContent(y++, x, "slots", "Advanced slot configuration...");
  std::shared_ptr<MenuSelectOptionTextArrow> sslfxp = mso.addTextArrow(y, x, "tlstransfer", "TLS transfers:");
  sslfxp->addOption("Always off", SITE_SSL_ALWAYS_OFF);
  sslfxp->addOption("Prefer off", SITE_SSL_PREFER_OFF);
  sslfxp->addOption("Prefer on", SITE_SSL_PREFER_ON);
  sslfxp->addOption("Always on", SITE_SSL_ALWAYS_ON);
  sslfxp->setOption(this->site->getSSLTransferPolicy());
  std::shared_ptr<MenuSelectOptionTextArrow> transferproto = mso.addTextArrow(y++, x + 30, "transferprotocol", "Transfer protocol:");
  transferproto->addOption("IPv4 only", static_cast<int>(TransferProtocol::IPV4_ONLY));
  transferproto->addOption("Prefer IPv4", static_cast<int>(TransferProtocol::PREFER_IPV4));
  transferproto->addOption("Prefer IPv6", static_cast<int>(TransferProtocol::PREFER_IPV6));
  transferproto->addOption("IPv6 only", static_cast<int>(TransferProtocol::IPV6_ONLY));
  transferproto->setOption(static_cast<int>(this->site->getTransferProtocol()));
  mso.addCheckBox(y, x, "stayloggedin", "Stay logged in:", this->site->getStayLoggedIn());
  std::shared_ptr<MenuSelectOptionTextArrow> listcommand = mso.addTextArrow(y, x + 21, "listcommand", "List command:");
  listcommand->addOption("STAT -l", SITE_LIST_STAT);
  listcommand->addOption("LIST", SITE_LIST_LIST);
  listcommand->setOption(this->site->getListCommand());
  mso.addStringField(y++, x + 47, "basepath", "Base path:", this->site->getBasePath().toString(), false, 19, 512);
  mso.addCheckBox(y, x, "cepr", "CEPR supported:", this->site->supportsCEPR());
  mso.addCheckBox(y, x + 21, "sscn", "SSCN supported:", this->site->supportsSSCN());
  mso.addCheckBox(y++, x + 42, "cpsv", "CPSV supported:", this->site->supportsCPSV());
  mso.addCheckBox(y, x, "binary", "Force binary mode:", this->site->forceBinaryMode());
  mso.addCheckBox(y++, x + 23, "brokenpasv", "Broken PASV:", this->site->hasBrokenPASV());
  mso.addStringField(y, x, "idletime", "Max idle time (s):", std::to_string(this->site->getMaxIdleTime()), false, 4);
  mso.addCheckBox(y, x + 25, "xdupe", "Use XDUPE:", this->site->useXDUPE());
  mso.addCheckBox(y++, x + 41, "pret", "Needs PRET:", this->site->needsPRET());
  std::shared_ptr<MenuSelectOptionTextArrow> useproxy = mso.addTextArrow(y, x, "useproxy", "Proxy:");
  std::shared_ptr<MenuSelectOptionTextArrow> usedataproxy = mso.addTextArrow(y++, x + 25, "dataproxy", "Data proxy:");
  ProxyManager * pm = global->getProxyManager();
  Proxy * proxy = pm->getDefaultProxy();
  Proxy * dataproxy = pm->getDefaultDataProxy();
  std::string globalproxyname = "None";
  if (proxy != nullptr) {
   globalproxyname = proxy->getName();
  }
  std::string globaldataproxyname = "None";
  if (dataproxy != nullptr) {
   globaldataproxyname = dataproxy->getName();
  }
  useproxy->addOption("(Global) " + globalproxyname, SITE_PROXY_GLOBAL);
  usedataproxy->addOption("(Global) " + globaldataproxyname, SITE_PROXY_GLOBAL);
  useproxy->addOption("None", SITE_PROXY_NONE);
  usedataproxy->addOption("None", SITE_PROXY_NONE);
  for (std::vector<Proxy *>::const_iterator it = pm->begin(); it != pm->end(); it++) {
   useproxy->addOption((*it)->getName(), SITE_PROXY_USE);
   usedataproxy->addOption((*it)->getName(), SITE_PROXY_USE);
  }
  int proxytype = this->site->getProxyType();
  int dataproxytype = this->site->getDataProxyType();
  useproxy->setOption(proxytype);
  usedataproxy->setOption(dataproxytype);
  if (proxytype == SITE_PROXY_USE) {
   useproxy->setOptionText(this->site->getProxy());
  }
  if (dataproxytype == SITE_PROXY_USE) {
   usedataproxy->setOptionText(this->site->getDataProxy());
  }
  mso.addTextButtonNoContent(y++, x, "skiplist", "Configure skiplist...");
  y++;
  mso.addCheckBox(y, x, "disabled", "Disabled:", this->site->getDisabled());
  std::shared_ptr<MenuSelectOptionTextArrow> allowupload = mso.addTextArrow(y, x + 15, "allowupload", "Allow upload:");
  allowupload->addOption("No", SITE_ALLOW_TRANSFER_NO);
  allowupload->addOption("Yes", SITE_ALLOW_TRANSFER_YES);
  allowupload->setOption(this->site->getAllowUpload());
  std::shared_ptr<MenuSelectOptionTextArrow> allowdownload = mso.addTextArrow(y++, x + 37, "allowdownload", "Allow download:");
  allowdownload->addOption("No", SITE_ALLOW_TRANSFER_NO);
  allowdownload->addOption("Yes", SITE_ALLOW_TRANSFER_YES);
  allowdownload->addOption("Affils only", SITE_ALLOW_DOWNLOAD_MATCH_ONLY);
  allowdownload->setOption(this->site->getAllowDownload());
  std::shared_ptr<MenuSelectOptionTextArrow> priority = mso.addTextArrow(y, x, "priority", "Priority:");
  priority->addOption(Site::getPriorityText(SitePriority::VERY_LOW), static_cast<int>(SitePriority::VERY_LOW));
  priority->addOption(Site::getPriorityText(SitePriority::LOW), static_cast<int>(SitePriority::LOW));
  priority->addOption(Site::getPriorityText(SitePriority::NORMAL), static_cast<int>(SitePriority::NORMAL));
  priority->addOption(Site::getPriorityText(SitePriority::HIGH), static_cast<int>(SitePriority::HIGH));
  priority->addOption(Site::getPriorityText(SitePriority::VERY_HIGH), static_cast<int>(SitePriority::VERY_HIGH));
  priority->setOption(static_cast<int>(this->site->getPriority()));
  std::shared_ptr<MenuSelectOptionTextArrow> refreshrate = mso.addTextArrow(y++, x + 24, "refreshrate", "List frequency:");
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::VERY_LOW), static_cast<int>(RefreshRate::VERY_LOW));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::FIXED_LOW), static_cast<int>(RefreshRate::FIXED_LOW));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::FIXED_AVERAGE), static_cast<int>(RefreshRate::FIXED_AVERAGE));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::FIXED_HIGH), static_cast<int>(RefreshRate::FIXED_HIGH));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::FIXED_VERY_HIGH), static_cast<int>(RefreshRate::FIXED_VERY_HIGH));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::AUTO), static_cast<int>(RefreshRate::AUTO));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::DYNAMIC_LOW), static_cast<int>(RefreshRate::DYNAMIC_LOW));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::DYNAMIC_AVERAGE), static_cast<int>(RefreshRate::DYNAMIC_AVERAGE));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::DYNAMIC_HIGH), static_cast<int>(RefreshRate::DYNAMIC_HIGH));
  refreshrate->addOption(Site::getRefreshRateText(RefreshRate::DYNAMIC_VERY_HIGH), static_cast<int>(RefreshRate::DYNAMIC_VERY_HIGH));
  refreshrate->setOption(static_cast<int>(this->site->getRefreshRate()));
  std::shared_ptr<MenuSelectOptionTextArrow> sourcepolicy = mso.addTextArrow(y, x, "sourcepolicy", "Transfer source policy:");
  sourcepolicy->addOption("Allow", SITE_TRANSFER_POLICY_ALLOW);
  sourcepolicy->addOption("Block", SITE_TRANSFER_POLICY_BLOCK);
  sourcepolicy->setOption(this->site->getTransferSourcePolicy());
  std::shared_ptr<MenuSelectOptionTextArrow> targetpolicy = mso.addTextArrow(y++, x + 34, "targetpolicy", "Transfer target policy:");
  targetpolicy->addOption("Allow", SITE_TRANSFER_POLICY_ALLOW);
  targetpolicy->addOption("Block", SITE_TRANSFER_POLICY_BLOCK);
  targetpolicy->setOption(this->site->getTransferTargetPolicy());
  mso.addStringField(y++, x, "exceptsrc", "", exceptsrc, false, 50, 512);

  mso.addStringField(y++, x, "exceptdst", "", exceptdst, false, 50, 512);
  mso.addStringField(y++, x, "affils", "Affils:", affilstr, false, 60, 1024);
  mso.addTextButtonNoContent(y++, x, "sections", "Configure sections...");
  std::shared_ptr<MenuSelectOptionTextArrow> maxspreadjobtimeseconds = mso.addTextArrow(y++, x, "maxspreadjobtimeseconds", "Max spread job runtime:");
  std::shared_ptr<MenuSelectOptionTextArrow> maxtransfertimeseconds = mso.addTextArrow(y++, x, "maxtransfertimeseconds", "Max transfer runtime:");
  addRuntimeOptions(maxspreadjobtimeseconds);
  addRuntimeOptions(maxtransfertimeseconds);
  maxspreadjobtimeseconds->setOption(this->site->getMaxSpreadJobTimeSeconds());
  maxtransfertimeseconds->setOption(this->site->getMaxTransferTimeSeconds());
  y++;
  mso.enterFocusFrom(0);
  init(row, col);
}

void EditSiteScreen::redraw() {
  vv->clear();
  if (slotsupdated) {
    std::shared_ptr<MenuSelectOptionTextArrow> logins = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("logins"));
    std::shared_ptr<MenuSelectOptionTextArrow> maxup = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxup"));
    std::shared_ptr<MenuSelectOptionTextArrow> maxdn = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxdn"));
    logins->setOption(modsite->getInternMaxLogins());
    maxup->setOption(modsite->getInternMaxUp());
    maxdn->setOption(modsite->getInternMaxDown());
    slotsupdated = false;
  }
  std::string slotslabel = "Advanced slot configuration...";
  bool free = modsite->getLeaveFreeSlot();
  int maxdnpre = modsite->getInternMaxDownPre();
  int maxdncomplete = modsite->getInternMaxDownComplete();
  int maxdntransferjob = modsite->getInternMaxDownTransferJob();
  std::string maxdnprestr = maxdnpre == -1 ? "All" : (maxdnpre == -2 ? "Normal" : std::to_string(maxdnpre));
  std::string maxdncompletestr = maxdncomplete == -1 ? "All" : (maxdncomplete == -2 ? "Normal" : std::to_string(maxdncomplete));
  std::string maxdntransferjobstr = maxdntransferjob == -1 ? "All" : (maxdntransferjob == -2 ? "Normal" : std::to_string(maxdntransferjob));
  if (free || maxdnpre || maxdncomplete || maxdntransferjob) {
    slotslabel = slotslabel + " (" + (free ? "free/" : "") + maxdnprestr + "/" + maxdncompletestr + "/" + maxdntransferjobstr + ")";
  }
  mso.getElement("slots")->setLabel(slotslabel);
  if (std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("sourcepolicy"))->getData() == SITE_TRANSFER_POLICY_ALLOW) {
    mso.getElement("exceptsrc")->setLabel("Block transfers from:");
  }
  else {
    mso.getElement("exceptsrc")->setLabel("Allow transfers from:");
  }
  if (std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("targetpolicy"))->getData() == SITE_TRANSFER_POLICY_ALLOW) {
    mso.getElement("exceptdst")->setLabel("Block transfers to:");
  }
  else {
    mso.getElement("exceptdst")->setLabel("Allow transfers to:");
  }
  std::string sectionslabel = "Configure sections...";
  unsigned int sectionssize = this->modsite->getSections().size();
  if (sectionssize) {
    sectionslabel += " (" + std::to_string(sectionssize) + ")";
  }
  std::string skiplistlabel = "Configure skiplist...";
  unsigned int skiplistsize = this->modsite->getSkipList().size();
  if (skiplistsize) {
    skiplistlabel += " (" + std::to_string(skiplistsize) + ")";
  }
  mso.getElement("sections")->setLabel(sectionslabel);
  mso.getElement("skiplist")->setLabel(skiplistlabel);
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
  vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    vv->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

void EditSiteScreen::command(const std::string & command, const std::string & arg) {
  if (command == "returnselectitems") {
    std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->setText(arg);
    ui->redraw();
  }
}

bool EditSiteScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      std::string identifier = activeelement->getIdentifier();
      activeelement->deactivate();
      active = false;
      if (identifier == "sourcepolicy" || identifier == "targetpolicy") {
        ui->redraw();
      }
      else {
        ui->update();
      }
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  std::list<std::string> exceptsrclist;
  std::list<std::string> exceptdstlist;
  std::string sitename;
  switch(action) {
    case KEYACTION_UP:
      if (mso.goUp()) {
        ui->redraw();
        return true;
      }
      return false;
    case KEYACTION_DOWN:
      if (mso.goDown()) {
        ui->redraw();
        return true;
      }
      return false;
    case KEYACTION_LEFT:
      if (mso.goLeft()) {
        ui->redraw();
        return true;
      }
      return false;
    case KEYACTION_RIGHT:
      if (mso.goRight()) {
        ui->redraw();
        return true;
      }
      return false;
    case KEYACTION_ENTER: {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      if (msoe->getIdentifier() == "skiplist") {
        modsite->setName(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("name"))->getData());
        ui->goSkiplist((SkipList *)&modsite->getSkipList());
        return true;
      }
      if (msoe->getIdentifier() == "sections") {
        modsite->setName(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("name"))->getData());
        ui->goSiteSections(modsite);
        return true;
      }
      if (msoe->getIdentifier() == "slots") {
        modsite->setName(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("name"))->getData());
        std::shared_ptr<MenuSelectOptionTextArrow> logins = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("logins"));
        std::shared_ptr<MenuSelectOptionTextArrow> maxup = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxup"));
        std::shared_ptr<MenuSelectOptionTextArrow> maxdn = std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("maxdn"));
        modsite->setMaxLogins(logins->getData());
        modsite->setMaxUp(maxup->getData());
        modsite->setMaxDn(maxdn->getData());
        ui->goSiteSlots(modsite);
        slotsupdated = true;
        return true;
      }
      activation = mso.activateSelected();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      if (activeelement->getIdentifier() == "exceptsrc") {
        activeelement->deactivate();
        active = false;
        std::string preselectstr = std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->getData();
        std::list<std::shared_ptr<Site> > preselected;
        fillPreselectionList(preselectstr, &preselected);
        std::list<std::shared_ptr<Site> > excluded;
        excluded.push_back(site);
        std::string action = "Block";
        if (std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("sourcepolicy"))->getData() == SITE_TRANSFER_POLICY_BLOCK) {
          action = "Allow";
        }
        ui->goSelectSites(action + " race transfers from these sites", preselected, excluded);
        return true;
      }
      if (activeelement->getIdentifier() == "exceptdst") {
        activeelement->deactivate();
        active = false;
        std::string preselectstr = std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->getData();
        std::list<std::shared_ptr<Site> > preselected;
        fillPreselectionList(preselectstr, &preselected);
        std::list<std::shared_ptr<Site> > excluded;
        excluded.push_back(site);
        std::string action = "Block";
        if (std::static_pointer_cast<MenuSelectOptionTextArrow>(mso.getElement("targetpolicy"))->getData() == SITE_TRANSFER_POLICY_BLOCK) {
          action = "Allow";
        }
        ui->goSelectSites(action + " race transfers to these sites", preselected, excluded);
        return true;
      }
      ui->update();
      ui->setLegend();
      return true;
    }
    case KEYACTION_DONE: {
      std::string newname = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("name"))->getData();
      bool changedname = newname != site->getName() && operation == "edit";
      if ((changedname || operation == "add") && !!global->getSiteManager()->getSite(newname)) {
        ui->goInfo("A site with that name already exists. Please choose another name.");
        return true;
      }
      site->setName(newname);
      for(unsigned int i = 0; i < mso.size(); i++) {
        std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "addr") {
          std::string addrports = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
          site->setAddresses(addrports);
        }
        else if (identifier == "user") {
          site->setUser(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "pass") {
          site->setPass(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "basepath") {
          site->setBasePath(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "logins") {
          site->setMaxLogins(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "maxup") {
          site->setMaxUp(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "maxdn") {
          site->setMaxDn(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "pret") {
          site->setPRET(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "binary") {
          site->setForceBinaryMode(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "tlsmode") {
          site->setTLSMode(static_cast<TLSMode>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "tlstransfer") {
          site->setSSLTransferPolicy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "transferprotocol") {
          site->setTransferProtocol(static_cast<TransferProtocol>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "sscn") {
          site->setSupportsSSCN(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "cpsv") {
          site->setSupportsCPSV(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "cepr") {
          site->setSupportsCEPR(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "stayloggedin") {
          site->setStayLoggedIn(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "listcommand") {
          site->setListCommand(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "xdupe") {
          site->setUseXDUPE(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "disabled") {
          site->setDisabled(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "allowupload") {
          site->setAllowUpload(static_cast<SiteAllowTransfer>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "allowdownload") {
          site->setAllowDownload(static_cast<SiteAllowTransfer>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "priority") {
          site->setPriority(static_cast<SitePriority>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "refreshrate") {
          site->setRefreshRate(static_cast<RefreshRate>(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData()));
        }
        else if (identifier == "brokenpasv") {
          site->setBrokenPASV(std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData());
        }
        else if (identifier == "idletime") {
          site->setMaxIdleTime(std::stoi(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData()));
        }
        else if (identifier == "useproxy") {
          int proxytype = std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData();
          site->setProxyType(proxytype);
          if (proxytype == SITE_PROXY_USE) {
            site->setProxy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getDataText());
          }
        }
        else if (identifier == "dataproxy") {
          int proxytype = std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData();
          site->setDataProxyType(proxytype);
          if (proxytype == SITE_PROXY_USE) {
            site->setDataProxy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getDataText());
          }
        }
        else if (identifier == "affils") {
          std::string affils = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
          site->clearAffils();
          size_t pos;
          while ((pos = affils.find(",")) != std::string::npos) {
            affils[pos] = ' ';
          }
          while ((pos = affils.find(";")) != std::string::npos) {
            affils[pos] = ' ';
          }
          std::list<std::string> affilslist = util::trim(util::split(affils));
          for (std::list<std::string>::const_iterator it = affilslist.begin(); it != affilslist.end(); it++) {
            site->addAffil(*it);
          }
        }
        else if (identifier == "sourcepolicy") {
          site->setTransferSourcePolicy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "targetpolicy") {
          site->setTransferTargetPolicy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "exceptsrc") {
          std::string sitestr = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
          exceptsrclist = util::trim(util::split(sitestr, ","));
        }
        else if (identifier == "exceptdst") {
          std::string sitestr = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
          exceptdstlist = util::trim(util::split(sitestr, ","));
        }
        else if (identifier == "maxspreadjobtimeseconds") {
          site->setMaxSpreadJobTimeSeconds(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
        else if (identifier == "maxtransfertimeseconds") {
          site->setMaxTransferTimeSeconds(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
      }
      site->setSkipList(modsite->getSkipList());
      site->setSections(modsite->getSections());

      site->setMaxDnPre(modsite->getInternMaxDownPre());
      site->setMaxDnComplete(modsite->getInternMaxDownComplete());
      site->setMaxDnTransferJob(modsite->getInternMaxDownTransferJob());
      site->setLeaveFreeSlot(modsite->getLeaveFreeSlot());

      if (operation == "add") {
        global->getSiteManager()->addSite(site);
      }
      else {
        global->getSiteManager()->sortSites();
      }

      sitename = site->getName();
      global->getSiteManager()->resetSitePairsForSite(sitename);
      for (std::list<std::string>::const_iterator it = exceptsrclist.begin(); it != exceptsrclist.end(); it++) {
        global->getSiteManager()->addExceptSourceForSite(sitename, *it);
      }
      for (std::list<std::string>::const_iterator it = exceptdstlist.begin(); it != exceptdstlist.end(); it++) {
        global->getSiteManager()->addExceptTargetForSite(sitename, *it);
      }

      global->getSiteLogicManager()->getSiteLogic(site->getName())->siteUpdated();
      if (changedname) {
        global->getSiteLogicManager()->getSiteLogic(site->getName())->updateName();
      }

      global->getSettingsLoaderSaver()->saveSettings();
      ui->returnToLast();
      return true;
    }
    case KEYACTION_SKIPLIST:
      modsite->setName(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("name"))->getData());
      ui->goSkiplist((SkipList *)&modsite->getSkipList());
      return true;
    case KEYACTION_SECTIONS:
      modsite->setName(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("name"))->getData());
      ui->goSiteSections(modsite);
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string EditSiteScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string EditSiteScreen::getInfoLabel() const {
  return "SITE OPTIONS";
}

void EditSiteScreen::fillPreselectionList(const std::string & preselectstr, std::list<std::shared_ptr<Site> > * list) const {
  std::list<std::string> preselectlist = util::trim(util::split(preselectstr, ","));
  for (std::list<std::string>::const_iterator it = preselectlist.begin(); it != preselectlist.end(); it++) {
    std::shared_ptr<Site> site = global->getSiteManager()->getSite(*it);
    list->push_back(site);
  }
}
