#include "ui.h"

#include <iostream>
#include <cassert>
#include <unistd.h>

#include "../core/workmanager.h"
#include "../core/tickpoke.h"
#include "../core/iomanager.h"
#include "../core/signal.h"
#include "../globalcontext.h"
#include "../subprocessmanager.h"
#include "../engine.h"
#include "../datafilehandler.h"
#include "../path.h"
#include "../remotecommandhandler.h"

#include "legendprinterkeybinds.h"
#include "legendprinterspreadjob.h"
#include "legendprintertransferjob.h"
#include "legendwindow.h"
#include "infowindow.h"
#include "uiwindow.h"
#include "chardraw.h"
#include "keybinds.h"

#include "screens/loginscreen.h"
#include "screens/newkeyscreen.h"
#include "screens/mainscreen.h"
#include "screens/editsitescreen.h"
#include "screens/confirmationscreen.h"
#include "screens/sitestatusscreen.h"
#include "screens/rawdatascreen.h"
#include "screens/browsescreen.h"
#include "screens/newracescreen.h"
#include "screens/racestatusscreen.h"
#include "screens/rawcommandscreen.h"
#include "screens/globaloptionsscreen.h"
#include "screens/skiplistscreen.h"
#include "screens/changekeyscreen.h"
#include "screens/eventlogscreen.h"
#include "screens/proxyoptionsscreen.h"
#include "screens/editproxyscreen.h"
#include "screens/viewfilescreen.h"
#include "screens/nukescreen.h"
#include "screens/fileviewersettingsscreen.h"
#include "screens/scoreboardscreen.h"
#include "screens/selectsitesscreen.h"
#include "screens/transfersscreen.h"
#include "screens/transferjobstatusscreen.h"
#include "screens/allracesscreen.h"
#include "screens/alltransferjobsscreen.h"
#include "screens/transferstatusscreen.h"
#include "screens/transfersfilterscreen.h"
#include "screens/infoscreen.h"
#include "screens/selectjobsscreen.h"
#include "screens/makedirscreen.h"
#include "screens/sectionsscreen.h"
#include "screens/editsectionscreen.h"
#include "screens/sitesectionsscreen.h"
#include "screens/editsitesectionscreen.h"
#include "screens/siteslotsscreen.h"
#include "screens/snakescreen.h"
#include "screens/disableencryptionscreen.h"
#include "screens/movescreen.h"
#include "screens/fileinfoscreen.h"
#include "screens/keybindsscreen.h"
#include "screens/metricsscreen.h"
#include "screens/transferpairingscreen.h"
#include "screens/externalscriptsscreen.h"

namespace {

static Ui * instance = new Ui();

enum KeyAction {
  KEYACTION_KILL_ALL,
  KEYACTION_START_PREPARED,
  KEYACTION_FULLSCREEN,
  KEYACTION_NEXT_PREPARED_STARTER,
  KEYACTION_TOGGLE_UDP
};

} // namespace

Ui::Ui() :
  vv(renderer),
  ticker(0),
  haspushed(false),
  pushused(false),
  legendenabled(false),
  infoenabled(false),
  legendmode(LEGEND_SCROLLING),
  highlightentireline(false),
  showxmastree(true),
  split(false),
  fullscreentoggle(false),
  globalkeybinds(std::make_shared<KeyBinds>("Global"))
{
  globalkeybinds->addBind({}, KEYACTION_KILL_ALL, "Kill all external viewers");
  globalkeybinds->addBind('p', KEYACTION_START_PREPARED, "Start latest prepared spread job");
  globalkeybinds->addBind('\\', KEYACTION_FULLSCREEN, "Toggle fullscreen mode");
  globalkeybinds->addBind('N', KEYACTION_NEXT_PREPARED_STARTER, "Toggle next prepared spread job auto starter");
  globalkeybinds->addBind('U', KEYACTION_TOGGLE_UDP, "Toggle UDP");
  addKeyBinds(globalkeybinds.get());
}

Ui::~Ui() {

}

