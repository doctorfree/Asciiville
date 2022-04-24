#include "sitelogic.h"

#include <cassert>

#include "core/tickpoke.h"
#include "core/eventreceiver.h"
#include "core/types.h"
#include "sitemanager.h"
#include "ftpconn.h"
#include "file.h"
#include "filelist.h"
#include "siterace.h"
#include "site.h"
#include "sitelogicmanager.h"
#include "globalcontext.h"
#include "scoreboardelement.h"
#include "potentialtracker.h"
#include "sitelogicrequest.h"
#include "sitelogicrequestready.h"
#include "uibase.h"
#include "rawbuffer.h"
#include "engine.h"
#include "connstatetracker.h"
#include "transfermonitor.h"
#include "delayedcommand.h"
#include "potentiallistelement.h"
#include "eventlog.h"
#include "recursivecommandlogic.h"
#include "transfermanager.h"
#include "localstorage.h"
#include "transferjob.h"
#include "commandowner.h"
#include "race.h"
#include "sitetransferjob.h"
#include "transferstatus.h"
#include "filelistdata.h"
#include "requestcallback.h"
#include "downloadfiledata.h"

// minimum sleep delay (between refreshes / hammer attempts) in ms
#define SLEEP_DELAY 150

// maximum number of dir refreshes in a row in the same race, if there are
// other races to switch to
#define MAX_CHECKS_ROW_VERY_LOW 1
#define MAX_CHECKS_ROW_PRIO_LOW 2
#define MAX_CHECKS_ROW_PRIO_NORMAL 3
#define MAX_CHECKS_ROW_PRIO_HIGH 4
#define MAX_CHECKS_ROW_PRIO_VERY_HIGH 5

// maximum number of ready requests available to be checked out, unless below
#define MAX_REQUEST_READY_QUEUE_SIZE 10

// minimum time that requests are guaranteed to live in the ready queue, in ms
#define MAX_REQUEST_READY_QUEUE_IDLE_TIME 10000

// ticker heartbeat, in ms
#define TICK_INTERVAL 50

// time that a filelist is considered newly updated and not in need of refresh,
// in ms
#define FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH 2000

// time that a filelist is considered newly updated and not in need of refresh
// when xdupe is enabled, in ms
#define FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH_XDUPE 5000

// time that a transfer slot stays reserved after disconnecting a connection
// with an ongoing transfer, in ms
#define TRANSFER_SLOT_CLEANUP_DELAY 2000

// time to wait before trying to revisit a directory that does not exist and
// previously could not be mkdired. time in ms
#define DELAY_BEFORE_REVISIT_FAILED_DIR 3000

// time to wait before trying to revisit a directory that does not exist
#define DELAY_BEFORE_REVISIT_NONEXISTING_DIR 1000

namespace {

enum RequestType {
  REQ_FILELIST,
  REQ_RAW,
  REQ_WIPE_RECURSIVE,
  REQ_WIPE,
  REQ_DEL_RECURSIVE,
  REQ_DEL_OWN,
  REQ_DEL,
  REQ_NUKE,
  REQ_IDLE,
  REQ_MKDIR,
  REQ_MOVE,
  REQ_DOWNLOAD_FILE_STAGE1,
  REQ_DOWNLOAD_FILE_STAGE2
};

int maxChecksRow(SitePriority priority) {
  switch (priority) {
    case SitePriority::VERY_LOW:
      return MAX_CHECKS_ROW_VERY_LOW;
    case SitePriority::LOW:
      return MAX_CHECKS_ROW_PRIO_LOW;
    case SitePriority::NORMAL:
      return MAX_CHECKS_ROW_PRIO_NORMAL;
    case SitePriority::HIGH:
      return MAX_CHECKS_ROW_PRIO_HIGH;
    case SitePriority::VERY_HIGH:
    default:
      return MAX_CHECKS_ROW_PRIO_VERY_HIGH;
  }
  return MAX_CHECKS_ROW_PRIO_VERY_HIGH;
}

class HandlingConnGuard {
public:
  HandlingConnGuard(ConnStateTracker& cst) : cst(cst) {
    cst.setHandlingConnection(true);
  }
  ~HandlingConnGuard() {
    cst.setHandlingConnection(false);
  }
private:
  ConnStateTracker& cst;
};

TransferType getTransferType(bool isdownload, const std::shared_ptr<CommandOwner>& co) {
  TransferType type(TransferType::REGULAR);
  if (isdownload) {
    if (!!co) {
      if (co->classType() == COMMANDOWNER_SITERACE) {
        if (std::static_pointer_cast<SiteRace>(co)->isDownloadOnly()) {
          type = TransferType::PRE;
        }
        else if (std::static_pointer_cast<SiteRace>(co)->isDone()) {
          type = TransferType::COMPLETE;
        }
      }
      else if (co->classType() == COMMANDOWNER_TRANSFERJOB) {
        type = TransferType::TRANSFERJOB;
      }
    }
  }
  return type;
}

void cleanupFailedRequest(const std::shared_ptr<SiteLogicRequest> & request) {
  if (request->getType() == REQ_DOWNLOAD_FILE_STAGE2) {
    delete static_cast<DownloadFileData*>(request->getPtrData());
  }
}

} // namespace

enum class Exists {
  NO,
  YES,
  FAIL
};

SiteLogic::SiteLogic(const std::string & sitename) :
  site(global->getSiteManager()->getSite(sitename)),
  rawcommandrawbuf(new RawBuffer(site->getName())),
  aggregatedrawbuf(new RawBuffer(site->getName())),
  slotsdn(0),
  slotsup(0),
  available(0),
  ptrack(new PotentialTracker(site->getMaxDown())),
  loggedin(0),
  requestidcounter(0),
  poke(false),
  currtime(0),
  timesincelastrequestready(0),
  refreshgovernor(site)
{
  global->getTickPoke()->startPoke(this, "SiteLogic", TICK_INTERVAL, 0);

  for (unsigned int i = 0; i < site->getMaxLogins(); i++) {
    connstatetracker.push_back(ConnStateTracker());
    conns.push_back(new FTPConn(this, i));
  }
}

SiteLogic::~SiteLogic() {
  global->getTickPoke()->stopPoke(this, 0);
  delete ptrack;
  for (unsigned int i = 0; i < conns.size(); i++) {
    delete conns[i];
  }
  delete rawcommandrawbuf;
  delete aggregatedrawbuf;
}

void SiteLogic::activateAll() {
  for (unsigned int i = 0; i < conns.size(); i++) {
    if (!conns[i]->isConnected()) {
      connstatetracker[i].resetIdleTime();
      conns[i]->login();
    }
    else {
      handleConnection(i);
    }
  }
}

void SiteLogic::disconnectAll(bool hard) {
  for (unsigned int i = 0; i < conns.size(); i++) {
    disconnectConn(i, hard);
  }
}

std::shared_ptr<SiteRace> SiteLogic::addRace(const std::shared_ptr<Race>& enginerace, const std::string & section, const std::string & release, bool downloadonly) {
  std::shared_ptr<SiteRace> race = std::make_shared<SiteRace>(enginerace, site->getName(), site->getSectionPath(section),
                                                              release, site->getUser(), site->getSkipList(), downloadonly);
  currentraces.push_back(race);
  activateAll();
  return race;
}

void SiteLogic::resetRace(const std::shared_ptr<SiteRace> & race) {
  bool current = false;
  for (const std::shared_ptr<SiteRace> & currentrace : currentraces) {
    if (currentrace == race) {
      current = true;
      break;
    }
  }
  if (!current) {
    currentraces.push_back(race);
  }
  activateAll();
}

void SiteLogic::addTransferJob(std::shared_ptr<SiteTransferJob> & tj) {
  transferjobs.push_back(tj);
  haveConnectedActivate(tj->getTransferJob().lock()->maxSlots());
}

void SiteLogic::tick(int message) {
  currtime += TICK_INTERVAL;
  for (std::list<DelayedCommand>::iterator it = delayedcommands.begin(); it != delayedcommands.end(); ++it) {
    DelayedCommand & delayedcommand = *it;
    if (delayedcommand.isActive()) {
      delayedcommand.currentTime(currtime);
      if (delayedcommand.isReleased()) {
        std::string event = delayedcommand.getCommand();
        if (event == "returnuploadslot") {
          --slotsup;
        }
        else if (event == "returndownloadslot") {
          --slotsdn;
        }
        it = delayedcommands.erase(it);
      }
    }
  }
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    connstatetracker[i].timePassed(TICK_INTERVAL);
    if (connstatetracker[i].getCommand().isReleased()) {
      assert(!connstatetracker[i].isLocked() && !conns[i]->isProcessing());
      DelayedCommand eventcommand = connstatetracker[i].getCommand();
      connstatetracker[i].getCommand().reset();
      connstatetracker[i].use();
      std::string event = eventcommand.getCommand();
      if (event == "userpass") {
        conns[i]->doUSER(false);
      }
      else if (event == "reconnect") {
        conns[i]->reconnect();
      }
      else if (event == "quit") {
        disconnectConn(i);
      }
    }
  }
  if (currtime % 60000 == 0) {
    site->tickMinute();
  }
  timesincelastrequestready += TICK_INTERVAL;
  clearExpiredReadyRequests();
  std::vector<unsigned int> idlingconns;
  for (unsigned int i = 0; i < conns.size(); i++) {
    if(connstatetracker[i].isLoggedIn() && !connstatetracker[i].isLocked() &&
        !conns[i]->isProcessing())
    {
      idlingconns.push_back(i);
    }
  }
  refreshgovernor.timePassed(TICK_INTERVAL);
  if (refreshgovernor.refreshAllowed() && !currentraces.empty() && !idlingconns.empty()) {
    unsigned int winindex = rand() % idlingconns.size();
    unsigned int winner = idlingconns[winindex];
    handleSpreadJobs(winner, false);
    if(connstatetracker[winner].isLocked() || conns[winner]->isProcessing()) {
      idlingconns.erase(idlingconns.begin() + winindex);
    }
  }
  for (unsigned int i : idlingconns) {
    if (connstatetracker[i].getTimePassed() > static_cast<int>(site->getMaxIdleTime() * 1000)) {
      if (site->getStayLoggedIn()) {
        antiAntiIdle(i);
      }
      else {
        disconnectConn(i);
      }
    }
  }
}

void SiteLogic::connectFailed(int id) {
  disconnectConn(id);
}

void SiteLogic::userDenied(int id) {
  disconnectConn(id);
}

void SiteLogic::userDeniedSiteFull(int id) {
  connstatetracker[id].delayedCommand("reconnect", SLEEP_DELAY, true);
}

