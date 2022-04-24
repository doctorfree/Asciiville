#pragma once

#include <list>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "../core/eventreceiver.h"
#include "../uibase.h"
#include "../settingsloadersaver.h"
#include "../path.h"
#include "../requestcallback.h"

#include "externalfileviewing.h"
#include "renderer.h"
#include "virtualview.h"

class UIWindow;
class InfoWindow;
class LegendWindow;
class FileList;
class UIFileList;
class UIFile;
class Site;
class DataFileHandler;
class TransferStatus;
class Path;
class SkipList;
class RawBuffer;
class KeyBinds;
class TransferPairing;
class ExternalScripts;

class LoginScreen;
class NewKeyScreen;
class MainScreen;
class ConfirmationScreen;
class EditSiteScreen;
class SiteStatusScreen;
class RawDataScreen;
class RawCommandScreen;
class BrowseScreen;
class NewRaceScreen;
class RaceStatusScreen;
class GlobalOptionsScreen;
class SkipListScreen;
class ChangeKeyScreen;
class EventLogScreen;
class ProxyOptionsScreen;
class EditProxyScreen;
class ViewFileScreen;
class NukeScreen;
class FileViewerSettingsScreen;
class ScoreBoardScreen;
class SelectSitesScreen;
class TransfersScreen;
class TransferJobStatusScreen;
class AllRacesScreen;
class AllTransferJobsScreen;
class TransferStatusScreen;
class TransfersFilterScreen;
class InfoScreen;
class SelectJobsScreen;
class MakeDirScreen;
class SectionsScreen;
class EditSectionScreen;
class SiteSectionsScreen;
class EditSiteSectionScreen;
class SiteSlotsScreen;
class SnakeScreen;
class DisableEncryptionScreen;
class MoveScreen;
class FileInfoScreen;
class KeyBindsScreen;
class MetricsScreen;
class TransferPairingScreen;
class ExternalScriptsScreen;

class LegendPrinterKeybinds;
struct TransferFilteringParameters;

enum LegendMode {
  LEGEND_DISABLED = 123,
  LEGEND_SCROLLING = 124,
  LEGEND_STATIC = 125
};