bool Ui::init() {

  Core::registerSignalDispatch(SIGWINCH, global->getWorkManager(), this);

  CharDraw::init();

  global->getSettingsLoaderSaver()->addSettingsAdder(this);

  if (!renderer.init()) {
    return false;
  }
  mainrow = renderer.getRow();
  col = renderer.getCol();
  vv.resize(mainrow, col);

  legendwindow = std::make_shared<LegendWindow>(this, 2, col);
  infowindow = std::make_shared<InfoWindow>(this, 2, col);
  loginscreen = std::make_shared<LoginScreen>(this);
  newkeyscreen = std::make_shared<NewKeyScreen>(this);
  mainscreen = std::make_shared<MainScreen>(this);
  confirmationscreen = std::make_shared<ConfirmationScreen>(this);
  editsitescreen = std::make_shared<EditSiteScreen>(this);
  sitestatusscreen = std::make_shared<SiteStatusScreen>(this);
  rawdatascreen = std::make_shared<RawDataScreen>(this);
  rawcommandscreen = std::make_shared<RawCommandScreen>(this);
  browsescreen = std::make_shared<BrowseScreen>(this);
  newracescreen = std::make_shared<NewRaceScreen>(this);
  racestatusscreen = std::make_shared<RaceStatusScreen>(this);
  globaloptionsscreen = std::make_shared<GlobalOptionsScreen>(this);
  skiplistscreen = std::make_shared<SkipListScreen>(this);
  changekeyscreen = std::make_shared<ChangeKeyScreen>(this);
  eventlogscreen = std::make_shared<EventLogScreen>(this);
  proxyoptionsscreen = std::make_shared<ProxyOptionsScreen>(this);
  editproxyscreen = std::make_shared<EditProxyScreen>(this);
  viewfilescreen = std::make_shared<ViewFileScreen>(this);
  nukescreen = std::make_shared<NukeScreen>(this);
  fileviewersettingsscreen = std::make_shared<FileViewerSettingsScreen>(this);
  scoreboardscreen = std::make_shared<ScoreBoardScreen>(this);
  selectsitesscreen = std::make_shared<SelectSitesScreen>(this);
  transfersscreen = std::make_shared<TransfersScreen>(this);
  transferjobstatusscreen = std::make_shared<TransferJobStatusScreen>(this);
  allracesscreen = std::make_shared<AllRacesScreen>(this);
  alltransferjobsscreen = std::make_shared<AllTransferJobsScreen>(this);
  transferstatusscreen = std::make_shared<TransferStatusScreen>(this);
  transfersfilterscreen = std::make_shared<TransfersFilterScreen>(this);
  infoscreen = std::make_shared<InfoScreen>(this);
  selectjobsscreen = std::make_shared<SelectJobsScreen>(this);
  makedirscreen = std::make_shared<MakeDirScreen>(this);
  sectionsscreen = std::make_shared<SectionsScreen>(this);
  editsectionscreen = std::make_shared<EditSectionScreen>(this);
  sitesectionsscreen = std::make_shared<SiteSectionsScreen>(this);
  editsitesectionscreen = std::make_shared<EditSiteSectionScreen>(this);
  siteslotsscreen = std::make_shared<SiteSlotsScreen>(this);
  snakescreen = std::make_shared<SnakeScreen>(this);
  disableencryptionscreen = std::make_shared<DisableEncryptionScreen>(this);
  movescreen = std::make_shared<MoveScreen>(this);
  fileinfoscreen = std::make_shared<FileInfoScreen>(this);
  keybindsscreen = std::make_shared<KeyBindsScreen>(this);
  metricsscreen = std::make_shared<MetricsScreen>(this);
  transferpairingscreen = std::make_shared<TransferPairingScreen>(this);
  externalscriptsscreen = std::make_shared<ExternalScriptsScreen>(this);
  mainwindows.push_back(mainscreen);
  mainwindows.push_back(newkeyscreen);
  mainwindows.push_back(confirmationscreen);
  mainwindows.push_back(editsitescreen);
  mainwindows.push_back(sitestatusscreen);
  mainwindows.push_back(rawdatascreen);
  mainwindows.push_back(rawcommandscreen);
  mainwindows.push_back(browsescreen);
  mainwindows.push_back(newracescreen);
  mainwindows.push_back(racestatusscreen);
  mainwindows.push_back(globaloptionsscreen);
  mainwindows.push_back(skiplistscreen);
  mainwindows.push_back(changekeyscreen);
  mainwindows.push_back(eventlogscreen);
  mainwindows.push_back(proxyoptionsscreen);
  mainwindows.push_back(editproxyscreen);
  mainwindows.push_back(viewfilescreen);
  mainwindows.push_back(nukescreen);
  mainwindows.push_back(fileviewersettingsscreen);
  mainwindows.push_back(scoreboardscreen);
  mainwindows.push_back(selectsitesscreen);
  mainwindows.push_back(transfersscreen);
  mainwindows.push_back(transferjobstatusscreen);
  mainwindows.push_back(allracesscreen);
  mainwindows.push_back(alltransferjobsscreen);
  mainwindows.push_back(transferstatusscreen);
  mainwindows.push_back(transfersfilterscreen);
  mainwindows.push_back(infoscreen);
  mainwindows.push_back(selectjobsscreen);
  mainwindows.push_back(makedirscreen);
  mainwindows.push_back(sectionsscreen);
  mainwindows.push_back(editsectionscreen);
  mainwindows.push_back(sitesectionsscreen);
  mainwindows.push_back(editsitesectionscreen);
  mainwindows.push_back(siteslotsscreen);
  mainwindows.push_back(snakescreen);
  mainwindows.push_back(disableencryptionscreen);
  mainwindows.push_back(movescreen);
  mainwindows.push_back(fileinfoscreen);
  mainwindows.push_back(keybindsscreen);
  mainwindows.push_back(metricsscreen);
  mainwindows.push_back(transferpairingscreen);
  mainwindows.push_back(externalscriptsscreen);

  legendprinterkeybinds = std::make_shared<LegendPrinterKeybinds>(this);
  legendwindow->setMainLegendPrinter(legendprinterkeybinds);

  if (global->getSettingsLoaderSaver()->getState() == DataFileState::EXISTS_ENCRYPTED) {
    loginscreen->initialize(mainrow, col);
    topwindow = loginscreen;
  }
  else {
    mainscreen->initialize(mainrow, col);
    topwindow = mainscreen;
    enableInfo();
    enableLegend();
  }
  infowindow->setLabel(topwindow->getWideInfoLabel());
  std::cin.putback('#'); // needed to be able to peek properly
  vv.render();
  renderer.refresh(infoenabled, legendenabled);
  global->getIOManager()->registerExternalFD(this, STDIN_FILENO);
  global->getTickPoke()->startPoke(this, "UI", 50, 0);
  return true;
}