void SiteLogic::userDeniedSimultaneousLogins(int id) {
  conns[id]->doUSER(true);
}

void SiteLogic::loginKillFailed(int id) {
  disconnectConn(id);
}

void SiteLogic::passDenied(int id) {
  disconnectConn(id);
}

void SiteLogic::TLSFailed(int id) {
  disconnectConn(id);
}

void SiteLogic::listRefreshed(int id) {
  connstatetracker[id].resetIdleTime();
  global->getStatistics()->addFileListRefresh();
  const std::shared_ptr<FileList>& fl = conns[id]->currentFileList();
  const std::shared_ptr<CommandOwner>& currentco = conns[id]->currentCommandOwner();
  if (!!currentco) {
    currentco->fileListUpdated(this, fl);
  }
  if (connstatetracker[id].getRecursiveLogic()->isActive()) {
    handleRecursiveLogic(id, conns[id]->currentFileList());
    return;
  }
  if (connstatetracker[id].hasRequest()) {
    const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
    if (request->getType() == REQ_FILELIST &&
        request->getData() == fl->getPath().toString())
    {
      FileListData * filelistdata = new FileListData(fl, conns[id]->getCwdRawBuffer());
      setRequestReady(id, filelistdata, true);
    }
    else if (request->getType() == REQ_DOWNLOAD_FILE_STAGE1 &&
             request->getData() == fl->getPath().toString())
    {
      File* f = fl->getFile(request->getData2());
      if (!f || f->isDirectory() || f->getSize() > 524288) {
        setRequestReady(id, nullptr, false);
      }
      else {
        connstatetracker[id].finishRequest();
        available++;
        DownloadFileData* dlfdata = new DownloadFileData();
        dlfdata->fl = fl;
        dlfdata->file = request->getData2();
        dlfdata->inmemory = request->getNumData();
        requests.emplace_back(request->getCallback(), request->getId(), REQ_DOWNLOAD_FILE_STAGE2, dlfdata);
      }
    }
  }
  if (!!currentco) {
    global->getEngine()->jobFileListRefreshed(this, currentco, fl);
    global->getEngine()->raceActionRequest();
  }
  handleConnection(id);
}

bool SiteLogic::setPathExists(int id, Exists exists, bool refreshtime) {
  const std::shared_ptr<FileList>& fl = conns[id]->currentFileList();
  if (!fl) {
    return false;
  }
  bool statechanged = false;
  FileListState state = fl->getState();
  if (state == FileListState::UNKNOWN) {
    if (exists == Exists::YES) {
      fl->setExists();
    }
    else if (exists == Exists::FAIL) {
      fl->setPreFailed();
    }
    else if (exists == Exists::NO) {
      fl->setNonExistent();
    }
    statechanged = true;
  }
  else if (state == FileListState::NONEXISTENT) {
    if (exists == Exists::YES) {
      fl->setExists();
      statechanged = true;
    }
    else if (exists == Exists::FAIL) {
      fl->setPreFailed();
      statechanged = true;
    }
  }
  else if (state == FileListState::PRE_FAIL) {
    if (exists == Exists::YES) {
      fl->setExists();
      statechanged = true;
    }
    else if (exists == Exists::NO) {
      fl->setFailed();
      statechanged = true;
    }
  }
  else if (state == FileListState::FAILED) {
    if (exists == Exists::YES) {
      fl->setExists();
      statechanged = true;
    }
  }
  else if (state == FileListState::LISTED || state == FileListState::EXISTS) {
    if (exists == Exists::NO) {
      if (fl->addListFailure()) {
        statechanged = true;
      }
    }
  }
  std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
  if (exists != Exists::YES) {
    if (fl->bumpUpdateState(UpdateState::REFRESHED)) {
      statechanged = true;
    }
  }
  if (refreshtime) {
    fl->setRefreshed();
  }
  if (currentco && statechanged) {
    currentco->fileListUpdated(this, fl);
  }
  return statechanged;
}

bool SiteLogic::handleCommandDelete(int id, bool success) {
  if (connstatetracker[id].hasRequest()) {
    int type = connstatetracker[id].getRequest()->getType();
    if (type == REQ_DEL) {
      setRequestReady(id, NULL, success);
    }
    else if (type == REQ_DEL_RECURSIVE || type == REQ_DEL_OWN) {
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        handleRecursiveLogic(id);
        return true;
      }
      else {
        setRequestReady(id, NULL, success);
      }
    }
  }
  return false;
}

bool SiteLogic::makeTargetDirectory(int id, bool includinglast, const std::shared_ptr<CommandOwner> & co) {
  Path trypath = conns[id]->getMKDCWDTargetSection();
  std::list<std::string> subdirs = conns[id]->getMKDSubdirs();
  while (co && subdirs.size() > (includinglast ? 0 : 1)) {
    trypath = trypath / subdirs.front();
    subdirs.pop_front();
    std::shared_ptr<FileList> fl = co->getFileListForFullPath(this, trypath);
    if (fl) {
      if (fl->getState() == FileListState::UNKNOWN || fl->getState() == FileListState::NONEXISTENT) {
        conns[id]->doMKD(fl, co);
        return true;
      }
    }
    else {
      conns[id]->doMKD(trypath, co);
      return true;
    }
  }
  return false;
}

void SiteLogic::commandSuccess(int id, FTPConnState state) {
  connstatetracker[id].resetIdleTime();
  std::list<SiteLogicRequest>::iterator it;
  if (!refreshgovernor.refreshAllowed() && refreshgovernor.immediateRefreshAllowed()) {
    if (state != FTPConnState::CWD && state != FTPConnState::LIST && state != FTPConnState::LIST_COMPLETE &&
        state != FTPConnState::PRET_LIST)
    {
      connstatetracker[id].setRefreshToken();
      refreshgovernor.useRefresh();
    }
  }
  switch (state) {
    case FTPConnState::LOGIN:
      loggedin++;
      available++;
      connstatetracker[id].setLoggedIn();
      break;
    case FTPConnState::PBSZ:
    case FTPConnState::PROT_P:
    case FTPConnState::PROT_C:
    case FTPConnState::SSCN_ON:
    case FTPConnState::SSCN_OFF:
    case FTPConnState::CEPR_ON:
      break;
    case FTPConnState::PORT:
    case FTPConnState::EPRT:
      if (connstatetracker[id].transferInitialized()) {
        connstatetracker[id].getTransferMonitor()->activeReady();
        return;
      }
      break;
    case FTPConnState::CWD: {
      setPathExists(id, Exists::YES, false);
      if (conns[id]->hasMKDCWDTarget()) {
        if (conns[id]->getCurrentPath() == conns[id]->getMKDCWDTargetSection() / conns[id]->getMKDCWDTargetPath()) {
          conns[id]->finishMKDCWDTarget();
        }
      }
      if (connstatetracker[id].hasRequest()) {
        const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
        if (request->getType() == REQ_FILELIST ||
            request->getType() == REQ_DOWNLOAD_FILE_STAGE1)
        {
          getFileListConn(id);
          return;
        }
        else if (request->getType() == REQ_RAW) {
          rawcommandrawbuf->writeLine(request->getData2());
          conns[id]->doRaw(request->getData2());
          return;
        }
        else if (request->getType() == REQ_IDLE) {
          setRequestReady(id, NULL, true);
          handleConnection(id);
          if (!conns[id]->isProcessing()) {
            if (!site->getStayLoggedIn()) {
              connstatetracker[id].delayedCommand("quit", request->getNumData() * 1000);
            }
          }
          return;
        }
        else if (request->getType() == REQ_MKDIR) {
          conns[id]->doMKD(request->getData2());
          return;
        }
        else if (request->getType() == REQ_WIPE) {
          conns[id]->doWipe(request->getData(), false);
          return;
        }
        else if (request->getType() == REQ_WIPE_RECURSIVE) {
          conns[id]->doWipe(request->getData(), true);
          return;
        }
        else if (request->getType() == REQ_NUKE) {
          conns[id]->doNuke(request->getData(), request->getNumData(), request->getData2());
          return;
        }
      }
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        handleRecursiveLogic(id);
        return;
      }
      break;
    }
    case FTPConnState::MKD:
      if (connstatetracker[id].hasRequest()) {
        const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
        if (request->getType() == REQ_MKDIR) {
          setRequestReady(id, NULL, true);
        }
      }
      if (conns[id]->hasMKDCWDTarget()) {
        std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
        const Path & targetcwdsect = conns[id]->getMKDCWDTargetSection();
        const Path & targetcwdpath = conns[id]->getMKDCWDTargetPath();
        const Path & targetpath = conns[id]->getTargetPath();
        setPathExists(id, Exists::YES, true);
        if (targetpath == targetcwdsect / targetcwdpath) {
          conns[id]->finishMKDCWDTarget();
          const std::shared_ptr<FileList>& fl = conns[id]->currentFileList();
          if (fl) {
            conns[id]->doCWD(fl, currentco);
          }
          else {
            conns[id]->doCWD(targetpath, currentco);
          }
          return;
        }
        if (makeTargetDirectory(id, true, currentco)) {
          return;
        }
      }
      break;
    case FTPConnState::PRET_RETR:
    case FTPConnState::PRET_STOR:
      if (connstatetracker[id].transferInitialized()) {
        assert(connstatetracker[id].getTransferPassive());
        passiveModeCommand(id);
        return;
      }
      break;
    case FTPConnState::RETR: // RETR started
      if (connstatetracker[id].transferInitialized()) {
        if (!connstatetracker[id].getTransferPassive()) {
          connstatetracker[id].getTransferMonitor()->activeStarted();
        }
      }
      return;
    case FTPConnState::RETR_COMPLETE:
      assert(connstatetracker[id].transferInitialized());
      connstatetracker[id].getTransferMonitor()->sourceComplete();
      transferComplete(id, true);
      connstatetracker[id].finishTransfer();
      handlePostDownload(id);
      return;
    case FTPConnState::STOR: // STOR started
      if (connstatetracker[id].transferInitialized()) {
        if (!connstatetracker[id].getTransferPassive()) {
          connstatetracker[id].getTransferMonitor()->activeStarted();
        }
      }
      return;
    case FTPConnState::STOR_COMPLETE: {
      assert(connstatetracker[id].transferInitialized());
      connstatetracker[id].getTransferMonitor()->targetComplete();
      transferComplete(id, false);
      const std::shared_ptr<FileList>& fl = connstatetracker[id].getTransferFileList();
      connstatetracker[id].finishTransfer();
      handlePostUpload(id, fl);
      return;
    }
    case FTPConnState::ABOR:
      break;
    case FTPConnState::PASV_ABORT:
      break;
    case FTPConnState::WIPE:
      if (connstatetracker[id].hasRequest()) {
        const std::shared_ptr<SiteLogicRequest> & request = connstatetracker[id].getRequest();
        if (request->getType() == REQ_WIPE_RECURSIVE ||
            request->getType() == REQ_WIPE)
        {
          setRequestReady(id, nullptr, true);
        }
      }
      break;
    case FTPConnState::DELE:
    case FTPConnState::RMD:
      if (handleCommandDelete(id, true)) {
        return;
      }
      break;
    case FTPConnState::NUKE:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->getType() == REQ_NUKE) {
          setRequestReady(id, nullptr, true);
        }
      }
      break;
    case FTPConnState::RNFR:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->getType() == REQ_MOVE) {
          std::string dstpath = connstatetracker[id].getRequest()->getData2();
          conns[id]->doRNTO(dstpath);
        }
      }
      break;
    case FTPConnState::RNTO:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->getType() == REQ_MOVE) {
          setRequestReady(id, nullptr, true);
        }
      }
      break;
    case FTPConnState::LIST:
      if (!connstatetracker[id].getTransferPassive()) {
        connstatetracker[id].getTransferMonitor()->activeStarted();
      }
      return;
    case FTPConnState::PRET_LIST:
      if (connstatetracker[id].transferInitialized()) {
        if (connstatetracker[id].getTransferPassive()) {
          passiveModeCommand(id);
          return;
        }
      }
      break;
    case FTPConnState::LIST_COMPLETE:
      if (connstatetracker[id].transferInitialized()) {
        TransferMonitor * tm = connstatetracker[id].getTransferMonitor();
        connstatetracker[id].finishTransfer();
        tm->sourceComplete();
      }
      handleConnection(id);
      return;
    case FTPConnState::QUIT:
      connstatetracker[id].setDisconnected();
      return;
    default:
      assert(false);
      return;
  }
  handleConnection(id);
}