class Ui : public Core::EventReceiver, public UIBase, public SettingsAdder, public RequestCallback {
private:
  Renderer renderer;
  VirtualView vv;
  ExternalFileViewing efv;
  std::vector<std::shared_ptr<UIWindow> > mainwindows;
  std::shared_ptr<UIWindow> topwindow;
  std::shared_ptr<InfoWindow> infowindow;
  std::shared_ptr<LegendWindow> legendwindow;
  std::shared_ptr<LoginScreen> loginscreen;
  std::shared_ptr<NewKeyScreen> newkeyscreen;
  std::shared_ptr<MainScreen> mainscreen;
  std::shared_ptr<ConfirmationScreen> confirmationscreen;
  std::shared_ptr<EditSiteScreen> editsitescreen;
  std::shared_ptr<SiteStatusScreen> sitestatusscreen;
  std::shared_ptr<RawDataScreen> rawdatascreen;
  std::shared_ptr<RawCommandScreen> rawcommandscreen;
  std::shared_ptr<BrowseScreen> browsescreen;
  std::shared_ptr<NewRaceScreen> newracescreen;
  std::shared_ptr<RaceStatusScreen> racestatusscreen;
  std::shared_ptr<GlobalOptionsScreen> globaloptionsscreen;
  std::shared_ptr<SkipListScreen> skiplistscreen;
  std::shared_ptr<ChangeKeyScreen> changekeyscreen;
  std::shared_ptr<EventLogScreen> eventlogscreen;
  std::shared_ptr<ProxyOptionsScreen> proxyoptionsscreen;
  std::shared_ptr<EditProxyScreen> editproxyscreen;
  std::shared_ptr<ViewFileScreen> viewfilescreen;
  std::shared_ptr<NukeScreen> nukescreen;
  std::shared_ptr<FileViewerSettingsScreen> fileviewersettingsscreen;
  std::shared_ptr<ScoreBoardScreen> scoreboardscreen;
  std::shared_ptr<SelectSitesScreen> selectsitesscreen;
  std::shared_ptr<TransfersScreen> transfersscreen;
  std::shared_ptr<TransferJobStatusScreen> transferjobstatusscreen;
  std::shared_ptr<AllRacesScreen> allracesscreen;
  std::shared_ptr<AllTransferJobsScreen> alltransferjobsscreen;
  std::shared_ptr<TransferStatusScreen> transferstatusscreen;
  std::shared_ptr<TransfersFilterScreen> transfersfilterscreen;
  std::shared_ptr<InfoScreen> infoscreen;
  std::shared_ptr<SelectJobsScreen> selectjobsscreen;
  std::shared_ptr<MakeDirScreen> makedirscreen;
  std::shared_ptr<SectionsScreen> sectionsscreen;
  std::shared_ptr<EditSectionScreen> editsectionscreen;
  std::shared_ptr<SiteSectionsScreen> sitesectionsscreen;
  std::shared_ptr<EditSiteSectionScreen> editsitesectionscreen;
  std::shared_ptr<SiteSlotsScreen> siteslotsscreen;
  std::shared_ptr<SnakeScreen> snakescreen;
  std::shared_ptr<DisableEncryptionScreen> disableencryptionscreen;
  std::shared_ptr<MoveScreen> movescreen;
  std::shared_ptr<FileInfoScreen> fileinfoscreen;
  std::shared_ptr<KeyBindsScreen> keybindsscreen;
  std::shared_ptr<MetricsScreen> metricsscreen;
  std::shared_ptr<TransferPairingScreen> transferpairingscreen;
  std::shared_ptr<ExternalScriptsScreen> externalscriptsscreen;
  std::shared_ptr<LegendPrinterKeybinds> legendprinterkeybinds;
  unsigned int mainrow;
  unsigned int col;
  int ticker;
  bool haspushed;
  bool pushused;
  bool legendenabled;
  bool infoenabled;
  LegendMode legendmode;
  bool highlightentireline;
  bool showxmastree;
  bool split;
  bool fullscreentoggle;
  std::string eventtext;
  std::list<std::shared_ptr<UIWindow> > history;
  std::shared_ptr<KeyBinds> globalkeybinds;
  std::set<KeyBinds*> allkeybinds;
  void enableInfo();
  void disableInfo();
  void enableLegend();
  void disableLegend();
  void redrawAll();
  void switchToWindow(const std::shared_ptr<UIWindow>& window, bool allowsplit = false, bool doredraw = false);
  void globalKeyBinds(int);
  void switchToLast();
  void setLegendText(const std::string& legendtext);
  void FDData(int sockid) override;
  void tick(int message) override;
  void requestReady(void* service, int requestid) override;
  void intermediateData(void* service, int requestid, void* data) override;
public:
  Ui();
  ~Ui();
  bool init() override;
  void backendPush() override;
  void terminalSizeChanged();
  void signal(int signal, int) override;
  void kill() override;
  Renderer& getRenderer();
  VirtualView& getVirtualView();
  ExternalFileViewing& getExternalFileViewing();
  void resizeTerm();
  void readConfiguration();
  void writeState();
  LegendMode getLegendMode() const;
  void setLegendMode(LegendMode);
  bool getHighlightEntireLine() const;
  void setHighlightEntireLine(bool highlight);
  bool getShowXmasTree() const;
  void setShowXmasTree(bool show);
  void returnToLast();
  void update();
  void setLegend();
  void addTempLegendTransferJob(unsigned int id);
  void addTempLegendSpreadJob(unsigned int id);
  void setInfo();
  void setSplit(bool split);
  void redraw();
  void showCursor();
  void hideCursor();
  void highlight(bool highlight);
  void goRawCommand(const std::string & site);
  void goRawCommand(const std::string & site, const Path & path, const std::string & arg = "");
  void goInfo(const std::string & message);
  void goConfirmation(const std::string & message);
  void goStrongConfirmation(const std::string & message);
  void goNuke(const std::string & site, const std::string & items, const Path & path);
  void goViewFile(const std::string &, const std::string &, const std::shared_ptr<FileList>& fl);
  void goViewFile(const Path &, const std::string &);
  void goNewRace(const std::string & site, const std::list<std::string> & sections, const std::list<std::pair<std::string, bool> > & items);
  void goSelectSites(const std::string &, std::list<std::shared_ptr<Site> >, std::list<std::shared_ptr<Site> >);
  void goSelectSitesFrom(const std::string &, std::list<std::shared_ptr<Site> >, std::list<std::shared_ptr<Site> >);
  void goSelectSpreadJobs();
  void goSelectTransferJobs();
  void goSkiplist(SkipList * skiplist = nullptr);
  void goChangeKey();
  void goProxy();
  void goFileViewerSettings();
  void goSiteStatus(const std::string &);
  void goRaceStatus(unsigned int);
  void goTransferJobStatus(unsigned int);
  void goTransferStatus(std::shared_ptr<TransferStatus>);
  void goGlobalOptions();
  void goEventLog();
  void goScoreBoard();
  void goTransfers();
  void goTransfersFilterSite(const std::string &);
  void goTransfersFilterSpreadJob(const std::string &);
  void goTransfersFilterTransferJob(const std::string &);
  void goTransfersFilterSpreadJobSite(const std::string & job, const std::string & site);
  void returnTransferFilters(const TransferFilteringParameters &);
  void goTransfersFiltering(const TransferFilteringParameters &);
  void goEditSite(const std::string &);
  void goAddSite();
  void goBrowse(const std::string& site, const Path& path = Path());
  void goBrowseSection(const std::string& site, const std::string& section);
  void goBrowseSplit(const std::string& site);
  void goBrowseSplit(const std::string& site, const Path& sitepath, const Path& localpath);
  void goBrowseSplit(const std::string& leftsite, const std::string& rightsite, const Path& leftpath, const Path& rightpath);
  void goBrowseLocal();
  void goContinueBrowsing();
  void goAddProxy();
  void goEditProxy(const std::string& proxy);
  void goRawData(const std::string& site);
  void goRawDataJump(const std::string& site, int id);
  void goRawBuffer(RawBuffer * rawbuffer, const std::string & label, const std::string & infotext);
  void goAllRaces();
  void goAllTransferJobs();
  void goInfo();
  void goMakeDir(const std::string & site, UIFileList & filelist);
  void goSiteSlots(const std::shared_ptr<Site> & site);
  void goSections();
  void goSelectSection(const std::list<std::string>& preselected = std::list<std::string>());
  void goSiteSections(const std::shared_ptr<Site> & site);
  void goAddSection();
  void goEditSection(const std::string & name);
  void goAddSiteSection(const std::shared_ptr<Site> & site, const Path & path = "");
  void goEditSiteSection(const std::shared_ptr<Site> & site, const std::string & section);
  void goSnake();
  void goEnableEncryption();
  void goDisableEncryption();
  void goMove(const std::string& items, const Path& srcpath, const std::string& dstpath, const std::string& firstitem, const std::string& site = "");
  void goFileInfo(UIFile* uifile);
  void goKeyBinds(KeyBinds* keybinds);
  void goGlobalKeyBinds();
  void goMetrics();
  void goTransferPairing(TransferPairing* transferpairing);
  void goExternalScripts(ExternalScripts* externalscripts, const std::string& description);
  void returnSelectItems(const std::string &);
  void key(const std::string &);
  void newKey(const std::string &);
  void confirmYes();
  void confirmNo();
  void returnNuke(int multiplier, const std::string& reason);
  void returnRaceStatus(unsigned int);
  void returnMakeDir(const std::string& dirname);
  void returnMove(const std::string& dstpath);
  void loadSettings(std::shared_ptr<DataFileHandler>) override;
  void saveSettings(std::shared_ptr<DataFileHandler>) override;
  void notify() override;
  WINDOW * getLegendWindow() const;
  void addKeyBinds(KeyBinds* keybinds);
  void removeKeyBinds(KeyBinds* keybinds);
  bool isTop(const UIWindow* window) const;
};