void Ui::backendPush() {
  haspushed = true;
  if (!pushused) {
    topwindow->update();
    vv.render();
    renderer.refresh(infoenabled, legendenabled);
    pushused = true;
  }
}

void Ui::signal(int signal, int) {
  if (signal == SIGWINCH) {
    terminalSizeChanged();
  }
  else {
    assert(false);
  }
}

void Ui::terminalSizeChanged() {
  renderer.resize(infoenabled, legendenabled);
  mainrow = renderer.getRow();
  if (infoenabled) {
    mainrow -= 2;
  }
  if (legendenabled) {
    mainrow -= 2;
  }
  col = renderer.getCol();
  vv.resize(mainrow, col);
  redrawAll();
  renderer.refresh(infoenabled, legendenabled);
}

LegendMode Ui::getLegendMode() const {
  return legendmode;
}

void Ui::setLegendMode(LegendMode newmode) {
  LegendMode oldmode = legendmode;
  legendmode = newmode;
  if (newmode != oldmode) {
    if (oldmode == LEGEND_DISABLED) {
      enableLegend();
    }
    else if (newmode == LEGEND_DISABLED) {
      disableLegend();
    }
    else {
      redrawAll();
    }
  }
}

bool Ui::getHighlightEntireLine() const {
  return highlightentireline;
}

void Ui::setHighlightEntireLine(bool highlight) {
  highlightentireline = highlight;
}

bool Ui::getShowXmasTree() const {
  return showxmastree;
}

void Ui::setShowXmasTree(bool show) {
  showxmastree = show;
}

void Ui::kill() {
  renderer.kill();
}

void Ui::FDData(int sockid) {
  unsigned int wch = renderer.read();
  if (!topwindow->keyPressedBase(wch)) {
    globalKeyBinds(wch);
  }
}

void Ui::enableInfo() {
  if (!infoenabled) {
    infoenabled = true;
    mainrow -= 2;
    vv.resize(mainrow, col);
    vv.reset();
    redrawAll();
    renderer.adjust(infoenabled, legendenabled);
  }
}

void Ui::disableInfo() {
  if (infoenabled) {
    infoenabled = false;
    mainrow += 2;
    vv.resize(mainrow, col);
    vv.reset();
    redrawAll();
    renderer.adjust(infoenabled, legendenabled);
  }
}

void Ui::enableLegend() {
  if (!legendenabled && getLegendMode() != LEGEND_DISABLED) {
    legendenabled = true;
    mainrow -= 2;
    vv.resize(mainrow, col);
    vv.reset();
    redrawAll();
    renderer.adjust(infoenabled, legendenabled);
  }
}