void SiteLogic::commandFail(int id) {
  commandFail(id, FailureType::UNDEFINED);
}

void SiteLogic::commandFail(int id, FailureType failuretype) {
  connstatetracker[id].resetIdleTime();
  FTPConnState state = conns[id]->getState();
  if (!refreshgovernor.refreshAllowed() && refreshgovernor.immediateRefreshAllowed()) {
    if (state != FTPConnState::CWD && state != FTPConnState::LIST && state != FTPConnState::LIST_COMPLETE &&
        state != FTPConnState::PRET_LIST)
    {
      connstatetracker[id].setRefreshToken();
      refreshgovernor.useRefresh();
    }
  }
  switch (state) {
    case FTPConnState::PBSZ:
    case FTPConnState::PROT_P:
    case FTPConnState::PROT_C:
    case FTPConnState::CPSV:
    case FTPConnState::PASV:
    case FTPConnState::PORT:
    case FTPConnState::CEPR_ON:
    case FTPConnState::EPSV:
    case FTPConnState::EPRT:
      if (connstatetracker[id].transferInitialized()) {
        handleTransferFail(id, TM_ERR_OTHER);
        return;
      }
      else {
        handleConnection(id);
        return;
      }
      break;
    case FTPConnState::CWD: {
      bool filelistupdated = false;
      if (setPathExists(id, Exists::NO, true)) {
        filelistupdated = true;
      }
      std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
      if (currentco && currentco->classType() == COMMANDOWNER_SITERACE && connstatetracker[id].lastChecked() == currentco) {
        connstatetracker[id].check();
      }
      if (connstatetracker[id].getRecursiveLogic()->isActive()) {
        connstatetracker[id].getRecursiveLogic()->failedCwd();
        if (connstatetracker[id].getRecursiveLogic()->isActive()) {
          handleRecursiveLogic(id);
        }
        else {
          handleConnection(id);
        }
        return;
      }
      std::shared_ptr<FileList> currentfl = conns[id]->currentFileList();
      if (conns[id]->hasMKDCWDTarget()) {
        if (site->getAllowUpload() == SITE_ALLOW_TRANSFER_NO ||
            (!!currentco && currentco->classType() == COMMANDOWNER_SITERACE &&
             std::static_pointer_cast<SiteRace>(currentco)->isDownloadOnly()))
        {
          conns[id]->finishMKDCWDTarget();
        }
        else {
          if (currentfl) {
            conns[id]->doMKD(currentfl, currentco);
          }
          else {
            conns[id]->doMKD(conns[id]->getTargetPath(), currentco);
          }
          return;
        }
      }
      if (connstatetracker[id].hasRequest()) {
        const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
        if (request->getType() == REQ_FILELIST ||
            request->getType() == REQ_RAW ||
            request->getType() == REQ_IDLE ||
            request->getType() == REQ_MKDIR ||
            request->getType() == REQ_WIPE ||
            request->getType() == REQ_WIPE_RECURSIVE ||
            request->getType() == REQ_NUKE ||
            request->getType() == REQ_DOWNLOAD_FILE_STAGE1 ||
            request->getType() == REQ_DOWNLOAD_FILE_STAGE2)
        {
          setRequestReady(id, NULL, false);
          handleConnection(id);
          if (request->getType() == REQ_IDLE && !conns[id]->isProcessing()) {
            if (site->getStayLoggedIn()) {
              connstatetracker[id].delayedCommand("antiantiidle", site->getMaxIdleTime() * 1000);
            }
            else {
              connstatetracker[id].delayedCommand("quit", request->getNumData() * 1000);
            }
          }
          return;
        }
      }
      if (filelistupdated) {
        if (currentco && currentfl) {
          global->getEngine()->jobFileListRefreshed(this, currentco, currentfl);
          global->getEngine()->raceActionRequest();
        }
        handleConnection(id);
        return;
      }
      if (connstatetracker[id].hasTransfer() && !connstatetracker[id].transferInitialized()) {
        handleConnection(id);
        return;
      }
      handleFail(id);
      return;
    }
    case FTPConnState::MKD:
      if (conns[id]->hasMKDCWDTarget()) {
        std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
        const Path & targetcwdsect = conns[id]->getMKDCWDTargetSection();
        const Path & targetcwdpath = conns[id]->getMKDCWDTargetPath();
        const Path & targetpath = conns[id]->getTargetPath();
        if (targetpath == targetcwdsect / targetcwdpath && makeTargetDirectory(id, false, currentco)) {
          return;
        }
        conns[id]->finishMKDCWDTarget();
        setPathExists(id, Exists::FAIL, true);
        const std::shared_ptr<FileList>& fl = conns[id]->currentFileList();
        if (fl) {
          conns[id]->doCWD(fl, currentco);
        }
        else {
          conns[id]->doCWD(targetpath, currentco);
        }
        return;
      }
      else {
        if (connstatetracker[id].hasRequest()) {
          const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
          if (request->getType() == REQ_MKDIR) {
            setRequestReady(id, NULL, false);
            handleConnection(id);
            return;
          }
        }
        std::shared_ptr<CommandOwner> currentco = conns[id]->currentCommandOwner();
        if (!!currentco && currentco->classType() == COMMANDOWNER_TRANSFERJOB) {
          handleConnection(id);
          return;
        }
      }
      break;
    case FTPConnState::PRET_RETR:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_PRET);
      return;
    case FTPConnState::PRET_STOR:
      if (failuretype == FailureType::DUPE) {
        handleTransferFail(id, CST_UPLOAD, TM_ERR_DUPE);
      }
      else {
        handleTransferFail(id, CST_UPLOAD, TM_ERR_PRET);
      }
      return;
    case FTPConnState::RETR:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_RETRSTOR);
      return;
    case FTPConnState::RETR_COMPLETE:
      handleTransferFail(id, CST_DOWNLOAD, TM_ERR_RETRSTOR_COMPLETE);
      return;
    case FTPConnState::STOR:
      if (failuretype == FailureType::DUPE) {
        handleTransferFail(id, CST_UPLOAD, TM_ERR_DUPE);
      }
      else {
        handleTransferFail(id, CST_UPLOAD, TM_ERR_RETRSTOR);
      }
      return;
    case FTPConnState::STOR_COMPLETE:
      handleTransferFail(id, CST_UPLOAD, TM_ERR_RETRSTOR_COMPLETE);
      return;
    case FTPConnState::WIPE:
      if (connstatetracker[id].hasRequest()) {
        const std::shared_ptr<SiteLogicRequest> & request = connstatetracker[id].getRequest();
        if (request->getType() == REQ_WIPE_RECURSIVE ||
            request->getType() == REQ_WIPE)
        {
          setRequestReady(id, nullptr, false);
        }
      }
      handleConnection(id);
      return;
    case FTPConnState::DELE:
    case FTPConnState::RMD:
      if (!handleCommandDelete(id, false)) {
        handleConnection(id);
      }
      return;
    case FTPConnState::NUKE:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->getType() == REQ_NUKE) {
          setRequestReady(id, nullptr, false);
        }
      }
      handleConnection(id);
      return;
    case FTPConnState::RNFR:
    case FTPConnState::RNTO:
      if (connstatetracker[id].hasRequest()) {
        if (connstatetracker[id].getRequest()->getType() == REQ_MOVE) {
          setRequestReady(id, nullptr, false);
        }
      }
      handleConnection(id);
      return;
    case FTPConnState::LIST:
      checkFailListRequest(id);
      handleTransferFail(id, CST_LIST, TM_ERR_RETRSTOR);
      return;
    case FTPConnState::PRET_LIST:
      checkFailListRequest(id);
      handleTransferFail(id, CST_LIST, TM_ERR_PRET);
      return;
    case FTPConnState::LIST_COMPLETE:
      checkFailListRequest(id);
      handleTransferFail(id, CST_LIST, TM_ERR_RETRSTOR_COMPLETE);
      return;
    default:
      disconnected(id);
      conns[id]->reconnect();
      return;
  }
}

void SiteLogic::checkFailListRequest(int id) {
  if (connstatetracker[id].hasRequest()) {
    if (connstatetracker[id].getRequest()->getType() == REQ_FILELIST) {
      setRequestReady(id, NULL, false);
    }
  }
}

void SiteLogic::handleFail(int id) {
  if (connstatetracker[id].isListOrTransferLocked()) {
    handleTransferFail(id, TM_ERR_OTHER);
    return;
  }
  if (connstatetracker[id].isLocked()) {
    handleConnection(id);
    return;
  }
}

void SiteLogic::handleTransferFail(int id, int err) {
  assert(connstatetracker[id].isListOrTransferLocked());
  handleTransferFail(id, connstatetracker[id].getTransferType(), err);
}

void SiteLogic::handleTransferFail(int id, int type, int err) {
  assert(connstatetracker[id].isListOrTransferLocked());
  if (connstatetracker[id].hasTransfer() && connstatetracker[id].transferInitialized() &&
      connstatetracker[id].getTransferPassive() &&
      !conns[id]->isProcessing() && (err == TM_ERR_RETRSTOR || err == TM_ERR_DUPE))
  {
    conns[id]->abortTransferPASV();
  }
  std::shared_ptr<FileList> fl = connstatetracker[id].getTransferFileList();
  reportTransferErrorAndFinish(id, type, err);
  if (type == CST_UPLOAD) {
    assert(fl);
    handlePostUpload(id, fl);
  }
  else {
    handlePostDownload(id);
  }
}

void SiteLogic::reportTransferErrorAndFinish(int id, int err) {
  reportTransferErrorAndFinish(id, connstatetracker[id].getTransferType(), err);
}

void SiteLogic::reportTransferErrorAndFinish(int id, int type, int err) {
  std::shared_ptr<CommandOwner> co = connstatetracker[id].getCommandOwner();
  std::shared_ptr<FileList> fl = connstatetracker[id].getTransferFileList();
  if (!connstatetracker[id].getTransferAborted()) {
    switch (type) {
      case CST_DOWNLOAD:
        connstatetracker[id].getTransferMonitor()->sourceError((TransferError)err);
        transferComplete(id, true, err != TM_ERR_CLEANUP);
        if (err == TM_ERR_CLEANUP) {
          delayedcommands.emplace_back();
          delayedcommands.back().set("returndownloadslot", currtime + TRANSFER_SLOT_CLEANUP_DELAY);
        }
        break;
      case CST_LIST:
        connstatetracker[id].getTransferMonitor()->sourceError((TransferError)err);
        break;
      case CST_UPLOAD:
        connstatetracker[id].getTransferMonitor()->targetError((TransferError)err);
        transferComplete(id, false, err != TM_ERR_CLEANUP);
        if (err == TM_ERR_CLEANUP) {
          delayedcommands.emplace_back();
          delayedcommands.back().set("returnuploadslot", currtime + TRANSFER_SLOT_CLEANUP_DELAY);
        }
        break;
    }
  }
  connstatetracker[id].finishTransfer();
  if (connstatetracker[id].hasRequest()) {
    const std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
    if (request->getType() == REQ_DOWNLOAD_FILE_STAGE1 || request->getType() == REQ_DOWNLOAD_FILE_STAGE2) {
      setRequestReady(id, nullptr, false);
    }
  }
  if (type == CST_UPLOAD && err == TM_ERR_DUPE && site->useXDUPE() && co && fl) {
    const std::list<std::string> & xdupelist = conns[id]->getXDUPEList();
    for (std::list<std::string>::const_iterator it = xdupelist.begin(); it != xdupelist.end(); it++) {
      fl->touchFile(*it, "XDUPE");
    }
    global->getEngine()->jobFileListRefreshed(this, co, fl);
  }
}

void SiteLogic::gotPath(int id, const std::string & path) {
  connstatetracker[id].resetIdleTime();
}

void SiteLogic::rawCommandResultRetrieved(int id, const std::string & result) {
  connstatetracker[id].resetIdleTime();
  rawcommandrawbuf->write(result);
  if (connstatetracker[id].hasRequest()) {
    if (connstatetracker[id].getRequest()->getType() == REQ_RAW) {
      std::string * data = new std::string(result);
      setRequestReady(id, data, true);
    }
  }
  handleConnection(id);
}

void SiteLogic::gotPassiveAddress(int id, const std::string & host, int port) {
  connstatetracker[id].resetIdleTime();
  if (connstatetracker[id].transferInitialized()) {
    connstatetracker[id].getTransferMonitor()->passiveReady(host, port);
  }
  else {
    handleConnection(id);
  }
}

void SiteLogic::timedout(int id) {

}

void SiteLogic::disconnected(int id) {
  cleanupConnection(id);
  if (connstatetracker[id].isLoggedIn() && !connstatetracker[id].isQuitting()) {
    loggedin--;
    available--;
  }
  connstatetracker[id].setDisconnected();
}

void SiteLogic::activateOne() {
  for (unsigned int i = 0; i < conns.size(); i++) {
    if (connstatetracker[i].isLoggedIn() && !connstatetracker[i].isLocked() &&
        !conns[i]->isProcessing())
    {
      handleConnection(i);
      return;
    }
  }
  if (loggedin < conns.size()) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (!conns[i]->isConnected()) {
        connstatetracker[i].resetIdleTime();
        conns[i]->login();
        break;
      }
    }
  }
}

void SiteLogic::haveConnectedActivate(unsigned int connected) {
  if (loggedin < connected) {
    int lefttoconnect = connected;
    for (unsigned int i = 0; i < conns.size() && lefttoconnect > 0; i++) {
      if (!conns[i]->isConnected()) {
        connstatetracker[i].resetIdleTime();
        conns[i]->login();
        lefttoconnect--;
      }
    }
  }
  else {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isLoggedIn() && !connstatetracker[i].isLocked() &&
          !conns[i]->isProcessing() && !connstatetracker[i].isHandlingConnection())
      {
        handleConnection(i);
        break;
      }
    }
  }
}

bool SiteLogic::handlePreTransfer(int id) {
  if (connstatetracker[id].getTransferMonitor()->willFail()) {
    handleTransferFail(id, TM_ERR_OTHER);
    return false;
  }
  const std::shared_ptr<FileList>& fl = connstatetracker[id].getTransferFileList();
  const Path & transferpath = fl->getPath();
  std::shared_ptr<CommandOwner> co = connstatetracker[id].getCommandOwner();
  if (conns[id]->getCurrentPath() != transferpath) {
    if (connstatetracker[id].getTransferType() == CST_UPLOAD &&
        fl->getState() == FileListState::NONEXISTENT)
    {
      std::pair<Path, Path> pathparts;
      if (co && co->classType() == COMMANDOWNER_TRANSFERJOB) {
        Path sectionpath = std::static_pointer_cast<SiteTransferJob>(co)->getPath();
        pathparts = std::pair<Path, Path>(sectionpath, transferpath - sectionpath);
      }
      else {
        pathparts = site->splitPathInSectionAndSubpath(transferpath);
      }
      conns[id]->setMKDCWDTarget(pathparts.first, pathparts.second);
      return makeTargetDirectory(id, true, co);
    }
    conns[id]->doCWD(fl, co);
    return true;
  }
  return false;
}

void SiteLogic::handlePostUpload(int id, const std::shared_ptr<FileList>& fl) {
  int filelistreusetime = fl->timeSinceLastRefreshed();
  if (!requests.empty() ||
      (!site->useXDUPE() && filelistreusetime > FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH) ||
      filelistreusetime > FILELIST_MAX_REUSE_TIME_BEFORE_REFRESH_XDUPE)
  {
    handleConnection(id);
    if (conns[id]->isProcessing()) {
      return;
    }
  }
  global->getEngine()->raceActionRequest();
  handleConnection(id);
}

void SiteLogic::handlePostDownload(int id) {
  if (!requests.empty()) {
    handleConnection(id);
    if (conns[id]->isProcessing()) {
      return;
    }
  }
  global->getEngine()->raceActionRequest();
  handleConnection(id);
}

void SiteLogic::handleConnection(int id) {
  HandlingConnGuard hcg(connstatetracker[id]);
  if (conns[id]->isProcessing()) {
    return;
  }
  if (handleLockCheck(id)) {
    return;
  }
  if (handleRequest(id)) {
    return;
  }
  if (handleTransferJobs(id)) {
    return;
  }
  handleSpreadJobs(id);
}

bool SiteLogic::handleLockCheck(int id) {
  if (connstatetracker[id].isLocked()) {
    if (connstatetracker[id].hasTransfer()) {
      connstatetracker[id].resetIdleTime();
      initTransfer(id);
      return true;
    }
    if (connstatetracker[id].isTransferLocked()) {
      handlePreTransfer(id);
    }
    return true;
  }
  return false;
}

bool SiteLogic::handleSpreadJobs(int id, bool requestaction) {
  if (currentraces.size()) {
    if (handleSpreadJob(id)) {
      return true;
    }
    if (requestaction) {
      global->getEngine()->raceActionRequest();
      return true;
    }
  }
  return false;
}

bool SiteLogic::handleTransferJobs(int id) {
  std::list<std::shared_ptr<SiteTransferJob> > targetjobs;
  for (std::list<std::shared_ptr<SiteTransferJob> >::iterator it = transferjobs.begin(); it != transferjobs.end();) {
    std::shared_ptr<SiteTransferJob> & tj = *it;
    if (tj->isDone()) {
      it = transferjobs.erase(it);
      continue;
    }
    if (tj->wantsList()) {
      std::shared_ptr<FileList> fl = tj->getListTarget();
      const Path & path = fl->getPath();
      connstatetracker[id].use();
      if (path != conns[id]->getCurrentPath()) {
        conns[id]->doCWD(fl, tj);
        return true;
      }
      getFileListConn(id, tj, fl);
      return true;
    }
    std::shared_ptr<TransferJob> ptj = tj->getTransferJob().lock();
    int type = ptj->getType();
    if (((type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) &&
         ptj->getSrc()->getCurrDown() < ptj->maxSlots()) ||
        (type == TRANSFERJOB_UPLOAD && getCurrUp() < ptj->maxSlots()))
    {
      targetjobs.push_back(tj);
    }
    ++it;
  }
  for (std::list<std::shared_ptr<SiteTransferJob> >::iterator it = targetjobs.begin(); it != targetjobs.end(); it++) {
    if (global->getEngine()->transferJobActionRequest(*it)) {
      handleConnection(id);
      return true;
    }
  }
  return false;
}