void Ui::disableLegend() {
  if (legendenabled) {
    legendenabled = false;
    mainrow += 2;
    vv.resize(mainrow, col);
    vv.reset();
    redrawAll();
    renderer.adjust(infoenabled, legendenabled);
  }
}

void Ui::redrawAll() {
  std::vector<std::shared_ptr<UIWindow> >::iterator it;
  for (it = mainwindows.begin(); it != mainwindows.end(); it++) {
    (*it)->resize(mainrow, col);
  }
  if (getLegendMode() != LEGEND_DISABLED) {
    legendwindow->resize(2, col);
    legendwindow->redraw();
  }
  if (infoenabled) {
    infowindow->resize(2, col);
    infowindow->redraw();
  }
  if (!!topwindow) {
    topwindow->resize(mainrow, col);
    topwindow->redraw();
  }
  vv.render();
}

void Ui::tick(int message) {
  if (!(ticker++ % 5)) {
    bool refresh = false;
    if (topwindow->autoUpdate()) {
      topwindow->update();
      vv.render();
      refresh = true;
    }
    if (legendenabled) {
      refresh = true;
      legendwindow->update();
    }
    if (infoenabled) {
      refresh = true;
      infowindow->setText(topwindow->getWideInfoText());
    }
    if (refresh) {
      renderer.refresh(infoenabled, legendenabled);
    }
  }
  pushused = false;
  if (haspushed) {
    backendPush();
    haspushed = false;
  }
}