bool SiteLogic::handleSpreadJob(int id) {
  std::set<std::shared_ptr<SiteRace>> triedraces;
  while (true) {
    std::shared_ptr<SiteRace> race;
    bool refresh = false;
    std::shared_ptr<SiteRace> lastchecked = connstatetracker[id].lastChecked();
    bool uselastchecked = false;
    if (lastchecked && !lastchecked->isDone() && lastchecked->anyFileListNotNonExistent() &&
        connstatetracker[id].checkCount() < maxChecksRow(site->getPriority()))
    {
      race = lastchecked;
      refresh = true;
      uselastchecked = true;
    }
    else {
      connstatetracker[id].resetLastChecked();
      for (unsigned int i = 0; i < currentraces.size(); i++) {
        if (!currentraces[i]->isDone() && !wasRecentlyListed(currentraces[i])) {
          race = currentraces[i];
          break;
        }
      }
      if (!race) {
        for (std::list<std::shared_ptr<SiteRace>>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
          if (!(*it)->isDone()) {
            race = *it;
            break;
          }
        }
      }
      if (!!race) {
        refresh = true;
        addRecentList(race);
      }
    }
    if (!race) {
      for (unsigned int i = 0; i < currentraces.size(); i++) {
        if (currentraces[i]->anyFileListNotNonExistent()) {
          race = currentraces[i];
          break;
        }
      }
    }
    if (!race) {
      if (!currentraces.empty()) {
        race = currentraces.front();
      }
      else {
        return false;
      }
    }
    if (triedraces.find(race) != triedraces.end()) {
      return false;
    }
    connstatetracker[id].setLastChecked(race);
    triedraces.insert(race);
    if (!connstatetracker[id].hasRefreshToken()) {
      if (refreshgovernor.refreshAllowed()) {
        refreshgovernor.useRefresh();
        connstatetracker[id].setRefreshToken();
      }
      else {
        refresh = false;
      }
    }
    else {
      refreshgovernor.useRefresh();
    }
    const Path & currentpath = conns[id]->getCurrentPath();
    const Path & racepath = race->getPath();
    bool goodpath = currentpath == racepath || // same path
                    currentpath.contains(racepath); // same path, currently in subdir
    std::shared_ptr<FileList> fl;
    if (goodpath) {
      Path subpath = currentpath - racepath;
      fl = race->getFileListForPath(subpath.toString());
      if (!fl && race->addSubDirectory(subpath.toString(), true)) {
        fl = race->getFileListForPath(subpath.toString());
      }
      if (refresh && fl && fl->getPath() != connstatetracker[id].getLastRefreshPath()) {
        connstatetracker[id].check();
        getFileListConn(id, race, fl);
        return true;
      }
    }
    if (!fl || fl->getPath() == connstatetracker[id].getLastRefreshPath()) {
      if (!fl || !uselastchecked) {
        std::string subpath = race->getRelevantSubPath();
        Path targetpath = race->getPath() / subpath;
        fl = race->getFileListForFullPath(this, targetpath);
        if (targetpath != currentpath) {
          int timesincelastrefresh = fl->timeSinceLastRefreshed();
          if ((fl->getState() == FileListState::FAILED && timesincelastrefresh < DELAY_BEFORE_REVISIT_FAILED_DIR) ||
              (fl->getState() == FileListState::NONEXISTENT && timesincelastrefresh < DELAY_BEFORE_REVISIT_NONEXISTING_DIR))
          {
            continue;
          }
          connstatetracker[id].use();
          conns[id]->doCWD(fl, race);
          return true;
        }
      }
      if (refresh) {
        connstatetracker[id].check();
        getFileListConn(id, race, fl);
        return true;
      }
    }
    return false;
  }
  return false;
}

bool SiteLogic::handleRequest(int id) {
  if (requests.empty()) {
    return false;
  }
  std::list<SiteLogicRequest>::iterator it;
  for (it = requests.begin(); it != requests.end(); it++) {
    if (it->getConnId() == id) {
      break;
    }
  }
  if (it == requests.end()) {
    for (it = requests.begin(); it != requests.end(); it++) {
      if (it->getConnId() == -1) {
        if (it->getType() == REQ_DOWNLOAD_FILE_STAGE2 && !downloadSlotAvailable())
        {
          continue;
        }
        it->setConnId(id);
        break;
      }
    }
  }
  if (it == requests.end()) {
    return false;
  }
  connstatetracker[id].use();
  SiteLogicRequest request = *it;
  requests.erase(it);
  connstatetracker[id].setRequest(request);
  available--;

  switch (request.getType()) {
    case REQ_DOWNLOAD_FILE_STAGE1:
    case REQ_FILELIST: { // filelist
      Path targetpath = request.getData();
      if (conns[id]->getCurrentPath() == targetpath) {
        getFileListConn(id);
      }
      else {
        conns[id]->doCWD(targetpath);
      }
      break;
    }
    case REQ_DOWNLOAD_FILE_STAGE2: {
      DownloadFileData* dlfdata = static_cast<DownloadFileData*>(request.getPtrData());
      std::shared_ptr<LocalFileList> localfl;
      if (!dlfdata->inmemory) {
        const Path temppath = global->getLocalStorage()->getTempPath();
        localfl = global->getLocalStorage()->getLocalFileList(temppath);
      }
      dlfdata->ts = global->getTransferManager()->attemptDownload(dlfdata->file, global->getSiteLogicManager()->getSiteLogic(this), id, dlfdata->fl, localfl);
      request.getCallback()->intermediateData(this, request.getId(), dlfdata);
      break;
    }
    case REQ_RAW: { // raw command
      Path targetpath = request.getData();
      if (conns[id]->getCurrentPath() != targetpath) {
        conns[id]->doCWD(targetpath);
      }
      else {
        rawcommandrawbuf->writeLine(request.getData2());
        conns[id]->doRaw(request.getData2());
      }
      break;
    }
    case REQ_WIPE_RECURSIVE: { // recursive wipe
      Path path(request.getData());
      if (path.isAbsolute() && conns[id]->getCurrentPath() != path.dirName()) {
        conns[id]->doCWD(path.dirName());
      }
      else {
        conns[id]->doWipe(request.getData(), true);
      }
      break;
    }
    case REQ_WIPE: { // wipe
      Path path(request.getData());
      if (path.isAbsolute() && conns[id]->getCurrentPath() != path.dirName()) {
        conns[id]->doCWD(path.dirName());
      }
      else {
        conns[id]->doWipe(request.getData(), false);
      }
      break;
    }
    case REQ_DEL_RECURSIVE: { // recursive delete
      std::string targetpath = request.getData();
      connstatetracker[id].getRecursiveLogic()->initialize(RCL_DELETE, targetpath, site->getUser());
      handleRecursiveLogic(id);
      break;
    }
    case REQ_DEL_OWN: { // recursive delete of own files
      std::string targetpath = request.getData();
      connstatetracker[id].getRecursiveLogic()->initialize(RCL_DELETEOWN, targetpath, site->getUser());
      handleRecursiveLogic(id);
      break;
    }
    case REQ_DEL: // delete
      conns[id]->doDELE(request.getData());
      break;
    case REQ_NUKE: { // nuke
      Path path(request.getData());
      if (path.isAbsolute() && conns[id]->getCurrentPath() != path.dirName()) {
        conns[id]->doCWD(path.dirName());
      }
      else {
        conns[id]->doNuke(request.getData(), request.getNumData(), request.getData2());
      }
      break;
    }
    case REQ_IDLE: { // idle
      Path targetpath = request.getData();
      if (conns[id]->getCurrentPath() != targetpath) {
        conns[id]->doCWD(targetpath);
      }
      else {
        setRequestReady(id, NULL, true);
        handleConnection(id);
        if (!conns[id]->isProcessing()) {
          if (site->getStayLoggedIn()) {
            connstatetracker[id].delayedCommand("antiantiidle", site->getMaxIdleTime() * 1000);
          }
          else {
            connstatetracker[id].delayedCommand("quit", request.getNumData() * 1000);
          }
        }
      }
      break;
    }
    case REQ_MKDIR: { // make directory
      Path targetpath = request.getData();
      if (conns[id]->getCurrentPath() != targetpath) {
        conns[id]->doCWD(targetpath);
      }
      else {
        conns[id]->doMKD(request.getData2());
      }
      break;
    }
    case REQ_MOVE: { // move item
      Path srcpath = request.getData();
      conns[id]->doRNFR(srcpath.toString());
      break;
    }
  }
  return true;
}

void SiteLogic::handleRecursiveLogic(int id, const std::shared_ptr<FileList>& fl) {
  Path actiontarget;
  if (fl != NULL) {
    connstatetracker[id].getRecursiveLogic()->addFileList(fl);
  }
  switch (connstatetracker[id].getRecursiveLogic()->getAction(conns[id]->getCurrentPath(), actiontarget)) {
    case RCL_ACTION_LIST:
      getFileListConn(id, true);
      break;
    case RCL_ACTION_CWD:
      conns[id]->doCWD(actiontarget);
      break;
    case RCL_ACTION_DELETE:
      conns[id]->doDELE(actiontarget);
      break;
    case RCL_ACTION_DELDIR:
      conns[id]->doRMD(actiontarget);
      break;
    case RCL_ACTION_NOOP:
      if (connstatetracker[id].hasRequest()) {
        int type = connstatetracker[id].getRequest()->getType();
        if (type == REQ_DEL || type == REQ_DEL_RECURSIVE || type == REQ_DEL_OWN) {
          setRequestReady(id, NULL, true);
        }
      }
      handleConnection(id);
      break;
  }
}

void SiteLogic::addRecentList(const std::shared_ptr<SiteRace> & sr) {
  for (std::list<std::shared_ptr<SiteRace>>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == sr) {
      recentlylistedraces.erase(it);
      break;
    }
  }
  recentlylistedraces.push_back(sr);
}

bool SiteLogic::wasRecentlyListed(const std::shared_ptr<SiteRace> & sr) const {
  for (std::list<std::shared_ptr<SiteRace>>::const_iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == sr) {
      return true;
    }
  }
  return false;
}