void Ui::switchToWindow(const std::shared_ptr<UIWindow>& window, bool allowsplit, bool doredraw) {
  history.push_back(topwindow);
  if (split && !allowsplit) {
    setSplit(false);
  }
  setLegendText(window->getLegendText());
  infowindow->setLabel(window->getWideInfoLabel());
  infowindow->setText(window->getWideInfoText());
  topwindow = window;

  if (doredraw) {
    vv.reset();
    topwindow->redraw();
  }
  vv.render();
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::globalKeyBinds(int ch) {
  bool match = true;
  int action = globalkeybinds->getKeyAction(ch);
  switch(action) {
    case KEYACTION_KILL_ALL:
      global->getSubProcessManager()->killAll();
      break;
    case KEYACTION_START_PREPARED:
      global->getEngine()->startLatestPreparedRace();
      break;
    case KEYACTION_NEXT_PREPARED_STARTER:
      global->getEngine()->toggleStartNextPreparedRace();
      break;
    case KEYACTION_TOGGLE_UDP:
    {
      bool enabled = global->getRemoteCommandHandler()->isEnabled();
      global->getRemoteCommandHandler()->setEnabled(!enabled);
      break;
    }
    case KEYACTION_FULLSCREEN:
      if (fullscreentoggle) {
        enableInfo();
        enableLegend();
        fullscreentoggle = false;
      }
      else {
        disableInfo();
        disableLegend();
        fullscreentoggle = true;
      }
      break;
    default:
      match = false;
      break;
  }
  if (match) {
    update();
  }
}

void Ui::returnToLast() {
  switchToLast();
  topwindow->redraw();
  infowindow->setText(topwindow->getWideInfoText());
  vv.render();
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::update() {
  topwindow->update();
  vv.render();
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::setLegend() {
  setLegendText(topwindow->getLegendText());
  infowindow->setText(topwindow->getWideInfoText());
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::addTempLegendTransferJob(unsigned int id) {
  std::shared_ptr<LegendPrinterTransferJob> lptj = std::make_shared<LegendPrinterTransferJob>(this, id);
  legendwindow->addTempLegendPrinter(lptj);
  legendwindow->redraw();
}

void Ui::addTempLegendSpreadJob(unsigned int id) {
  std::shared_ptr<LegendPrinterSpreadJob> lpsj = std::make_shared<LegendPrinterSpreadJob>(this, id);
  legendwindow->addTempLegendPrinter(lpsj);
  legendwindow->redraw();
}

void Ui::setInfo() {
  if (!topwindow) {
    return;
  }
  infowindow->setLabel(topwindow->getWideInfoLabel());
  infowindow->setText(topwindow->getWideInfoText());
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::setSplit(bool split) {
  if (this->split != split) {
    this->split = split;
    infowindow->setSplit(split);
    infowindow->redraw();
    legendwindow->setSplit(split);
    legendwindow->redraw();
  }
}

void Ui::redraw() {
  topwindow->redraw();
  vv.render();
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::hideCursor() {
  renderer.hideCursor();
}

void Ui::showCursor() {
  renderer.showCursor();
}

void Ui::highlight(bool highlight) {
  renderer.highlight(highlight);
}

void Ui::goRawCommand(const std::string & site) {
  rawcommandscreen->initialize(mainrow, col, site);
  switchToWindow(rawcommandscreen);
}

void Ui::goRawCommand(const std::string & site, const Path & path, const std::string & arg) {
  rawcommandscreen->initialize(mainrow, col, site, path, arg);
  switchToWindow(rawcommandscreen);
}

void Ui::goInfo(const std::string & message) {
  confirmationscreen->initialize(mainrow, col, message, ConfirmationMode::INFO);
  switchToWindow(confirmationscreen);
}

void Ui::goConfirmation(const std::string & message) {
  confirmationscreen->initialize(mainrow, col, message, ConfirmationMode::NORMAL);
  switchToWindow(confirmationscreen);
}

void Ui::goStrongConfirmation(const std::string & message) {
  confirmationscreen->initialize(mainrow, col, message, ConfirmationMode::STRONG);
  switchToWindow(confirmationscreen);
}

void Ui::goNuke(const std::string & site, const std::string & items, const Path & path) {
  nukescreen->initialize(mainrow, col, site, items, path);
  switchToWindow(nukescreen);
}

void Ui::goViewFile(const std::string & site, const std::string & file, const std::shared_ptr<FileList>& fl) {
  viewfilescreen->initialize(mainrow, col, site, file, fl);
  switchToWindow(viewfilescreen);
}

void Ui::goViewFile(const Path & dir, const std::string & file) {
  viewfilescreen->initialize(mainrow, col, dir, file);
  switchToWindow(viewfilescreen);
}

void Ui::goNewRace(const std::string & site, const std::list<std::string> & sections, const std::list<std::pair<std::string, bool> > & items) {
  newracescreen->initialize(mainrow, col, site, sections, items);
  switchToWindow(newracescreen);
}

void Ui::goSelectSites(const std::string & message, std::list<std::shared_ptr<Site> > currentsitelist, std::list<std::shared_ptr<Site> > excludedsitelist) {
  selectsitesscreen->initializeExclude(mainrow, col, message, currentsitelist, excludedsitelist);
  switchToWindow(selectsitesscreen);
}

void Ui::goSelectSitesFrom(const std::string & message, std::list<std::shared_ptr<Site> > currentsitelist, std::list<std::shared_ptr<Site> > sitelist) {
  selectsitesscreen->initializeSelect(mainrow, col, message, currentsitelist, sitelist);
  switchToWindow(selectsitesscreen);
}

void Ui::goSelectSpreadJobs() {
  selectjobsscreen->initialize(mainrow, col, JOBTYPE_SPREADJOB);
  switchToWindow(selectjobsscreen);
}

void Ui::goSelectTransferJobs() {
  selectjobsscreen->initialize(mainrow, col, JOBTYPE_TRANSFERJOB);
  switchToWindow(selectjobsscreen);
}

void Ui::goSkiplist(SkipList * skiplist) {
  skiplistscreen->initialize(mainrow, col, skiplist);
  switchToWindow(skiplistscreen);
}

void Ui::goChangeKey() {
  changekeyscreen->initialize(mainrow, col);
  switchToWindow(changekeyscreen);
}

void Ui::goProxy() {
  proxyoptionsscreen->initialize(mainrow, col);
  switchToWindow(proxyoptionsscreen);
}

void Ui::goFileViewerSettings() {
  fileviewersettingsscreen->initialize(mainrow, col);
  switchToWindow(fileviewersettingsscreen);
}

void Ui::goSiteStatus(const std::string & site) {
  sitestatusscreen->initialize(mainrow, col, site);
  switchToWindow(sitestatusscreen);
}

void Ui::goRaceStatus(unsigned int id) {
  if (!racestatusscreen->initialize(mainrow, col, id)) {
    return;
  }
  switchToWindow(racestatusscreen);
}

void Ui::goTransferJobStatus(unsigned int id) {
  if (!transferjobstatusscreen->initialize(mainrow, col, id)) {
    return;
  }
  switchToWindow(transferjobstatusscreen);
}

void Ui::goTransferStatus(std::shared_ptr<TransferStatus> ts) {
  transferstatusscreen->initialize(mainrow, col, ts);
  switchToWindow(transferstatusscreen);
}

void Ui::goGlobalOptions() {
  globaloptionsscreen->initialize(mainrow, col);
  switchToWindow(globaloptionsscreen);
}

void Ui::goEventLog() {
  eventlogscreen->initialize(mainrow, col);
  switchToWindow(eventlogscreen);
}

void Ui::goScoreBoard() {
  scoreboardscreen->initialize(mainrow, col);
  switchToWindow(scoreboardscreen);
}

void Ui::goTransfers() {
  transfersscreen->initialize(mainrow, col);
  switchToWindow(transfersscreen);
}

void Ui::goTransfersFilterSite(const std::string& site) {
  transfersscreen->initializeFilterSite(mainrow, col, site);
  switchToWindow(transfersscreen);
}

void Ui::goTransfersFilterSpreadJob(const std::string& job) {
  transfersscreen->initializeFilterSpreadJob(mainrow, col, job);
  switchToWindow(transfersscreen);
}

void Ui::goTransfersFilterTransferJob(const std::string& job) {
  transfersscreen->initializeFilterTransferJob(mainrow, col, job);
  switchToWindow(transfersscreen);
}

void Ui::goTransfersFilterSpreadJobSite(const std::string& job, const std::string& site) {
  transfersscreen->initializeFilterSpreadJobSite(mainrow, col, job, site);
  switchToWindow(transfersscreen);
}

void Ui::returnTransferFilters(const TransferFilteringParameters& tfp) {
  transfersscreen->initialize(mainrow, col, tfp);
  switchToLast();
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::goTransfersFiltering(const TransferFilteringParameters& tfp) {
  transfersfilterscreen->initialize(mainrow, col, tfp);
  switchToWindow(transfersfilterscreen);
}

void Ui::goEditSite(const std::string& site) {
  editsitescreen->initialize(mainrow, col, "edit", site);
  switchToWindow(editsitescreen);
}

void Ui::goAddSite() {
  editsitescreen->initialize(mainrow, col, "add", "");
  switchToWindow(editsitescreen);
}

void Ui::goBrowse(const std::string& site, const Path& path) {
  browsescreen->initializeSite(mainrow, col, site, path);
  switchToWindow(browsescreen);
}

void Ui::goBrowseSection(const std::string& site, const std::string& section) {
  browsescreen->initializeSite(mainrow, col, site, section);
  switchToWindow(browsescreen);
}

void Ui::goBrowseSplit(const std::string& site) {
  browsescreen->initializeSplit(mainrow, col, site);
  switchToWindow(browsescreen, true);
}

void Ui::goBrowseSplit(const std::string& site, const Path& sitepath, const Path& localpath) {
  browsescreen->initializeSiteLocal(mainrow, col, site, sitepath, localpath);
  switchToWindow(browsescreen, true);
}

void Ui::goBrowseSplit(const std::string& leftsite, const std::string& rightsite, const Path& leftpath, const Path& rightpath) {
  browsescreen->initializeSiteSite(mainrow, col, leftsite, rightsite, leftpath, rightpath);
  switchToWindow(browsescreen, true);
}

void Ui::goBrowseLocal() {
  browsescreen->initializeLocal(mainrow, col);
  switchToWindow(browsescreen);
}

void Ui::goContinueBrowsing() {
  if (browsescreen->isInitialized()) {
    switchToWindow(browsescreen, true, true);
  }
}

void Ui::goAddProxy() {
  editproxyscreen->initialize(mainrow, col, "add", "");
  switchToWindow(editproxyscreen);
}

void Ui::goEditProxy(const std::string& proxy) {
  editproxyscreen->initialize(mainrow, col, "edit", proxy);
  switchToWindow(editproxyscreen);
}

void Ui::goRawData(const std::string& site) {
  rawdatascreen->initialize(mainrow, col, site, 0);
  switchToWindow(rawdatascreen);
}

void Ui::goRawDataJump(const std::string& site, int id) {
  rawdatascreen->initialize(mainrow, col, site, id);
  topwindow = rawdatascreen;
  infowindow->setLabel(topwindow->getWideInfoLabel());
  infowindow->setText(topwindow->getWideInfoText());
  vv.render();
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::goRawBuffer(RawBuffer * rawbuf, const std::string & label, const std::string & infotext) {
  rawcommandscreen->initialize(mainrow, col, rawbuf, label, infotext);
  switchToWindow(rawcommandscreen);
}

void Ui::goAllRaces() {
  allracesscreen->initialize(mainrow, col);
  switchToWindow(allracesscreen);
}

void Ui::goAllTransferJobs() {
  alltransferjobsscreen->initialize(mainrow, col);
  switchToWindow(alltransferjobsscreen);
}

void Ui::goInfo() {
  infoscreen->initialize(mainrow, col);
  switchToWindow(infoscreen);
}

void Ui::goMakeDir(const std::string & site, UIFileList & filelist) {
  makedirscreen->initialize(mainrow, col, site, filelist);
  switchToWindow(makedirscreen);
}

void Ui::goSiteSlots(const std::shared_ptr<Site> & site) {
  siteslotsscreen->initialize(mainrow, col, site);
  switchToWindow(siteslotsscreen);
}

void Ui::goSections() {
  sectionsscreen->initialize(mainrow, col);
  switchToWindow(sectionsscreen);
}

void Ui::goSelectSection(const std::list<std::string>& preselected) {
  sectionsscreen->initialize(mainrow, col, true, preselected);
  switchToWindow(sectionsscreen);
}

void Ui::goSiteSections(const std::shared_ptr<Site> & site) {
  sitesectionsscreen->initialize(mainrow, col, site);
  switchToWindow(sitesectionsscreen);
}

void Ui::goEditSection(const std::string & name) {
  editsectionscreen->initialize(mainrow, col, name);
  switchToWindow(editsectionscreen);
}

void Ui::goAddSection() {
  editsectionscreen->initialize(mainrow, col);
  switchToWindow(editsectionscreen);
}

void Ui::goAddSiteSection(const std::shared_ptr<Site> & site, const Path & path) {
  editsitesectionscreen->initialize(mainrow, col, site, path);
  switchToWindow(editsitesectionscreen);
}

void Ui::goEditSiteSection(const std::shared_ptr<Site> & site, const std::string & section) {
  editsitesectionscreen->initialize(mainrow, col, site, section);
  switchToWindow(editsitesectionscreen);
}

void Ui::goSnake() {
  snakescreen->initialize(mainrow, col);
  switchToWindow(snakescreen);
}

void Ui::goEnableEncryption() {
  newkeyscreen->initialize(mainrow, col);
  switchToWindow(newkeyscreen);
}

void Ui::goDisableEncryption() {
  disableencryptionscreen->initialize(mainrow, col);
  switchToWindow(disableencryptionscreen);
}

void Ui::goMove(const std::string& items, const Path& srcpath, const std::string& dstpath, const std::string& firstitem, const std::string& site) {
  movescreen->initialize(mainrow, col, site, items, srcpath, dstpath, firstitem);
  switchToWindow(movescreen);
}

void Ui::goFileInfo(UIFile* uifile) {
  fileinfoscreen->initialize(mainrow, col, uifile);
  switchToWindow(fileinfoscreen);
}

void Ui::goKeyBinds(KeyBinds* keybinds) {
  keybindsscreen->initialize(mainrow, col, keybinds);
  switchToWindow(keybindsscreen);
}

void Ui::goGlobalKeyBinds() {
  keybindsscreen->initialize(mainrow, col, globalkeybinds.get());
  switchToWindow(keybindsscreen);
}

void Ui::goMetrics() {
  metricsscreen->initialize(mainrow, col);
  switchToWindow(metricsscreen);
}

void Ui::goTransferPairing(TransferPairing* transferpairing) {
  transferpairingscreen->initialize(mainrow, col, transferpairing);
  switchToWindow(transferpairingscreen);
}

void Ui::goExternalScripts(ExternalScripts* externalscripts, const std::string& description) {
  externalscriptsscreen->initialize(mainrow, col, externalscripts, description);
  switchToWindow(externalscriptsscreen);
}

void Ui::returnSelectItems(const std::string & items) {
  switchToLast();
  topwindow->command("returnselectitems", items);
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::key(const std::string & key) {
  bool result = global->getSettingsLoaderSaver()->tryDecrypt(key);
  if (result) {
    mainscreen->initialize(mainrow, col);
    switchToWindow(mainscreen);
    enableInfo();
    enableLegend();
  }
  else {
    topwindow->update();
    vv.render();
    renderer.refresh(infoenabled, legendenabled);
  }
}

void Ui::newKey(const std::string & newkey) {
  assert(global->getSettingsLoaderSaver()->setEncrypted(newkey));
  returnToLast();
}

void Ui::confirmYes() {
  switchToLast();
  topwindow->command("yes");
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::confirmNo() {
  switchToLast();
  topwindow->command("no");
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::returnNuke(int multiplier, const std::string & reason) {
  switchToLast();
  topwindow->command("nuke", std::to_string(multiplier) + ";" + reason);
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::returnRaceStatus(unsigned int id) {
  history.clear();
  topwindow = mainscreen;
  racestatusscreen->initialize(mainrow, col, id);
  switchToWindow(racestatusscreen);
}

void Ui::returnMakeDir(const std::string & dirname) {
  switchToLast();
  topwindow->command("makedir", dirname);
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::returnMove(const std::string& dstpath) {
  switchToLast();
  topwindow->command("move", dstpath);
  renderer.refresh(infoenabled, legendenabled);
}

void Ui::setLegendText(const std::string& legendtext) {
  legendprinterkeybinds->setText(legendtext);
  legendwindow->clearTempLegendPrinters();
  legendwindow->update();
}
void Ui::switchToLast() {
  vv.reset();
  if (split) {
    setSplit(false);
  }
  topwindow = history.back();
  history.pop_back();
  setLegendText(topwindow->getLegendText());
  infowindow->setLabel(topwindow->getWideInfoLabel());
  infowindow->setText(topwindow->getWideInfoText());
}

void Ui::loadSettings(std::shared_ptr<DataFileHandler> dfh) {
  std::vector<std::string> lines;
  dfh->getDataFor("UI", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("legendmode")) {
      setLegendMode((LegendMode) std::stoi(value));
    }
    else if (!setting.compare("highlightentireline")) {
      setHighlightEntireLine(!value.compare("true"));
    }
    else if (!setting.compare("keybind")) {
      std::vector<std::string> tokens = util::splitVec(value, "$");
      int keyaction = std::stoi(tokens[1]);
      int scope = std::stoi(tokens[2]);
      std::list<std::string> keys = util::split(tokens[3], ",");
      std::set<unsigned int> keyset;
      for (const std::string& key : keys) {
        if (!key.empty()) {
          keyset.insert(std::stoi(key));
        }
      }
      for (KeyBinds* keybinds : allkeybinds) {
        if (keybinds->getName() == tokens[0]) {
          keybinds->replaceBind(keyaction, scope, keyset);
          break;
        }
      }
    }
  }
  dfh->getDataFor("ExternalFileViewing", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("video")) {
      efv.setVideoViewer(value);
    }
    else if (!setting.compare("audio")) {
      efv.setAudioViewer(value);
    }
    else if (!setting.compare("image")) {
      efv.setImageViewer(value);
    }
    else if (!setting.compare("pdf")) {
      efv.setPDFViewer(value);
    }
  }
}

void Ui::saveSettings(std::shared_ptr<DataFileHandler> dfh) {
  dfh->addOutputLine("UI", "legendmode=" + std::to_string(getLegendMode()));
  dfh->addOutputLine("UI", "highlightentireline=" + std::string(getHighlightEntireLine() ? "true" : "false"));
  for (const KeyBinds* keybinds : allkeybinds) {
    for (std::list<KeyBinds::KeyData>::const_iterator it = keybinds->begin(); it != keybinds->end(); ++it) {
      if (it->configuredkeys != it->originalkeys) {
        std::string keys;
        for (unsigned int key : it->configuredkeys) {
          keys += (keys.empty() ? "" : ",") + std::to_string(key);
        }
        dfh->addOutputLine("UI", "keybind=" + keybinds->getName() + "$" + std::to_string(it->keyaction) + "$" + std::to_string(it->scope) + "$" + keys);
      }
    }
  }
  std::string filetag = "ExternalFileViewing";
  dfh->addOutputLine(filetag, "video=" + efv.getVideoViewer());
  dfh->addOutputLine(filetag, "audio=" + efv.getAudioViewer());
  dfh->addOutputLine(filetag, "image=" + efv.getImageViewer());
  dfh->addOutputLine(filetag, "pdf=" + efv.getPDFViewer());
}

void Ui::notify() {
  renderer.bell();
}

void Ui::requestReady(void* service, int requestid) {
  backendPush();
}

void Ui::intermediateData(void* service, int requestid, void* data) {
  backendPush();
}

void Ui::addKeyBinds(KeyBinds* keybinds) {
  allkeybinds.insert(keybinds);
}

void Ui::removeKeyBinds(KeyBinds* keybinds) {
  allkeybinds.erase(keybinds);
}

bool Ui::isTop(const UIWindow* window) const {
  return window == topwindow.get();
}

Renderer& Ui::getRenderer() {
  return renderer;
}
VirtualView& Ui::getVirtualView() {
  return vv;
}

ExternalFileViewing& Ui::getExternalFileViewing() {
  return efv;
}