void SiteLogic::initTransfer(int id) {
  if (conns[id]->isProcessing()) {
    return;
  }
  if (connstatetracker[id].getTransferMonitor()->willFail()) {
    handleTransferFail(id, TM_ERR_OTHER);
    return;
  }
  int transfertype = connstatetracker[id].getTransferType();
  if (!connstatetracker[id].transferInitialized()) {
    connstatetracker[id].initializeTransfer();
  }
  bool transferssl = connstatetracker[id].getTransferSSL();
  bool transfersslclient = connstatetracker[id].getTransferSSLClient();
  bool sscnmode = conns[id]->getSSCNMode();
  bool passive = connstatetracker[id].getTransferPassive();
  Status status = connstatetracker[id].getTransferMonitor()->getStatus();
  if (transfertype != CST_LIST) {
    if (handlePreTransfer(id)) {
      return;
    }
  }
  if (transferssl && conns[id]->getProtectedMode() != ProtMode::P) {
    conns[id]->doPROTP();
    return;
  }
  if (!transferssl && conns[id]->getProtectedMode() != ProtMode::C) {
    if (site->getTLSMode() != TLSMode::NONE || conns[id]->getProtectedMode() == ProtMode::P) {
      conns[id]->doPROTC();
      return;
    }
  }
  bool ipv6 = connstatetracker[id].getTransferIPv6();
  if (ipv6 && passive && !conns[id]->getCEPREnabled()) {
    conns[id]->doCEPRON();
    return;
  }
  if (status != (passive ? TM_STATUS_AWAITING_PASSIVE : TM_STATUS_AWAITING_ACTIVE)) {
    return;
  }
  if (connstatetracker[id].getTransferFXP()) {
    if (transferssl && (!passive || ipv6 || !site->supportsCPSV())) {
      if (transfersslclient && !sscnmode) {
        conns[id]->doSSCN(true);
        return;
      }
      if (!transfersslclient && sscnmode) {
        conns[id]->doSSCN(false);
        return;
      }
    }
  }
  else if (transferssl && sscnmode) {
    conns[id]->doSSCN(false);
    return;
  }
  if (!passive) {
    if (ipv6) {
      conns[id]->doEPRT(connstatetracker[id].getTransferHost(), connstatetracker[id].getTransferPort());
    }
    else {
      conns[id]->doPORT(connstatetracker[id].getTransferHost(), connstatetracker[id].getTransferPort());
    }
  }
  else {
    if (site->needsPRET()) {
      switch (transfertype) {
        case CST_DOWNLOAD:
          conns[id]->doPRETRETR(connstatetracker[id].getTransferFile());
          break;
        case CST_UPLOAD:
          conns[id]->doPRETSTOR(connstatetracker[id].getTransferFile());
          break;
        case CST_LIST:
          conns[id]->doPRETLIST();
          break;
      }
    }
    else {
      passiveModeCommand(id);
    }
  }
}

int SiteLogic::requestFileList(RequestCallback* cb, const Path & path) {
  int requestid = requestidcounter++;
  requests.emplace_back(cb, requestid, REQ_FILELIST, path.toString());
  activateOne();
  return requestid;
}

int SiteLogic::requestDownloadFile(RequestCallback* cb, const Path& path, const std::string& file, bool inmemory) {
  int requestid = requestidcounter++;
  requests.emplace_back(cb, requestid, REQ_DOWNLOAD_FILE_STAGE1, path.toString(), file, inmemory);
  activateOne();
  return requestid;
}

int SiteLogic::requestDownloadFile(RequestCallback* cb, const std::shared_ptr<FileList>& fl, const std::string& file, bool inmemory) {
  int requestid = requestidcounter++;
  DownloadFileData* dlfdata = new DownloadFileData();
  dlfdata->fl = fl;
  dlfdata->inmemory = inmemory;
  dlfdata->file = file;
  requests.emplace_back(cb, requestid, REQ_DOWNLOAD_FILE_STAGE2, dlfdata);
  activateOne();
  return requestid;
}

int SiteLogic::requestRawCommand(RequestCallback* cb, const std::string & command) {
  return requestRawCommand(cb, site->getBasePath(), command);
}

int SiteLogic::requestRawCommand(RequestCallback* cb, const Path & path, const std::string & command) {
  int requestid = requestidcounter++;
  std::string expandedcommand = expandVariables(command);
  requests.emplace_back(cb, requestid, REQ_RAW, path.toString(), expandedcommand);
  activateOne();
  return requestid;
}

int SiteLogic::requestWipe(RequestCallback* cb, const Path & path, bool recursive) {
  int requestid = requestidcounter++;
  if (recursive) {
    requests.emplace_back(cb, requestid, REQ_WIPE_RECURSIVE, path.toString());
  }
  else {
    requests.emplace_back(cb, requestid, REQ_WIPE, path.toString());
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestDelete(RequestCallback* cb, const Path& path, bool recursive, bool allfiles) {
  int requestid = requestidcounter++;
  if (recursive) {
    int req = allfiles ? REQ_DEL_RECURSIVE : REQ_DEL_OWN;
    requests.emplace_back(cb, requestid, req, path.toString());
  }
  else {
    requests.emplace_back(cb, requestid, REQ_DEL, path.toString());
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestNuke(RequestCallback* cb, const Path & path, int multiplier, const std::string & reason) {
  int requestid = requestidcounter++;
  requests.emplace_back(cb, requestid, REQ_NUKE, path.toString(), reason, multiplier);
  activateOne();
  return requestid;
}

int SiteLogic::requestOneIdle(RequestCallback* cb) {
  int requestid = requestidcounter++;
  requests.emplace_back(cb, requestid, REQ_IDLE, site->getBasePath().toString(), site->getMaxIdleTime());
  activateOne();
  return requestid;
}

int SiteLogic::requestAllIdle(RequestCallback* cb, const Path & path, int idletime) {
  if (!idletime) {
    idletime = site->getMaxIdleTime();
  }
  int requestid = -1;
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    requestid = requestidcounter++;
    SiteLogicRequest request(cb, requestid, REQ_IDLE, path.toString(), idletime);
    request.setConnId(i);
    requests.push_back(request);
  }
  activateAll();
  return requestid;
}

int SiteLogic::requestAllIdle(RequestCallback* cb, int idletime) {
  return requestAllIdle(cb, site->getBasePath(), idletime);
}

int SiteLogic::requestMakeDirectory(RequestCallback* cb, const Path& path, const std::string & dirname) {
  int requestid = requestidcounter++;
  Path dirpath(dirname);
  if (dirpath.isRelative()) {
    requests.emplace_back(cb, requestid, REQ_MKDIR, path.toString(), dirname);
  }
  else {
    requests.emplace_back(cb, requestid, REQ_MKDIR, dirpath.dirName(), dirpath.baseName());
  }
  activateOne();
  return requestid;
}

int SiteLogic::requestMove(RequestCallback* cb, const Path& srcpath, const Path& dstpath) {
  int requestid = requestidcounter++;
  requests.emplace_back(cb, requestid, REQ_MOVE, srcpath.toString(), dstpath.toString());
  activateOne();
  return requestid;
}

bool SiteLogic::requestReady(int requestid) const {
  std::list<SiteLogicRequestReady>::const_iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->getId() == requestid) {
      return true;
    }
  }
  return false;
}

void SiteLogic::abortRace(const std::shared_ptr<SiteRace> & race) {
  if (!race) {
    return;
  }
  race->abort();
  removeRace(race);
}

void SiteLogic::removeRace(const std::shared_ptr<SiteRace> & race) {
  if (!race) {
    return;
  }
  abortTransfers(race);
  for (std::list<std::shared_ptr<SiteRace>>::iterator it = recentlylistedraces.begin(); it != recentlylistedraces.end(); it++) {
    if (*it == race) {
      recentlylistedraces.erase(it);
      break;
    }
  }
  for (std::vector<std::shared_ptr<SiteRace>>::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if (*it == race) {
      currentraces.erase(it);
      break;
    }
  }
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    connstatetracker[i].purgeSiteRace(race);
  }
  for (unsigned int i = 0; i < conns.size(); i++) {
    if (conns[i]->currentCommandOwner() == race) {
      conns[i]->resetCurrentCommandOwner();
    }
  }
}

void SiteLogic::abortTransfers(const std::shared_ptr<CommandOwner> & co) {
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    if (connstatetracker[i].isLoggedIn() && connstatetracker[i].isTransferLocked() &&
        connstatetracker[i].getCommandOwner() == co)
    {
      connstatetracker[i].getTransferMonitor()->getTransferStatus()->setAborted();
      disconnectConn(i);
      connectConn(i);
    }
  }
}

FileListData* SiteLogic::getFileListData(int requestid) const {
  std::list<SiteLogicRequestReady>::const_iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->getId() == requestid) {
      return static_cast<FileListData*>(it->getData());
    }
  }
  return nullptr;
}

DownloadFileData* SiteLogic::getDownloadFileData(int requestid) const {
  std::list<SiteLogicRequestReady>::const_iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->getId() == requestid) {
      return static_cast<DownloadFileData*>(it->getData());
    }
  }
  return nullptr;
}

void* SiteLogic::getOngoingRequestData(int requestid) const {
  for (size_t i = 0; i < connstatetracker.size(); ++i) {
    if (connstatetracker[i].hasRequest()) {
      const std::shared_ptr<SiteLogicRequest>& request = connstatetracker[i].getRequest();
      if (request->getId() == requestid) {
        return request->getPtrData();
      }
    }
  }
  return nullptr;
}

std::string SiteLogic::getRawCommandResult(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->getId() == requestid) {
      std::string ret = *static_cast<std::string *>(it->getData());
      return ret;
    }
  }
  return "";
}

bool SiteLogic::requestStatus(int requestid) const {
  std::list<SiteLogicRequestReady>::const_iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->getId() == requestid) {
      return it->getStatus();
    }
  }
  return false;
}

bool SiteLogic::finishRequest(int requestid) {
  std::list<SiteLogicRequestReady>::iterator it;
  for (it = requestsready.begin(); it != requestsready.end(); it++) {
    if (it->getId() == requestid) {
      bool status = it->getStatus();
      clearReadyRequest(*it);
      requestsready.erase(it);
      return status;
    }
  }
  for (unsigned int i = 0; i < connstatetracker.size(); i++) {
    if (connstatetracker[i].hasRequest() &&
        connstatetracker[i].getRequest()->getId() == requestid)
    {
      connstatetracker[i].finishRequest();
      if (!connstatetracker[i].isTransferLocked()) {
        available++;
      }
      return true;
    }
  }
  assert(false);
  return false;
}

const std::shared_ptr<Site> & SiteLogic::getSite() const {
  return site;
}

std::shared_ptr<SiteRace> SiteLogic::getRace(const std::string & race) const {
  for (std::vector<std::shared_ptr<SiteRace>>::const_iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getName().compare(race) == 0) {
      return *it;
    }
  }
  return std::shared_ptr<SiteRace>();
}

std::shared_ptr<SiteRace> SiteLogic::getRace(unsigned int id) const {
  for (std::vector<std::shared_ptr<SiteRace>>::const_iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return std::shared_ptr<SiteRace>();
}

bool SiteLogic::lockDownloadConn(const std::shared_ptr<FileList>& fl, int* ret, const std::shared_ptr<CommandOwner>& co, TransferMonitor* tm) {
  return lockTransferConn(fl, ret, tm, co, true);
}

bool SiteLogic::lockUploadConn(const std::shared_ptr<FileList>& fl, int* ret, const std::shared_ptr<CommandOwner>& co, TransferMonitor* tm) {
  return lockTransferConn(fl, ret, tm, co, false);
}

bool SiteLogic::lockTransferConn(const std::shared_ptr<FileList>& fl, int* ret, TransferMonitor* tm, const std::shared_ptr<CommandOwner>& co, bool isdownload) {
  TransferType type = getTransferType(isdownload, co);
  int lastreadyid = -1;
  bool foundreadythread = false;
  const Path & path = fl->getPath();
  for (unsigned int i = 0; i < conns.size(); i++) {
    if(connstatetracker[i].isLoggedIn() && !connstatetracker[i].isLocked() &&
        !conns[i]->isProcessing())
    {
      foundreadythread = true;
      lastreadyid = i;
      if (conns[i]->getCurrentPath() == path) {
        if (!getSlot(isdownload, type)) {
          return false;
        }
        *ret = i;
        connstatetracker[i].lockForTransfer(tm, fl, co, isdownload);
        conns[i]->setRawBufferCallback(tm);
        return true;
      }
    }
  }
  if (!foundreadythread) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (connstatetracker[i].isLoggedIn() && !connstatetracker[i].isHardLocked()) {
        foundreadythread = true;
        if (lastreadyid == -1) {
          lastreadyid = i;
          if (conns[i]->getTargetPath() == path) {
            if (!getSlot(isdownload, type)) {
              return false;
            }
            *ret = i;
            connstatetracker[i].lockForTransfer(tm, fl, co, isdownload);
            conns[i]->setRawBufferCallback(tm);
            handleConnection(i);
            return true;
          }
        }
      }
    }
  }
  if (foundreadythread) {
    if (!getSlot(isdownload, type)) {
      return false;
    }
    *ret = lastreadyid;
    connstatetracker[lastreadyid].lockForTransfer(tm, fl, co, isdownload);
    conns[lastreadyid]->setRawBufferCallback(tm);
    handleConnection(lastreadyid);
    return true;
  }
  else return false;
}

void SiteLogic::returnConn(int id, bool istransfer) {
  if (connstatetracker[id].isTransferLocked() && connstatetracker[id].getTransferType() == CST_DOWNLOAD) {
    transferComplete(id, true);
  }
  else if (connstatetracker[id].isTransferLocked() && connstatetracker[id].getTransferType() == CST_UPLOAD) {
    transferComplete(id, false);
  }
  if (istransfer) {
    connstatetracker[id].finishFileTransfer();
  }
  else {
    connstatetracker[id].finishTransfer();
  }
  handleConnection(id);
}

void SiteLogic::registerDownloadLock(int id, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co, TransferMonitor* tm) {
  TransferType type = getTransferType(true, co);
  available++;
  assert(getSlot(true, type));
  connstatetracker[id].lockForTransfer(tm, fl, co, true);
  conns[id]->setRawBufferCallback(tm);
  handleConnection(id);
}

void SiteLogic::setNumConnections(unsigned int num) {
  while (num < conns.size()) {
    bool success = false;
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (!conns[i]->isConnected()) {
        connstatetracker.erase(connstatetracker.begin() + i);
        delete conns[i];
        conns.erase(conns.begin() + i);
        success = true;
        break;
      }
    }
    if (success) {
      continue;
    }
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (conns[i]->isConnected() && !conns[i]->isProcessing()) {
        disconnectConn(i, true);
        connstatetracker.erase(connstatetracker.begin() + i);
        delete conns[i];
        conns.erase(conns.begin() + i);
        success = true;
        break;
      }
    }
    if (success) {
      continue;
    }
    if (conns.size() > 0) {
      disconnectConn(0, true);
      connstatetracker.erase(connstatetracker.begin());
      delete conns[0];
      conns.erase(conns.begin());
    }
  }
  while (num > conns.size()) {
    connstatetracker.push_back(ConnStateTracker());
    conns.push_back(new FTPConn(this, conns.size()));
  }
  for (unsigned int i = 0; i < conns.size(); i++) {
    conns[i]->setId(i);
  }
  ptrack->updateSlots(site->getMaxDown());
}

bool SiteLogic::downloadSlotAvailable(TransferType type) const {
  if (!available) {
    return false;
  }
  int maxlogins = site->getMaxLogins();
  if ((site->getLeaveFreeSlot() && slotsdn + slotsup + 1 >= maxlogins && maxlogins > 1)) {
    return false;
  }
  switch (type) {
    case TransferType::REGULAR:
      return slotsdn < static_cast<int>(site->getMaxDown());
    case TransferType::PRE:
      return slotsdn < static_cast<int>(site->getMaxDownPre());
    case TransferType::COMPLETE:
      return slotsdn < static_cast<int>(site->getMaxDownComplete());
    case TransferType::TRANSFERJOB:
      return slotsdn < static_cast<int>(site->getMaxDownTransferJob());
  }
  return false;
}

bool SiteLogic::uploadSlotAvailable() const {
  if (!available) {
    return false;
  }
  int maxlogins = site->getMaxLogins();
  if ((site->getLeaveFreeSlot() && slotsdn + slotsup + 1 >= maxlogins && maxlogins > 1)) {
    return false;
  }
  return slotsup < static_cast<int>(site->getMaxUp());
}

int SiteLogic::slotsAvailable() const {
  return available;
}

void SiteLogic::transferComplete(int id, bool isdownload, bool returntransferslot) {
  if (returntransferslot) {
    if (isdownload) {
      slotsdn--;
    }
    else {
      slotsup--;
    }
  }
  if (!connstatetracker[id].hasRequest()) {
    available++;
  }
  conns[id]->unsetRawBufferCallback();
}

bool SiteLogic::getSlot(bool isdownload, TransferType type) {
  if (isdownload) {
    if ((type == TransferType::REGULAR && slotsdn >= static_cast<int>(site->getMaxDown())) ||
        (type == TransferType::PRE && slotsdn >= static_cast<int>(site->getMaxDownPre())) ||
        (type == TransferType::COMPLETE && slotsdn >= static_cast<int>(site->getMaxDownComplete())) ||
        (type == TransferType::TRANSFERJOB && slotsdn >= static_cast<int>(site->getMaxDownTransferJob())))
    {
      return false;
    }
    slotsdn++;
  }
  else {
    if (slotsup >= static_cast<int>(site->getMaxUp())) {
      return false;
    }
    slotsup++;
  }
  available--;
  return true;
}

void SiteLogic::pushPotential(int score, const std::string & file, const std::shared_ptr<SiteLogic> & dst) {
  ptrack->pushPotential(score, file, dst, dst->getSite()->getMaxUp());
}

bool SiteLogic::potentialCheck(int score) {
  int max = ptrack->getMaxAvailablePotential();
  if (score >= max * 0.65) {
    return true;
  }
  return false;
}

int SiteLogic::getPotential() {
  return ptrack->getMaxAvailablePotential();
}

void SiteLogic::siteUpdated() {
  refreshgovernor.update();
  setNumConnections(site->getMaxLogins());
}

void SiteLogic::updateName() {
  for (unsigned int i = 0; i < conns.size(); i++) {
    conns[i]->updateName();
  }
  rawcommandrawbuf->rename(site->getName());
}

int SiteLogic::getCurrLogins() const {
  return loggedin;
}

int SiteLogic::getCurrDown() const {
  return slotsdn;
}

int SiteLogic::getCurrUp() const {
  return slotsup;
}

int SiteLogic::getCleanlyClosedConnectionsCount() const {
  int count = 0;
  for (int i = 0; i < (int)conns.size(); ++i) {
    if (conns[i]->isCleanlyClosed()) {
      count++;
    }
  }
  return count;
}

void SiteLogic::connectConn(int id) {
  if (!conns[id]->isConnected()) {
    connstatetracker[id].resetIdleTime();
    conns[id]->login();
  }
}

void SiteLogic::disconnectConn(int id, bool hard) {
  connstatetracker[id].resetIdleTime();
  cleanupConnection(id);
  if (conns[id]->isConnected()) {
    if (connstatetracker[id].isLoggedIn() && !connstatetracker[id].isQuitting()) {
      loggedin--;
      available--;
    }
    if (!hard && connstatetracker[id].isLoggedIn() && !conns[id]->isProcessing()) {
      conns[id]->doQUIT();
      connstatetracker[id].setQuitting();
    }
    else {
      conns[id]->disconnect();
      connstatetracker[id].setDisconnected();
    }
  }
}

void SiteLogic::cleanupConnection(int id) {
  while (connstatetracker[id].isListOrTransferLocked()) {
    reportTransferErrorAndFinish(id, TM_ERR_CLEANUP);
  }
  if (connstatetracker[id].hasRequest()) {
    setRequestReady(id, NULL, false);
  }
  for (int i = 0; i < (int)conns.size(); ++i) {
    if (i != id && conns[i]->isConnected()) {
      return;
    }
  }
  bool erased = true;
  while (erased) {
    erased = false;
    for (std::list<SiteLogicRequest>::iterator it = requests.begin(); it != requests.end(); it++)
    {
      if (it->getConnId() != -1 && it->getConnId() != id) {
        continue;
      }
      connstatetracker[id].setRequest(*it);
      requests.erase(it);
      setRequestReady(id, NULL, false, false);
      erased = true;
      break;
    }
  }
}

void SiteLogic::finishTransferGracefully(int id) {
  assert(connstatetracker[id].hasTransfer() &&
               !connstatetracker[id].isListLocked());
  switch (connstatetracker[id].getTransferType()) {
    case CST_DOWNLOAD:
      connstatetracker[id].getTransferMonitor()->sourceComplete();
      transferComplete(id, true);
      break;
    case CST_UPLOAD:
      connstatetracker[id].getTransferMonitor()->targetComplete();
      transferComplete(id, false);
      break;
  }
  connstatetracker[id].finishTransfer();
}

void SiteLogic::listCompleted(int id, int storeid, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner> & co) {
  const Core::BinaryData& data = global->getLocalStorage()->getStoreContent(storeid);
  conns[id]->setListData(co, fl);
  conns[id]->parseFileList((char *) &data[0], data.size());
  listRefreshed(id);
  global->getLocalStorage()->purgeStoreContent(storeid);
}

void SiteLogic::downloadCompleted(int id, int storeid, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner> & co) {
  if (!connstatetracker[id].hasRequest()) {
    return;
  }
  std::shared_ptr<SiteLogicRequest> request = connstatetracker[id].getRequest();
  if (request->getType() != REQ_DOWNLOAD_FILE_STAGE2) {
    return;
  }
  DownloadFileData* dlfdata = static_cast<DownloadFileData*>(request->getPtrData());
  if (storeid != -1) {
    dlfdata->data = global->getLocalStorage()->getStoreContent(storeid);
    global->getLocalStorage()->purgeStoreContent(storeid);
  }
  setRequestReady(id, dlfdata, true);
}

void SiteLogic::issueRawCommand(unsigned int id, const std::string & command) {
  int requestid = requestidcounter++;
  std::string expandedcommand = expandVariables(command);
  SiteLogicRequest request(nullptr, requestid, REQ_RAW, conns[id]->getCurrentPath().toString(), command);
  request.setConnId(id);
  requests.push_back(request);
  if (!conns[id]->isConnected()) {
    connectConn(id);
    return;
  }
  if (connstatetracker[id].isLoggedIn() && !connstatetracker[id].isLocked() && !conns[id]->isProcessing()) {
    handleRequest(id);
  }
}

RawBuffer * SiteLogic::getRawCommandBuffer() const {
  return rawcommandrawbuf;
}

RawBuffer * SiteLogic::getAggregatedRawBuffer() const {
  return aggregatedrawbuf;
}

void SiteLogic::raceGlobalComplete(const std::shared_ptr<SiteRace> & sr) {
  removeRace(sr);
  if (site->getStayLoggedIn()) {
    return;
  }
  bool stillactive = !currentraces.empty();
  for (std::list<std::shared_ptr<SiteTransferJob> >::const_iterator it = transferjobs.begin(); it != transferjobs.end(); it++) {
    if (!(*it)->isDone()) {
      stillactive = true;
      break;
    }
  }
  if (!stillactive) {
    for (unsigned int i = 0; i < conns.size(); i++) {
      if (conns[i]->isConnected() && !conns[i]->isProcessing() && !connstatetracker[i].isLocked()) {
        disconnectConn(i);
      }
    }
  }
}

void SiteLogic::raceLocalComplete(const std::shared_ptr<SiteRace> & sr, int uploadslotsleft, bool reportdone) {
  sr->complete(reportdone);
  if (site->getStayLoggedIn()) {
    return;
  }
  bool stillactive = false;
  for (unsigned int i = 0; i < currentraces.size(); i++) {
    if (!currentraces[i]->isDone()) {
      stillactive = true;
      break;
    }
  }
  if (!stillactive) {
    int downloadslots = site->getMaxDown();
    if (static_cast<int>(site->getMaxDownComplete()) > downloadslots) {
      downloadslots = site->getMaxDownComplete();
    }
    if (sr->isDownloadOnly() && static_cast<int>(site->getMaxDownPre()) > downloadslots) {
      downloadslots = site->getMaxDownPre();
    }
    int stillneededslots = downloadslots < uploadslotsleft ? downloadslots : uploadslotsleft;
    int killnum = site->getMaxLogins() - stillneededslots;
    if (killnum < 0) {
      killnum = 0;
    }
    for (unsigned int i = 0; i < conns.size() && killnum > 0; i++) {
      if (conns[i]->isConnected() && !conns[i]->isProcessing() && !connstatetracker[i].isLocked()) {
        disconnectConn(i);
        killnum--;
      }
    }
  }
}

const std::vector<FTPConn *> * SiteLogic::getConns() const {
  return &conns;
}

FTPConn * SiteLogic::getConn(int id) const {
  std::vector<FTPConn *>::const_iterator it;
  for (it = conns.begin(); it != conns.end(); it++) {
    if ((*it)->getId() == id) {
      return *it;
    }
  }
  return NULL;
}

std::string SiteLogic::getStatus(int id) const {
  int idletime = connstatetracker[id].getTimePassed()/1000;
  if (!conns[id]->isProcessing() && conns[id]->isConnected() && idletime) {
    return "IDLE " + std::to_string(idletime) + "s";
  }
  return conns[id]->getStatus();
}

void SiteLogic::preparePassiveTransfer(int id, const std::string& file, bool fxp, bool ipv6, bool ssl, bool sslclient) {
  connstatetracker[id].setTransfer(file, fxp, ipv6, ssl, sslclient);
  initTransfer(id);
}

void SiteLogic::prepareActiveTransfer(int id, const std::string& file, bool fxp, bool ipv6, const std::string& host, int port, bool ssl, bool sslclient) {
  connstatetracker[id].setTransfer(file, fxp, ipv6, host, port, ssl, sslclient);
  initTransfer(id);
}

void SiteLogic::preparePassiveList(int id, TransferMonitor* tmb, bool ipv6, bool ssl) {
  connstatetracker[id].setList(tmb, ipv6, ssl);
  initTransfer(id);
}

void SiteLogic::prepareActiveList(int id, TransferMonitor* tmb, bool ipv6, const std::string & host, int port, bool ssl) {
  connstatetracker[id].setList(tmb, ipv6, host, port, ssl);
  initTransfer(id);
}

void SiteLogic::download(int id) {
  if (connstatetracker[id].transferInitialized()) {
    if (!connstatetracker[id].getTransferAborted()) {
      conns[id]->doRETR(connstatetracker[id].getTransferFile());
    }
    else {
      transferComplete(id, true);
      connstatetracker[id].finishTransfer();
      handleConnection(id);
    }
  }
  else {
    handleConnection(id);
  }
}

void SiteLogic::upload(int id) {
  if (connstatetracker[id].transferInitialized()) {
    if (!connstatetracker[id].getTransferAborted()) {
      conns[id]->doSTOR(connstatetracker[id].getTransferFile());
    }
    else {
      transferComplete(id, false);
      connstatetracker[id].finishTransfer();
      handleConnection(id);
    }
  }
  else {
    handleConnection(id);
  }
}

void SiteLogic::list(int id) {
  conns[id]->doLIST();
}

void SiteLogic::listAll(int id) {
  conns[id]->doLISTa();
}

void SiteLogic::getFileListConn(int id, bool hiddenfiles) {
  connstatetracker[id].use();
  connstatetracker[id].useRefreshTokenFor(conns[id]->getCurrentPath().toString());
  if (site->getListCommand() == SITE_LIST_STAT) {
    if (hiddenfiles) {
      conns[id]->doSTATla();
    }
    else {
      conns[id]->doSTAT();
    }
  }
  else {
    std::shared_ptr<FileList> fl = conns[id]->newFileList();
    global->getTransferManager()->getFileList(global->getSiteLogicManager()->getSiteLogic(this), id, hiddenfiles, fl, conns[id]->isIPv6());
  }
}

void SiteLogic::getFileListConn(int id, const std::shared_ptr<CommandOwner> & co, const std::shared_ptr<FileList>& fl) {
  assert(!!fl);
  connstatetracker[id].use();
  connstatetracker[id].useRefreshTokenFor(fl->getPath().toString());
  if (site->getListCommand() == SITE_LIST_STAT) {
    conns[id]->doSTAT(co, fl);
  }
  else {
    global->getTransferManager()->getFileList(global->getSiteLogicManager()->getSiteLogic(this), id, false, fl, conns[id]->isIPv6(), co);
  }
}

void SiteLogic::passiveModeCommand(int id) {
  if (connstatetracker[id].getTransferIPv6()) {
    conns[id]->doEPSV();
    return;
  }
  if (connstatetracker[id].getTransferFXP() &&
      connstatetracker[id].getTransferSSL() &&
      connstatetracker[id].getTransferSSLClient() &&
      site->supportsCPSV())
  {
    conns[id]->doCPSV();
  }
  else {
    conns[id]->doPASV();
  }
}

const ConnStateTracker * SiteLogic::getConnStateTracker(int id) const {
  return &connstatetracker[id];
}

void SiteLogic::setRequestReady(unsigned int id, void* data, bool status, bool returnslot) {
  const std::shared_ptr<SiteLogicRequest> & request = connstatetracker[id].getRequest();
  int requestid = request->getId();
  requestsready.push_back(SiteLogicRequestReady(request->getType(), requestid, data, status));
  clearExpiredReadyRequests();
  timesincelastrequestready = 0;
  RequestCallback* cb = request->getCallback();
  if (!status) {
    cleanupFailedRequest(request);
  }
  connstatetracker[id].finishRequest();
  if (returnslot && !connstatetracker[id].isTransferLocked()) {
    available++;
  }
  if (cb) {
    cb->requestReady(this, requestid);
  }
  else {
    finishRequest(requestid);
  }
}

void SiteLogic::clearExpiredReadyRequests() {
  while (timesincelastrequestready > MAX_REQUEST_READY_QUEUE_IDLE_TIME && requestsready.size() > MAX_REQUEST_READY_QUEUE_SIZE) {
    clearReadyRequest(requestsready.front());
    requestsready.pop_front();
  }
}

void SiteLogic::clearReadyRequest(SiteLogicRequestReady & request) {
  void * data = request.getData();
  if (request.getStatus()) {
    if (request.getType() == REQ_FILELIST) {
      delete static_cast<FileListData *>(data);
    }
    else if (request.getType() == REQ_RAW) {
      delete static_cast<std::string *>(data);
    }
    else if (request.getType() == REQ_DOWNLOAD_FILE_STAGE2) {
      delete static_cast<DownloadFileData*>(data);
    }
  }
}

void SiteLogic::antiAntiIdle(int id) {
  handleConnection(id);
  if (!connstatetracker[id].isLocked() && !conns[id]->isProcessing()) {
    Path target = site->getBasePath();
    if (conns[id]->getCurrentPath() == target) {
      if (!site->getSections().empty()) {
        int randomsection = rand() % site->getSections().size();
        int i = 0;
        for (std::map<std::string, Path>::const_iterator it = site->sectionsBegin(); it != site->sectionsEnd(); ++it, ++i) {
          if (i == randomsection) {
            target = it->second;
          }
        }
      }
    }
    int requestid = requestidcounter++;
    SiteLogicRequest request(nullptr, requestid, REQ_FILELIST, target.toString());
    request.setConnId(id);
    requests.push_back(request);
    handleConnection(id);
  }
}

std::string SiteLogic::expandVariables(const std::string& text) const {
  std::string expanded = text;
  std::string pathtoken = "$path(";
  size_t pos = text.find(pathtoken);
  if (pos != std::string::npos) {
    std::string replacewith;
    size_t endpos = text.find(")", pos);
    if (endpos != std::string::npos) {
      std::string section = text.substr(pos + pathtoken.length(), endpos - pos - pathtoken.length());
      if (site->hasSection(section)) {
        replacewith = site->getSectionPath(section).toString();
      }
      else {
        replacewith = "<section not found>";
      }
      expanded.replace(pos, endpos - pos + 1, replacewith);
    }
  }
  return expanded;
}
