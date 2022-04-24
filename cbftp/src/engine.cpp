#include "engine.h"

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <set>
#include <tuple>
#include <unordered_set>

#include "core/workmanager.h"
#include "core/tickpoke.h"
#include "scoreboard.h"
#include "scoreboardelement.h"
#include "globalcontext.h"
#include "sitelogic.h"
#include "site.h"
#include "filelist.h"
#include "file.h"
#include "path.h"
#include "sitelogicmanager.h"
#include "transfermanager.h"
#include "race.h"
#include "siterace.h"
#include "skiplist.h"
#include "eventlog.h"
#include "sitemanager.h"
#include "sitetransferjob.h"
#include "transferjob.h"
#include "pendingtransfer.h"
#include "localstorage.h"
#include "localfilelist.h"
#include "transferstatus.h"
#include "util.h"
#include "preparedrace.h"
#include "sectionmanager.h"
#include "transferprotocol.h"

#define POKEINTERVAL 1000
#define STATICTIMEFORCOMPLETION 5000
#define DIROBSERVETIME 20000
#define SFVDIROBSERVETIME 5000
#define NFO_PRIO_AFTER_SEC 15
#define RETRY_CONNECT_UNTIL_SEC 15
#define MAX_PERCENTAGE_FOR_INCOMPLETE_DELETE 95

namespace {

enum EngineTickMessages {
  TICK_MSG_TICKER,
};

PrioType getPrioType(File * f) {
  if (f->getExtension().compare("sfv") == 0) {
    return PrioType::PRIO;
  }
  else if (f->getExtension().compare("nfo") == 0) {
    return PrioType::PRIO_LATER;
  }
  return PrioType::NORMAL;
}

struct AddSite {
  AddSite(const std::string& name, const std::shared_ptr<SiteLogic>& sl, bool downloadonly) : name(name), sl(sl), downloadonly(downloadonly) {}
  std::string name;
  std::shared_ptr<SiteLogic> sl;
  bool downloadonly;
};

bool containsPattern(const std::shared_ptr<FileList>& fl, const SkipListMatch& match, bool dir) {
  return (!match.regex && fl->containsPattern(match.matchpattern, match.matchedpath, dir)) ||
         (match.regex && fl->containsRegexPattern(match.matchregexpattern, match.matchedpath, dir));
}

bool containsPatternBefore(const std::shared_ptr<FileList>& fl, const SkipListMatch& match, const std::string& item) {
  return (!match.regex && fl->containsPatternBefore(match.matchpattern, true, item)) ||
         (match.regex && fl->containsRegexPatternBefore(match.matchregexpattern, true, item));
}

}

Engine::Engine() :
  scoreboard(std::make_shared<ScoreBoard>()),
  failboard(std::make_shared<ScoreBoard>()),
  maxavgspeed(1024),
  pokeregistered(false),
  nextid(1),
  maxpointsfilesize(2000),
  maxpointsavgspeed(3000),
  maxpointspriority(2500),
  maxpointspercentageowned(2000),
  maxpointslowprogress(2000),
  preparedraceexpirytime(120),
  startnextpreparedtimeout(300),
  startnextprepared(false),
  nextpreparedtimeremaining(0),
  forcescoreboard(false),
  maxspreadjobshistory(100),
  maxtransferjobshistory(100),
  maxspreadjobtimeseconds(0)
{
}

Engine::~Engine() {

}

std::shared_ptr<Race> Engine::newSpreadJob(int profile, const std::string& release, const std::string& section, const std::list<std::string>& sites, bool reset, const std::list<std::string>& dlonlysites) {
  if (release.empty()) {
    global->getEventLog()->log("Engine", "Spread job skipped due to missing release name.");
    return std::shared_ptr<Race>();
  }
  if (section.empty()) {
    global->getEventLog()->log("Engine", "Spread job skipped due to missing section name.");
    return std::shared_ptr<Race>();
  }
  Section * sectionptr = global->getSectionManager()->getSection(section);
  if (!sectionptr) {
    global->getEventLog()->log("Engine", "Spread job skipped due to undefined section: " + section);
    return std::shared_ptr<Race>();
  }
  std::shared_ptr<Race> race;
  bool append = false;
  for (std::list<std::shared_ptr<Race> >::iterator it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getName() == release && (*it)->getSection() == section) {
      race = *it;
      append = true;
      break;
    }
  }
  if (release.find("/") != std::string::npos) {
    global->getEventLog()->log("Engine", "Spread job skipped due to invalid target: " + release);
    return std::shared_ptr<Race>();
  }
  if (!race) {
    race = std::make_shared<Race>(nextid++, static_cast<SpreadProfile>(profile), release, section);
  }
  std::list<AddSite> addsites;
  bool globalskipped = sectionptr->getSkipList().check(release, true, false).action == SKIPLIST_DENY;
  std::list<std::string> skippedsites;
  std::unordered_map<std::string, bool> allsites; // sitename, download-only
  for (const std::string& site : sites) {
    allsites[site] = false;
  }
  for (const std::string& site : dlonlysites) {
    allsites[site] = true;
  }
  for (const std::pair<const std::string, bool>& site : allsites) {
    const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site.first);
    if (!sl) {
      global->getEventLog()->log("Engine", "Trying to use a nonexisting site: " + site.first);
      continue;
    }
    if (sl->getSite()->getDisabled()) {
      global->getEventLog()->log("Engine", "Skipping disabled site: " + site.first);
      continue;
    }
    bool downloadonly = site.second || (sl->getSite()->isAffiliated(race->getGroup()) && profile != SPREAD_DISTRIBUTE);
    if (downloadonly && sl->getSite()->getAllowDownload() == SiteAllowTransfer::SITE_ALLOW_TRANSFER_NO) {
      global->getEventLog()->log("Engine", "Skipping site because of download-only mode and download disabled: " + site.first);
      continue;
    }
    if (!sl->getSite()->hasSection(section)) {
      global->getEventLog()->log("Engine", "Trying to use an undefined section: " +
          section + " on " + site.first);
      continue;
    }
    SkipListMatch match = sl->getSite()->getSkipList().check((sl->getSite()->getSectionPath(section) / race->getName()).toString(),
                                                             true, false, &sectionptr->getSkipList());
    if (match.action == SKIPLIST_DENY && !downloadonly) {
      skippedsites.push_back(sl->getSite()->getName());
      continue;
    }
    if (append && !!race->getSiteRace(site.first)) {
      continue;
    }
    addsites.emplace_back(site.first, sl, downloadonly);
  }
  if (!addsites.empty()) {
    for (const std::string & skipsite : skippedsites) {
      global->getEventLog()->log("Engine", "Skipping site " + skipsite +
                                 " due to skiplist match: " + release);
      continue;
    }
  }
  else if (globalskipped) {
    global->getEventLog()->log("Engine", "Spread job skipped due to skiplist match: " + release);
    return std::shared_ptr<Race>();
  }
  bool noupload = true;
  bool nodownload = true;
  for (const AddSite& site : addsites) {
    if (site.sl->getSite()->getAllowUpload() == SITE_ALLOW_TRANSFER_YES && !site.downloadonly) {
      noupload = false;
    }
    if (site.sl->getSite()->getAllowDownload() == SITE_ALLOW_TRANSFER_YES ||
        (site.sl->getSite()->getAllowDownload() == SITE_ALLOW_DOWNLOAD_MATCH_ONLY && site.downloadonly))
    {
      nodownload = false;
    }
  }
  if (!append && (noupload || nodownload) && profile != SPREAD_DISTRIBUTE) {
    global->getEventLog()->log("Engine", "Ignoring attempt to spread " + release + " in "
        + section + " since no transfers would be performed.");
    return std::shared_ptr<Race>();
  }
  if (addsites.size() < 2 && !append) {
    global->getEventLog()->log("Engine", "Ignoring attempt to spread " + release + " in "
        + section + " on less than 2 sites.");
    return std::shared_ptr<Race>();
  }
  if (addsites.size() > 0 || append) {
    checkStartPoke();
    if (profile == SPREAD_PREPARE) {
      if (startnextprepared) {
        startnextprepared = false;
        profile = SPREAD_RACE;
      }
      else {
        global->getEventLog()->log("Engine", "Preparing spread job: " + section + "/" + release +
                  " on " + std::to_string((int)addsites.size()) + " sites.");
        std::list<std::pair<std::string, bool>> preparesites;
        for (const AddSite& site : addsites) {
          preparesites.emplace_back(site.name, site.downloadonly);
        }
        int preparedraceid = append ? nextid++ : race->getId();
        preparedraces.push_back(std::make_shared<PreparedRace>(preparedraceid, release, section, preparesites, reset, preparedraceexpirytime));
        for (const AddSite& site : addsites) {
          site.sl->activateAll();
        }
        return std::shared_ptr<Race>();
      }
    }
    bool readdtocurrent = true;
    if (append) {
      if (reset) {
        resetRace(race, true);
      }
      if (currentraces.contains(race)) {
        readdtocurrent = false;
      }
      else {
        global->getEventLog()->log("Engine", "Reactivating spread job: " + section + "/" + release);
        race->setUndone();
        for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
          it->second->activateAll();
        }
      }
    }
    for (const AddSite& site : addsites) {
      addSiteToRace(race, site.name, site.downloadonly);
    }
    if (!append) {
      currentraces.push_back(race);
      allraces.push_back(race);
      rotateSpreadJobsHistory();
      dropped = 0;
      global->getEventLog()->log("Engine", "Starting spread job: " + section + "/" + release +
          " on " + std::to_string((int)addsites.size()) + " sites.");
      sectionptr->addJob();
      global->getStatistics()->addSpreadJob();
    }
    else {
      if (addsites.size()) {
        global->getEventLog()->log("Engine", "Appending to spread job: " + section + "/" + release +
            " with " + std::to_string((int)addsites.size()) + " site" + (addsites.size() > 1 ? "s" : "") + ".");
      }
      if (readdtocurrent) {
        currentraces.push_back(race);
        removeFromFinished(race);
      }
    }
    setSpeedScale();
    preSeedPotentialData(race);
  }
  return race;
}

std::shared_ptr<Race> Engine::newRace(const std::string& release, const std::string& section, const std::list<std::string>& sites, bool reset, const std::list<std::string>& dlonlysites) {
  return newSpreadJob(SPREAD_RACE, release, section, sites, reset, dlonlysites);
}

std::shared_ptr<Race> Engine::newDistribute(const std::string& release, const std::string& section, const std::list<std::string>& sites, bool reset, const std::list<std::string>& dlonlysites) {
  return newSpreadJob(SPREAD_DISTRIBUTE, release, section, sites, reset, dlonlysites);
}

bool Engine::prepareRace(const std::string & release, const std::string& section, const std::list<std::string>& sites, bool reset, const std::list<std::string>& dlonlysites) {
  size_t preparedbefore = preparedraces.size();
  std::shared_ptr<Race> race = newSpreadJob(SPREAD_PREPARE, release, section, sites, reset, dlonlysites);
  return preparedraces.size() > preparedbefore || race;
}

void Engine::startPreparedRace(unsigned int id) {
  std::list<std::shared_ptr<PreparedRace>>::iterator it = preparedraces.find(id);
  if (it != preparedraces.end()) {
    std::list<std::string> sites;
    std::list<std::string> dlonlysites;
    for (const std::pair<std::string, bool>& site : (*it)->getSites()) {
      if (site.second) {
        dlonlysites.push_back(site.first);
      }
      else {
        sites.push_back(site.first);
      }
    }
    newRace((*it)->getName(), (*it)->getSection(), sites, (*it)->getReset(), dlonlysites);
    preparedraces.erase(it);
  }
}

void Engine::deletePreparedRace(unsigned int id) {
  preparedraces.erase(id);
}

void Engine::startLatestPreparedRace() {
  if (preparedraces.size()) {
    std::shared_ptr<PreparedRace> preparedrace = preparedraces.back();
    startPreparedRace(preparedraces.back()->getId());
  }
}

void Engine::toggleStartNextPreparedRace() {
  if (!startnextprepared) {
    startnextprepared = true;
    nextpreparedtimeremaining = getNextPreparedRaceStarterTimeout() * 1000;
    checkStartPoke();
    global->getEventLog()->log("Engine", "Enabling next prepared spread job starter");
  }
  else {
    startnextprepared = false;
    global->getEventLog()->log("Engine", "Disabling next prepared spread job starter");
  }
}

bool Engine::getNextPreparedRaceStarterEnabled() const {
  return startnextprepared;
}

int Engine::getNextPreparedRaceStarterTimeout() const {
  return startnextpreparedtimeout;
}

int Engine::getNextPreparedRaceStarterTimeRemaining() const {
  return nextpreparedtimeremaining / 1000;
}

unsigned int Engine::newTransferJobDownload(const std::string& srcsite, const std::shared_ptr<FileList>& srcfilelist, const std::string& file, const Path& dstpath) {
  return newTransferJobDownload(srcsite, srcfilelist, file, dstpath, file);
}

unsigned int Engine::newTransferJobUpload(const Path& srcpath, const std::string& file, const std::string& dstsite, const std::shared_ptr<FileList>& dstfilelist) {
  return newTransferJobUpload(srcpath, file, dstsite, dstfilelist, file);
}

unsigned int Engine::newTransferJobFXP(const std::string& srcsite, const std::shared_ptr<FileList>& srcfilelist, const std::string& dstsite, const std::shared_ptr<FileList>& dstfilelist, const std::string& file) {
  return newTransferJobFXP(srcsite, srcfilelist, file, dstsite, dstfilelist, file);
}

unsigned int Engine::newTransferJobDownload(const std::string& srcsite, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcfile, const Path& dstpath, const std::string& dstfile) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(srcsite);
  if (!sl) {
    global->getEventLog()->log("Engine", "Bad site name: " + srcsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, sl, srcfilelist, srcfile, dstpath, dstfile);
  tj->createSiteTransferJobs(tj);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting download job: " + srcfile +
            " from " + srcsite);
  sl->addTransferJob(tj->getSrcTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  rotateTransferJobsHistory();
  return id;
}

unsigned int Engine::newTransferJobDownload(const std::string& srcsite, const Path& srcpath, const std::string& srcsection, const std::string& srcfile, const Path& dstpath, const std::string& dstfile) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(srcsite);
  if (!sl) {
    global->getEventLog()->log("Engine", "Bad site name: " + srcsite);
    return 0;
  }
  if (!srcsection.empty() && !sl->getSite()->hasSection(srcsection)) {
    global->getEventLog()->log("Engine", "Bad section name: " + srcsection);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, sl, srcpath, srcsection, srcfile, dstpath, dstfile);
  tj->createSiteTransferJobs(tj);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting download job: " + srcfile +
            " from " + srcsite);
  sl->addTransferJob(tj->getSrcTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  rotateTransferJobsHistory();
  return id;
}

unsigned int Engine::newTransferJobUpload(const Path& srcpath, const std::string& srcfile, const std::string& dstsite, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstfile) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!sl) {
    global->getEventLog()->log("Engine", "Bad site name: " + dstsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, srcpath, srcfile, sl, dstfilelist, dstfile);
  tj->createSiteTransferJobs(tj);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting upload job: " + srcfile +
            " to " + dstsite);
  sl->addTransferJob(tj->getDstTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  rotateTransferJobsHistory();
  return id;
}

unsigned int Engine::newTransferJobUpload(const Path& srcpath, const std::string& srcfile, const std::string& dstsite, const Path& dstpath, const std::string& dstsection, const std::string& dstfile) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!sl) {
    global->getEventLog()->log("Engine", "Bad site name: " + dstsite);
    return 0;
  }
  if (!dstsection.empty() && !sl->getSite()->hasSection(dstsection)) {
    global->getEventLog()->log("Engine", "Bad section name: " + dstsection);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, srcpath, srcfile, sl, dstpath, dstsection, dstfile);
  tj->createSiteTransferJobs(tj);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting upload job: " + srcfile +
            " to " + dstsite);
  sl->addTransferJob(tj->getDstTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  rotateTransferJobsHistory();
  return id;
}

unsigned int Engine::newTransferJobFXP(const std::string& srcsite, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcfile, const std::string& dstsite, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstfile) {
  const std::shared_ptr<SiteLogic> slsrc = global->getSiteLogicManager()->getSiteLogic(srcsite);
  const std::shared_ptr<SiteLogic> sldst = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!slsrc) {
    global->getEventLog()->log("Engine", "Bad site name: " + srcsite);
    return 0;
  }
  if (!sldst) {
    global->getEventLog()->log("Engine", "Bad site name: " + dstsite);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, slsrc, srcfilelist, srcfile, sldst, dstfilelist, dstfile);
  tj->createSiteTransferJobs(tj);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting FXP job: " + srcfile +
            " - " + srcsite + " -> " + dstsite);
  slsrc->addTransferJob(tj->getSrcTransferJob());
  sldst->addTransferJob(tj->getDstTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  rotateTransferJobsHistory();
  return id;
}

unsigned int Engine::newTransferJobFXP(const std::string& srcsite, const Path& srcpath, const std::string& srcsection, const std::string& srcfile, const std::string& dstsite, const Path& dstpath, const std::string& dstsection, const std::string& dstfile) {
  const std::shared_ptr<SiteLogic> slsrc = global->getSiteLogicManager()->getSiteLogic(srcsite);
  const std::shared_ptr<SiteLogic> sldst = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!slsrc) {
    global->getEventLog()->log("Engine", "Bad site name: " + srcsite);
    return 0;
  }
  if (!sldst) {
    global->getEventLog()->log("Engine", "Bad site name: " + dstsite);
    return 0;
  }
  if (!srcsection.empty() && !slsrc->getSite()->hasSection(srcsection)) {
    global->getEventLog()->log("Engine", "Bad section name: " + srcsection);
    return 0;
  }
  if (!dstsection.empty() && !sldst->getSite()->hasSection(dstsection)) {
    global->getEventLog()->log("Engine", "Bad section name: " + dstsection);
    return 0;
  }
  if (srcfile.empty() || dstfile.empty()) {
    global->getEventLog()->log("Engine", "Bad file name.");
    return 0;
  }
  unsigned int id = nextid++;
  std::shared_ptr<TransferJob> tj = std::make_shared<TransferJob>(id, slsrc, srcpath, srcsection, srcfile, sldst, dstpath, dstsection, dstfile);
  tj->createSiteTransferJobs(tj);
  alltransferjobs.push_back(tj);
  currenttransferjobs.push_back(tj);
  global->getEventLog()->log("Engine", "Starting FXP job: " + srcfile +
            " - " + srcsite + " -> " + dstsite);
  slsrc->addTransferJob(tj->getSrcTransferJob());
  sldst->addTransferJob(tj->getDstTransferJob());
  checkStartPoke();
  global->getStatistics()->addTransferJob();
  rotateTransferJobsHistory();
  return id;
}

void Engine::removeSiteFromRace(const std::shared_ptr<Race> & race, const std::string & site) {
  if (!!race) {
    std::shared_ptr<SiteRace> sr = race->getSiteRace(site);
    if (!!sr) {
      const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site);
      race->removeSite(sr);
      wipeFromScoreBoard(sr);
      if (!!sl) {
        sl->abortRace(sr);
      }
    }
  }
}

void Engine::removeSiteFromAllRunningSpreadJobs(const std::string& site) {
  for (std::list<std::shared_ptr<Race>>::const_iterator it = getCurrentRacesBegin(); it != getCurrentRacesEnd(); ++it) {
    removeSiteFromRace(*it, site);
  }
}

void Engine::removeSiteFromRaceDeleteFiles(const std::shared_ptr<Race> & race, const std::string & site, bool allfiles, bool deleteoncomplete) {
  if (!!race) {
    std::shared_ptr<SiteRace> sr = race->getSiteRace(site);
    if (!!sr) {
      const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site);
      race->removeSite(sr);
      wipeFromScoreBoard(sr);
      if (!!sl) {
        if (deleteoncomplete || isIncompleteEnoughForDelete(race, sr)) {
          sl->requestDelete(nullptr, sr->getPath(), true, allfiles);
        }
        sl->abortRace(sr);
      }
    }
  }
}

void Engine::abortRace(const std::shared_ptr<Race> & race) {
  if (!!race && !race->isDone()) {
    race->abort();
    for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
      it->second->abortRace(it->first);
      wipeFromScoreBoard(it->first);
    }
    currentraces.remove(race);
    finishedraces.push_back(race);
    global->getEventLog()->log("Engine", "Spread job aborted: " + race->getName());
    rotateSpreadJobsHistory();
  }
}

void Engine::resetRace(const std::shared_ptr<Race> & race, bool hard) {
  if (!!race) {
    race->reset();
    for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
      wipeFromScoreBoard(it->first);
      if (hard) {
        it->first->hardReset();
      }
      else {
        it->first->softReset();
      }
      it->second->resetRace(it->first);
    }
    if (!currentraces.contains(race)) {
      currentraces.push_back(race);
      removeFromFinished(race);
    }
    checkStartPoke();
    global->getEventLog()->log("Engine", "Spread job reset: " + race->getName());
  }
}

void Engine::wipeFromScoreBoard(const std::shared_ptr<SiteRace> & sr) {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it;
  for (it = sr->fileListsBegin(); it != sr->fileListsEnd(); it++) {
    scoreboard->wipe(it->second);
    failboard->wipe(it->second);
  }
}

bool Engine::waitingInScoreBoard(const std::shared_ptr<Race> & race) const {
  std::vector<ScoreBoardElement *>::iterator it;
  for (it = scoreboard->begin(); it != scoreboard->end(); ++it) {
    ScoreBoardElement * sbe = *it;
    if (sbe->getRace() != race) {
      continue;
    }
    if (transferExpectedSoon(sbe)) {
      return true;
    }
  }
  return false;
}

void Engine::restoreFromFailed(const std::shared_ptr<Race>& race) {
  std::vector<ScoreBoardElement*>::iterator it;
  std::list<std::tuple<std::string, std::shared_ptr<FileList>, std::shared_ptr<FileList>>> removelist;
  for (it = failboard->begin(); it != failboard->end(); ++it) {
    ScoreBoardElement* sbe = *it;
    if (sbe->getRace() == race) {
      scoreboard->update(sbe);
      removelist.emplace_back(sbe->fileName(), sbe->getSourceFileList(),
                              sbe->getDestinationFileList());
    }
  }
  for (const std::tuple<std::string, std::shared_ptr<FileList>, std::shared_ptr<FileList>>& elem : removelist) {
    failboard->remove(std::get<0>(elem), std::get<1>(elem), std::get<2>(elem));
  }
  scoreboard->sort();
  scoreboard->shuffleEquals();
}

void Engine::removeFromFinished(const std::shared_ptr<Race> & race) {
  finishedraces.erase(race);
}

void Engine::clearSkipListCaches() {
  for (const std::shared_ptr<Site> & site : skiplistcachesites) {
    site->getSkipList().wipeCache();
  }
  for (const std::string & section : skiplistcachesections) {
    Section * sec = global->getSectionManager()->getSection(section);
    if (sec) {
      sec->getSkipList().wipeCache();
    }
  }
  global->getSkipList()->wipeCache();
  skiplistcachesites.clear();
  skiplistcachesections.clear();
}

bool Engine::transferExpectedSoon(ScoreBoardElement * sbe) const {
  const std::string & filename = sbe->fileName();
  if (sbe->getDestinationSiteRace()->getStatus() != RaceStatus::RUNNING) {
    return false;
  }
  if (!sbe->getSource()->getCurrLogins() || !sbe->getDestination()->getCurrLogins()) {
    return false;
  }
  if (sbe->wasAttempted()) {
    return false;
  }
  if (!sbe->getSourceFileList()->contains(filename)) {
    return false;
  }
  if (sbe->getDestinationFileList()->contains(filename)) {
    return false;
  }
  return true;
}

void Engine::deleteOnAllSites(const std::shared_ptr<Race> & race, bool allfiles, bool deleteoncomplete) {
  std::list<std::shared_ptr<Site> > sites;
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
    if (deleteoncomplete || isIncompleteEnoughForDelete(race, it->first)) {
      sites.push_back(it->second->getSite());
    }
  }
  deleteOnSites(race, sites, allfiles);
}

bool Engine::isIncompleteEnoughForDelete(const std::shared_ptr<Race> & race, const std::shared_ptr<SiteRace> & siterace) const {
  return siterace->getStatus() != RaceStatus::DONE &&
         (!race->estimatedTotalSize() || (siterace->getTotalFileSize() * 100) / race->estimatedTotalSize() < MAX_PERCENTAGE_FOR_INCOMPLETE_DELETE);
}

void Engine::deleteOnSites(const std::shared_ptr<Race> & race, std::list<std::shared_ptr<Site> > delsites, bool allfiles) {
  if (!!race) {
    if (race->getStatus() == RaceStatus::RUNNING) {
      abortRace(race);
    }
    std::string sites;
    for (std::list<std::shared_ptr<Site> >::const_iterator it = delsites.begin(); it != delsites.end(); it++) {
      if (!*it) {
        continue;
      }
      std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic((*it)->getName());
      if (!sl) {
        continue;
      }
      std::shared_ptr<SiteRace> sr = race->getSiteRace((*it)->getName());
      if (!sr) {
        global->getEventLog()->log("Engine", "Site " + (*it)->getName() + " is not in spread job: " + race->getName());
        continue;
      }
      const Path & path = sr->getPath();
      sl->requestDelete(nullptr, path, true, allfiles);
      sites += (*it)->getName() + ",";
    }
    if (sites.length() > 0) {
      sites = sites.substr(0, sites.length() - 1);
      global->getEventLog()->log("Engine", std::string("Attempting delete of ") + (allfiles ? "all" : "own") + " files in " + race->getName() + " on: " + sites);
    }
  }
}

void Engine::abortTransferJob(const std::shared_ptr<TransferJob>& tj) {
  tj->abort();
  currenttransferjobs.remove(tj);
  global->getEventLog()->log("Engine", "Transfer job aborted: " + tj->getSrcFileName());
  rotateTransferJobsHistory();
}

void Engine::resetTransferJob(const std::shared_ptr<TransferJob>& tj) {
  tj->reset();
  bool current = false;
  for (const std::shared_ptr<TransferJob>& currenttj : currenttransferjobs) {
    if (currenttj == tj) {
      current = true;
      break;
    }
  }
  if (!current) {
    currenttransferjobs.push_back(tj);
  }
  if (tj->getSrc()) {
    tj->getSrc()->activateOne();
  }
  if (tj->getDst()) {
    tj->getDst()->activateOne();
  }
  checkStartPoke();
  global->getEventLog()->log("Engine", "Transfer job reset: " + tj->getSrcFileName());
}

void Engine::jobFileListRefreshed(SiteLogic * sls, const std::shared_ptr<CommandOwner> & commandowner, const std::shared_ptr<FileList>& fl) {
  switch (commandowner->classType()) {
    case COMMANDOWNER_SITERACE: {
      std::shared_ptr<SiteRace> sr = std::static_pointer_cast<SiteRace>(commandowner);
      if (sr->isDone()) {
        break;
      }
      std::shared_ptr<Race> race = sr->getRace().lock();
      if (!race) {
        break;
      }
      bool addedtoboard = false;
      std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it;
      std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(sls);
      if (!sl) {
        break;
      }
      for (it = sr->fileListsBegin(); it != sr->fileListsEnd(); it++) {
        const std::shared_ptr<FileList>& itfl = it->second;
        if (!itfl->inScoreBoard() &&
            itfl->getState() != FileListState::FAILED &&
            (itfl == fl || itfl->getState() == FileListState::NONEXISTENT))
        {
          addedtoboard = true;
          addToScoreBoard(itfl, sr, sl);
        }
      }
      if (fl->getScoreBoardUpdateState() == UpdateState::CHANGED) {
        spreadjobfilelistschanged[fl] = std::make_pair(sr, sl);
      }
      if (!global->getWorkManager()->overload() || forcescoreboard) {
        forcescoreboard = false;
        updateScoreBoard();
        checkIfRaceComplete(sl, race);
      }
      else {
        if (addedtoboard) {
          scoreboard->sort();
          scoreboard->shuffleEquals();
        }
        ++dropped;
      }
      break;
    }
    case COMMANDOWNER_TRANSFERJOB: {
      std::shared_ptr<TransferJob> tj = std::static_pointer_cast<SiteTransferJob>(commandowner)->getTransferJob().lock();
      assert(tj);
      refreshPendingTransferList(tj);
      break;
    }
  }
}

bool Engine::transferJobActionRequest(const std::shared_ptr<SiteTransferJob> & stj) {
  std::shared_ptr<TransferJob> tj = stj->getTransferJob().lock();
  assert(tj);
  if (tj->getType() == TRANSFERJOB_FXP && stj->otherWantsList()) {
    stj->getOtherSiteLogic()->haveConnectedActivate(1);
    return false;
  }
  std::unordered_map<std::shared_ptr<TransferJob>, std::list<PendingTransfer> >::iterator it = pendingtransfers.find(tj);
  if (it == pendingtransfers.end()) {
    pendingtransfers[tj] = std::list<PendingTransfer>();
    it = pendingtransfers.find(tj);
  }
  refreshPendingTransferList(tj);
  if (it->second.size() == 0) {
    bool action = tj->refreshOrAlmostDone();
    if (tj->getType() == TRANSFERJOB_FXP && stj->otherWantsList()) {
      stj->getOtherSiteLogic()->haveConnectedActivate(1);
      return false;
    }
    return action;
  }
  tj->clearRefreshLists();
  bool started = false;
  while (!it->second.empty() && !started) {
    PendingTransfer pt = it->second.front();
    switch (pt.type()) {
      case PENDINGTRANSFER_DOWNLOAD:
      {
        if (!pt.getSrc()->downloadSlotAvailable(TransferType::TRANSFERJOB)) {
          return false;
        }
        std::shared_ptr<TransferStatus> ts = global->getTransferManager()->attemptDownload(pt.getSrcFileName(),
            pt.getSrc(), pt.getSrcFileList(), pt.getLocalFileList(), tj->getSrcTransferJob());
        if (!!ts) {
          tj->addTransfer(ts);
          started = true;
        }
        break;
      }
      case PENDINGTRANSFER_UPLOAD:
      {
        if (!pt.getDst()->uploadSlotAvailable()) return false;
        std::shared_ptr<TransferStatus> ts = global->getTransferManager()->attemptUpload(pt.getSrcFileName(),
            pt.getLocalFileList(), pt.getDst(), pt.getDstFileList(), tj->getDstTransferJob());
        if (!!ts) {
          tj->addTransfer(ts);
          started = true;
        }
        break;
      }
      case PENDINGTRANSFER_FXP:
      {
        if (!pt.getSrc()->downloadSlotAvailable(TransferType::TRANSFERJOB)) {
          return false;
        }
        if (!pt.getDst()->uploadSlotAvailable()) {
          return false;
        }
        if (pt.getDst() == pt.getSrc() && pt.getDst()->slotsAvailable() < 2) {
          pt.getDst()->haveConnectedActivate(2);
          return false;
        }
        std::shared_ptr<TransferStatus> ts = global->getTransferManager()->attemptTransfer(pt.getSrcFileName(),
            pt.getSrc(), pt.getSrcFileList(), pt.getDst(), pt.getDstFileList(), tj->getSrcTransferJob(), tj->getDstTransferJob());
        if (!!ts) {
          tj->addTransfer(ts);
          started = true;
        }
        break;
      }
    }
  }
  if (tj->getStatus() == TRANSFERJOB_QUEUED) {
    tj->start();
  }
  return true;
}

void Engine::raceActionRequest() {
  issueOptimalTransfers();
}

void Engine::estimateRaceSizes() {
  for (std::list<std::shared_ptr<Race> >::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    estimateRaceSize(*itr, true);
  }
}

void Engine::estimateRaceSize(const std::shared_ptr<Race> & race, bool forceupdate) {
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator its = race->begin(); its != race->end(); its++) {
    const std::shared_ptr<SiteRace> & srs = its->first;
    const SkipList & siteskiplist = its->second->getSite()->getSkipList();
    const SkipList & sectionskiplist = race->getSectionSkipList();
    std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator itfl;
    for (itfl = srs->fileListsBegin(); itfl != srs->fileListsEnd(); itfl++) {
      const std::shared_ptr<FileList>& fls = itfl->second;
      if (!forceupdate && srs->sizeEstimated(fls)) {
        continue;
      }

      if (fls->hasExtension("sfv")) {
        if (srs->getSFVObservedTime(fls) > SFVDIROBSERVETIME) {
          reportCurrentSize(siteskiplist, sectionskiplist, srs, fls, true);
          continue;
        }
      }
      else {
        if (srs->getObservedTime(fls) > DIROBSERVETIME) {
          reportCurrentSize(siteskiplist, sectionskiplist, srs, fls, true);
          continue;
        }
      }
      reportCurrentSize(siteskiplist, sectionskiplist, srs, fls, false);
    }
  }
}

bool setContainsPattern(const std::unordered_set<std::string> & uniques, const SkipListMatch& match) {
  std::string path = Path(match.matchedpath).dirName();
  if (match.regex) {
    if (path.empty()) {
      for (const std::string & unique : uniques) {
        if (std::regex_match(unique, match.matchregexpattern)) {
          return true;
        }
      }
    }
    else {
      for (const std::string & unique : uniques) {
        if (std::regex_match(path + "/" + unique, match.matchregexpattern)) {
          return true;
        }
      }
    }
  }
  else {
    if (path.empty()) {
      for (const std::string & unique : uniques) {
        if (util::wildcmp(match.matchpattern.c_str(), unique.c_str())) {
          return true;
        }
      }
    }
    else {
      for (const std::string & unique : uniques) {
        if (util::wildcmp(match.matchpattern.c_str(), (path + "/" + unique).c_str())) {
          return true;
        }
      }
    }
  }
  return false;
}

void Engine::reportCurrentSize(const SkipList & siteskiplist, const SkipList & sectionskiplist, const std::shared_ptr<SiteRace> & srs, const std::shared_ptr<FileList>& fls, bool final) {
  std::unordered_set<std::string> uniques;
  std::list<File *>::const_iterator itf;
  std::string subpath = srs->getSubPathForFileList(fls);
  bool similar = false;
  std::string firstsimilar;
  for (itf = fls->begin(); itf != fls->end(); itf++) {
    File * file = *itf;
    if (file->isDirectory()) {
      continue;
    }
    std::string filename = file->getName();
    size_t lastdotpos = filename.rfind(".");
    if (lastdotpos != std::string::npos && filename.length() > 8 &&
        lastdotpos == filename.length() - 8 && filename.substr(lastdotpos) == ".missing")
    {
      filename = filename.substr(0, lastdotpos); // special hack for some zipscripts
      lastdotpos = filename.rfind(".");
    }
    if (lastdotpos != std::string::npos) {
      size_t len = filename.length();
      size_t checkpos = lastdotpos + 1;
      while (checkpos < len) {
        if (isalnum(filename[checkpos])) {
          ++checkpos;
        }
        else {
          filename = filename.substr(0, checkpos);
          break;
        }
      }
    }
    Path prepend = subpath;
    SkipListMatch match = siteskiplist.check((prepend / filename).toString(), false, true, &sectionskiplist);
    if (match.action == SKIPLIST_DENY ||
        (match.action == SKIPLIST_UNIQUE && setContainsPattern(uniques, match)))
    {
      continue;
    }
    if (match.action == SKIPLIST_SIMILAR) {
      if (!similar) {
        firstsimilar = filename;
        similar = true;
      }
      else if (FileList::checkUnsimilar(filename, firstsimilar)) {
        continue;
      }
    }
    uniques.insert(filename);
  }
  srs->reportSize(fls, uniques, final);
}

void Engine::addToScoreBoard(const std::shared_ptr<FileList>& fl, const std::shared_ptr<SiteRace> & sr, const std::shared_ptr<SiteLogic> & sl) {
  assert(fl->getState() != FileListState::FAILED);
  const std::shared_ptr<Site> & site = sl->getSite();
  const SkipList & skip = site->getSkipList();
  std::string subpath = sr->getSubPathForFileList(fl);
  Path subpathpath(subpath);
  std::shared_ptr<Race> race = sr->getRace().lock();
  if (!race) {
    return;
  }
  bool racemode = race->getProfile() == SPREAD_RACE;
  SitePriority priority = site->getPriority();
  const SkipList & secskip = race->getSectionSkipList();
  bool flskip = false;
  if (!subpath.empty()) {
    SkipListMatch dirmatch = skip.check(subpath, true, true, &secskip);
    if (dirmatch.action == SKIPLIST_DENY ||
        (dirmatch.action == SKIPLIST_UNIQUE &&
         containsPatternBefore(sr->getFileListForPath(""), dirmatch, subpath)))
    {
      flskip = true;
    }
  }
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator cmpit = race->begin(); cmpit != race->end(); cmpit++) {
    const std::shared_ptr<SiteLogic> & cmpsl = cmpit->second;
    const std::shared_ptr<Site> & cmpsite = cmpsl->getSite();
    const SkipList & cmpskip = cmpsite->getSkipList();
    const std::shared_ptr<SiteRace> & cmpsr = cmpit->first;
    std::shared_ptr<FileList> cmpfl = cmpsr->getFileListForPath(subpath);
    if (!cmpfl) {
      cmpsr->addSubDirectory(subpath);
      continue;
    }
    SitePriority cmppriority = cmpsite->getPriority();
    if (raceTransferPossible(cmpsl, cmpsr, sl, sr, race) && !flskip) {
      addToScoreBoardForPair(cmpsl, cmpsite, cmpsr, cmpfl, sl, site, sr, fl, skip, secskip, race, subpathpath, priority, racemode);
    }
    if (fl->getNumUploadedFiles() && raceTransferPossible(sl, sr, cmpsl, cmpsr, race)) {
      addToScoreBoardForPair(sl, site, sr, fl, cmpsl, cmpsite, cmpsr, cmpfl, cmpskip, secskip, race, subpathpath, cmppriority, racemode);
    }
  }
  fl->setInScoreBoard();
}

void Engine::addToScoreBoardForPair(const std::shared_ptr<SiteLogic> & sls, const std::shared_ptr<Site> & ss, const std::shared_ptr<SiteRace> & srs,
                            const std::shared_ptr<FileList>& fls, const std::shared_ptr<SiteLogic> & sld, const std::shared_ptr<Site> & ds,
                            const std::shared_ptr<SiteRace> & srd, const std::shared_ptr<FileList>& fld, const SkipList & dstskip,
                            const SkipList & secskip,
                            const std::shared_ptr<Race> & race, const Path & subpath, SitePriority priority,
                            bool racemode)
{
  if (fld->getState() == FileListState::UNKNOWN || fld->getState() == FileListState::PRE_FAIL || fld->getState() == FileListState::FAILED) {
    return;
  }
  int avgspeed = ss->getAverageSpeed(ds->getName());
  if (avgspeed > maxavgspeed) {
    avgspeed = maxavgspeed;
  }
  std::list<File *>::const_iterator itf;
  for (itf = fls->begin(); itf != fls->end(); itf++) {
    File * f = *itf;
    const std::string & name = f->getName();
    if (fld->contains(name) || f->isDirectory() || f->getSize() == 0) {
      continue;
    }
    SkipListMatch filematch = dstskip.check((subpath / name).toString(), false, true, &secskip);
    if (filematch.action == SKIPLIST_DENY ||
        (filematch.action == SKIPLIST_UNIQUE && containsPattern(fld, filematch, false)))
    {
      continue;
    }
    if (filematch.action == SKIPLIST_SIMILAR) {
      if (!fld->similarChecked()) {
        fld->checkSimilar(&dstskip, &secskip);
      }
      if (fld->containsUnsimilar(name)) {
        continue;
      }
    }
    PrioType p = getPrioType(f);
    unsigned long long int filesize = f->getSize();
    unsigned short score = calculateScore(p, filesize, race, fls, srs, fld, srd, avgspeed, priority, racemode);
    scoreboard->update(name, score, filesize, p, sls, fls, srs, sld, fld, srd, race, subpath.toString());
    race->resetUpdateCheckCounter();
  }
}

void Engine::updateScoreBoard() {
  for (const std::pair<const std::shared_ptr<FileList>, std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic>>> & changedlist : spreadjobfilelistschanged) {
    const std::shared_ptr<FileList>& fl = changedlist.first;
    const std::shared_ptr<SiteRace>& sr = changedlist.second.first;
    const std::shared_ptr<SiteLogic>& sl = changedlist.second.second;
    const std::shared_ptr<Site>& site = sl->getSite();
    const SkipList& skip = site->getSkipList();
    std::string subpath = sr->getSubPathForFileList(fl);
    Path subpathpath(subpath);
    std::shared_ptr<Race> race = sr->getRace().lock();
    if (!race) {
      continue;
    }
    bool racemode = race->getProfile() == SPREAD_RACE;
    const SkipList & secskip = race->getSectionSkipList();
    if (fl->getState() == FileListState::FAILED) {
      scoreboard->wipe(fl);
      failboard->wipe(fl);
      fl->unsetInScoreBoard();
      continue;
    }
    for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator cmpit = race->begin(); cmpit != race->end(); cmpit++) {
      const std::shared_ptr<SiteLogic> & cmpsl = cmpit->second;
      const std::shared_ptr<Site> & cmpsite = cmpsl->getSite();
      const SkipList & cmpskip = cmpsite->getSkipList();
      const std::shared_ptr<SiteRace> & cmpsr = cmpit->first;
      bool regulartransferpossible = raceTransferPossible(sl, sr, cmpsl, cmpsr, race);
      std::shared_ptr<FileList> cmprootfl = cmpsr->getFileListForPath("");
      if (!subpath.empty()) {
        SkipListMatch dirmatch = cmpskip.check(subpath, true, true, &secskip);
        if (dirmatch.action == SKIPLIST_DENY ||
            (dirmatch.action == SKIPLIST_UNIQUE &&
             containsPatternBefore(cmprootfl, dirmatch, subpath)))
        {
          continue;
        }
      }
      std::shared_ptr<FileList> cmpfl = cmpsr->getFileListForPath(subpath);
      if (!cmpfl) {
        cmpsr->addSubDirectory(subpath);
        continue;
      }
      FileListState cmpstate = cmpfl->getState();
      if (cmpstate == FileListState::UNKNOWN) {
        continue;
      }
      bool cmpfailed = cmpstate == FileListState::FAILED;
      int avgspeed = site->getAverageSpeed(cmpsite->getName());
      if (avgspeed > maxavgspeed) {
        avgspeed = maxavgspeed;
      }
      SitePriority cmppriority = cmpsite->getPriority();
      for (std::unordered_set<std::string>::const_iterator it = fl->scoreBoardChangedFilesBegin(); it != fl->scoreBoardChangedFilesEnd(); it++) {
        const std::string & name = *it;
        File * f = fl->getFile(name);
        if (f == nullptr) { // special case when file has been deleted, reverse transfer from cmp->changed
          scoreboard->remove(name, fl, cmpfl);
          failboard->remove(name, fl, cmpfl);
          if (cmpfailed || !raceTransferPossible(cmpsl, cmpsr, sl, sr, race)) {
            continue;
          }
          f = cmpfl->getFile(name);
          if (f == nullptr) {
            continue;
          }
          if (!subpath.empty()) {
            SkipListMatch dirmatch = skip.check(subpath, true, true, &secskip);
            if (dirmatch.action == SKIPLIST_DENY ||
                (dirmatch.action == SKIPLIST_UNIQUE &&
                 containsPatternBefore(sr->getFileListForPath(""), dirmatch, subpath)))
            {
              continue;
            }
          }
          if (fl->contains(name) || f->isDirectory() || f->getSize() == 0) {
            continue;
          }
          SkipListMatch filematch = skip.check((subpathpath / name).toString(), false, true, &secskip);
          if (filematch.action == SKIPLIST_DENY || (filematch.action == SKIPLIST_UNIQUE &&
                                                    containsPattern(fl, filematch, false)))
          {
            continue;
          }
          if (filematch.action == SKIPLIST_SIMILAR) {
            if (!fl->similarChecked()) {
              fl->checkSimilar(&skip, &secskip);
            }
            if (fl->containsUnsimilar(name)) {
              continue;
            }
          }
          SitePriority priority = site->getPriority();
          avgspeed = cmpsite->getAverageSpeed(site->getName());
          if (avgspeed > maxavgspeed) {
            avgspeed = maxavgspeed;
          }
          PrioType p = getPrioType(f);
          unsigned long long int filesize = f->getSize();
          unsigned short score = calculateScore(p, filesize, race, cmpfl, cmpsr, fl, sr, avgspeed, priority, racemode);
          std::shared_ptr<ScoreBoard> & updateboard = (race->hasFailedTransfer(name, cmpfl, fl)) ? failboard : scoreboard;
          updateboard->update(name, score, filesize, p, cmpsl, cmpfl, cmpsr, sl, fl, sr, race, subpath);
          race->resetUpdateCheckCounter();
          continue;
        }
        if (scoreboard->remove(name, cmpfl, fl)) {
          scoreboard->resetSkipChecked(fl);
        }
        failboard->remove(name, cmpfl, fl);
        if (cmpfailed || cmpfl->contains(name) || !regulartransferpossible || f->isDirectory() || f->getSize() == 0) {
          continue;
        }
        SkipListMatch filematch = cmpskip.check((subpathpath / name).toString(), false, true, &secskip);
        if (filematch.action == SKIPLIST_DENY || (filematch.action == SKIPLIST_UNIQUE &&
                                                  containsPattern(cmpfl, filematch, false)))
        {
          continue;
        }
        if (filematch.action == SKIPLIST_SIMILAR) {
          if (!cmpfl->similarChecked()) {
            cmpfl->checkSimilar(&cmpskip, &secskip);
          }
          if (cmpfl->containsUnsimilar(name)) {
            continue;
          }
        }
        PrioType p = getPrioType(f);
        unsigned long long int filesize = f->getSize();
        unsigned short score = calculateScore(p, filesize, race, fl, sr, cmpfl, cmpsr, avgspeed, cmppriority, racemode);
        std::shared_ptr<ScoreBoard> & updateboard = (race->hasFailedTransfer(name, fl, cmpfl)) ? failboard : scoreboard;
        updateboard->update(name, score, filesize, p, sl, fl, sr, cmpsl, cmpfl, cmpsr, race, subpath);
        race->resetUpdateCheckCounter();
      }
    }
    fl->resetScoreBoardUpdates();
  }
  spreadjobfilelistschanged.clear();
  scoreboard->sort();
  scoreboard->shuffleEquals();
}

void Engine::refreshScoreBoard() {
  std::vector<ScoreBoardElement *>::iterator it;
  for (it = scoreboard->begin(); it != scoreboard->end(); ++it) {
    ScoreBoardElement * sbe = *it;
    sbe->update(calculateScore(sbe));
  }
  scoreboard->sort();
  scoreboard->shuffleEquals();
}

void Engine::refreshPendingTransferList(const std::shared_ptr<TransferJob>& tj) {
  std::unordered_map<std::shared_ptr<TransferJob>, std::list<PendingTransfer> >::iterator it = pendingtransfers.find(tj);
  if (it == pendingtransfers.end()) {
    pendingtransfers[tj] = std::list<PendingTransfer>();
  }
  it = pendingtransfers.find(tj);
  std::list<PendingTransfer> & list = it->second;
  list.clear();
  tj->clearExisting();
  switch (tj->getType()) {
    case TRANSFERJOB_DOWNLOAD: {
      std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it2;
      for (it2 = tj->srcFileListsBegin(); it2 != tj->srcFileListsEnd(); it2++) {
        const std::shared_ptr<FileList>& srclist = it2->second;
        std::shared_ptr<LocalFileList> dstlist = tj->findLocalFileList(it2->first);
        for (std::list<File *>::iterator srcit = srclist->begin(); srcit != srclist->end(); srcit++) {
          File * f = *srcit;
          const std::string & filename = f->getName();
          bool onefilejob = it2->first == "" && !f->isDirectory() && filename == tj->getSrcFileName();
          if ((it2->first != "" || onefilejob) &&
              !f->isDirectory() && f->getSize() > 0)
          {
            std::unordered_map<std::string, LocalFile>::const_iterator dstit;
            if (!dstlist) {
              dstlist = tj->wantedLocalDstList(it2->first);
            }
            dstit = dstlist->find(filename);
            if (tj->hasFailedTransfer((Path(dstlist->getPath()) / filename).toString())) {
              continue;
            }
            const Path subpath = it2->first;
            if (!dstlist || dstit == dstlist->end() || dstit->second.getSize() == 0) {
              if (!onefilejob) {
                SkipListMatch match = global->getSkipList()->check((dstlist->getPath() / filename).toString(), false, false);
                if (match.action == SKIPLIST_DENY) {
                  continue;
                }
              }
              PendingTransfer p(tj->getSrc(), srclist, filename, dstlist, filename);
              addPendingTransfer(list, p);
              tj->addPendingTransfer(subpath / filename, f->getSize());
            }
            else {
              tj->targetExists(subpath / filename);
            }
          }
        }
      }
      break;
    }
    case TRANSFERJOB_UPLOAD: {
      const SkipList& dstskip = &tj->getDst()->getSite()->getSkipList();
      std::unordered_map<std::string, std::shared_ptr<LocalFileList> >::const_iterator lit;
      for (lit = tj->localFileListsBegin(); lit != tj->localFileListsEnd(); lit++) {
        std::shared_ptr<FileList> dstlist = tj->findDstList(lit->first);
        if (dstlist == NULL || dstlist->getState() == FileListState::UNKNOWN || dstlist->getState() == FileListState::EXISTS) {
          continue;
        }
        for (std::unordered_map<std::string, LocalFile>::const_iterator lfit = lit->second->begin(); lfit != lit->second->end(); lfit++) {
          bool onefilejob = lit->first == "" && !lfit->second.isDirectory() && lfit->first == tj->getSrcFileName();
          if ((lit->first != "" || onefilejob) &&
              !lfit->second.isDirectory() && lfit->second.getSize() > 0)
          {
            std::string filename = lfit->first;
            const Path subpath = lit->first;
            if (dstlist->getFile(filename) == NULL) {
              if (tj->hasFailedTransfer((Path(dstlist->getPath()) / filename).toString())) {
                continue;
              }
              if (!onefilejob) {
                SkipListMatch filematch = dstskip.check((dstlist->getPath() / filename).toString(), false, false, tj->getDstSectionSkipList());
                if (filematch.action == SKIPLIST_DENY ||
                    (filematch.action == SKIPLIST_UNIQUE && containsPattern(dstlist, filematch, false)))
                {
                  continue;
                }
                if (filematch.action == SKIPLIST_SIMILAR) {
                  if (!dstlist->similarChecked()) {
                    dstlist->checkSimilar(&dstskip);
                  }
                  if (dstlist->containsUnsimilar(filename)) {
                    continue;
                  }
                }
              }
              PendingTransfer p(lit->second, filename, tj->getDst(), dstlist, filename);
              addPendingTransfer(list, p);
              tj->addPendingTransfer(subpath / filename, lfit->second.getSize());
            }
            else {
              tj->targetExists(subpath / filename);
            }
          }
        }
      }
      break;
    }
    case TRANSFERJOB_FXP: {
      const SkipList& dstskip = &tj->getDst()->getSite()->getSkipList();
      std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it2;
      for (it2 = tj->srcFileListsBegin(); it2 != tj->srcFileListsEnd(); it2++) {
        const std::shared_ptr<FileList>& srclist = it2->second;
        std::shared_ptr<FileList> dstlist = tj->findDstList(it2->first);
        if (dstlist == NULL || dstlist->getState() == FileListState::UNKNOWN || dstlist->getState() == FileListState::EXISTS) {
          continue;
        }
        for (std::list<File *>::iterator srcit = srclist->begin(); srcit != srclist->end(); srcit++) {
          File * f = *srcit;
          const std::string & filename = f->getName();
          bool onefilejob = it2->first == "" && !f->isDirectory() && filename == tj->getSrcFileName();
          if ((it2->first != "" || onefilejob) &&
              !f->isDirectory() && f->getSize() > 0)
          {
            const Path subpath = it2->first;
            if (dstlist->getFile(filename) == NULL) {
              if (tj->hasFailedTransfer((Path(dstlist->getPath()) / filename).toString())) {
                continue;
              }
              if (!onefilejob) {
                SkipListMatch filematch = dstskip.check((dstlist->getPath() / filename).toString(), false, false, tj->getDstSectionSkipList());
                if (filematch.action == SKIPLIST_DENY ||
                   (filematch.action == SKIPLIST_UNIQUE && containsPattern(dstlist, filematch, false)))
                {
                 continue;
                }
                if (filematch.action == SKIPLIST_SIMILAR) {
                  if (!dstlist->similarChecked()) {
                    dstlist->checkSimilar(&dstskip);
                  }
                  if (dstlist->containsUnsimilar(filename)) {
                    continue;
                  }
                }
              }
              PendingTransfer p(tj->getSrc(), srclist, filename, tj->getDst(), dstlist, filename);
              addPendingTransfer(list, p);

              tj->addPendingTransfer(subpath / filename, f->getSize());
            }
            else {
              tj->targetExists(subpath / filename);
            }
          }
        }
      }
      break;
    }
  }
  tj->updateStatus();
}

void Engine::addPendingTransfer(std::list<PendingTransfer> & list, PendingTransfer & p) {
  std::string extension = File::getExtension(p.getSrcFileName());
  if (extension == "sfv" || extension == "nfo") {
    list.push_front(p); // sfv and nfo files have top priority
  }
  else {
    list.push_back(p);
  }
}

void Engine::issueOptimalTransfers() {
  std::vector<ScoreBoardElement *>::const_iterator it;
  ScoreBoardElement * sbe;
  std::shared_ptr<SiteLogic> sls;
  std::shared_ptr<SiteLogic> sld;
  std::string filename;
  std::shared_ptr<Race> race;
  std::list<std::tuple<std::string, std::shared_ptr<FileList>, std::shared_ptr<FileList>>> removelist;
  for (it = scoreboard->begin(); it != scoreboard->end(); it++) {
    sbe = *it;
    sls = sbe->getSource();
    sld = sbe->getDestination();
    race = sbe->getRace();
    filename = sbe->fileName();
    if (!sld->uploadSlotAvailable()) {
      continue;
    }
    if (!sbe->skipChecked()) {
      SkipListMatch match = sbe->getDestination()->getSite()->getSkipList().check(
          (Path(sbe->subDir()) / filename).toString(), false, true, &sbe->getRace()->getSectionSkipList());
      if (match.action == SKIPLIST_SIMILAR) {
        if (!sbe->getDestinationFileList()->similarChecked()) {
          sbe->getDestinationFileList()->checkSimilar(&sbe->getDestination()->getSite()->getSkipList(),
                                                      &sbe->getRace()->getSectionSkipList());
        }
      }
      bool remove = false;
      if (match.action == SKIPLIST_UNIQUE) {
        if (containsPattern(sbe->getDestinationFileList(), match, false)) {
          remove = true;
        }
      }
      else {
        sbe->setSkipChecked();
      }
      if (match.action == SKIPLIST_SIMILAR && sbe->getDestinationFileList()->containsUnsimilar(filename))
      {
        remove = true;
      }
      if (remove) {
        removelist.emplace_back(sbe->fileName(), sbe->getSourceFileList(),
                                        sbe->getDestinationFileList());
        continue;
      }
    }
    if (!transferExpectedSoon(sbe)) {
      continue;
    }
    //potentiality handling
    if (sbe->getPriorityType() == PrioType::NORMAL) { // priority files shouldn't affect the potential tracking
      sls->pushPotential(sbe->getScore(), filename, sld);
    }
    if (!sls->potentialCheck(sbe->getScore())) {
      continue;
    }
    TransferType type(TransferType::REGULAR);
    if (sbe->getSourceSiteRace()->isDownloadOnly()) {
      type = TransferType::PRE;
    }
    else if (sbe->getSourceSiteRace()->isDone()) {
      type = TransferType::COMPLETE;
    }
    if (!sls->downloadSlotAvailable(type)) {
      continue;
    }
    std::shared_ptr<TransferStatus> ts =
      global->getTransferManager()->attemptTransfer(filename, sls,
        sbe->getSourceFileList(), sld, sbe->getDestinationFileList(),
        sbe->getSourceSiteRace(), sbe->getDestinationSiteRace());
    if (!!ts) {
      race->addTransfer(ts);
    }
    sbe->setAttempted();
  }
  for (const std::tuple<std::string, std::shared_ptr<FileList>, std::shared_ptr<FileList>> & elem : removelist) {
    scoreboard->remove(std::get<0>(elem), std::get<1>(elem), std::get<2>(elem));
  }
  if (!removelist.empty()) {
    scoreboard->sort();
    scoreboard->shuffleEquals();
  }
}

void Engine::transferFailed(const std::shared_ptr<TransferStatus> & ts, int err) {
  TransferStatusCallback * cb = ts->getCallback();
  if (cb && cb->callbackType() == CallbackType::RACE &&
      static_cast<Race *>(cb)->hasFailedTransfer(ts->getFile(), ts->getSourceFileList(), ts->getTargetFileList()))
  {
    ScoreBoardElement * sbe = scoreboard->find(ts->getFile(), ts->getSourceFileList(), ts->getTargetFileList());
    if (sbe) {
      failboard->update(sbe);
      scoreboard->remove(sbe);
      scoreboard->sort();
      scoreboard->shuffleEquals();
    }
  }
}

void Engine::checkIfRaceComplete(const std::shared_ptr<SiteLogic> & sls, std::shared_ptr<Race> & race) {
  std::shared_ptr<SiteRace> srs = sls->getRace(race->getName());
  if (!srs->isDone()) {
    bool unfinisheddirs = false;
    bool emptydirs = false;
    int completedlists = 0;
    std::unordered_set<std::string> subpaths = race->getSubPaths();
    for (std::unordered_set<std::string>::iterator itsp = subpaths.begin(); itsp != subpaths.end(); itsp++) {
      std::shared_ptr<FileList> spfl = srs->getFileListForPath(*itsp);
      if (!!spfl && spfl->getState() == FileListState::LISTED) {
        if (!race->sizeEstimated(*itsp)) {
          if (spfl->getSize() > 0) {
            unfinisheddirs = true;
            continue;
          }
          emptydirs = true;
        }
        else if (spfl->getNumUploadedFiles() >= race->estimatedSize(*itsp) &&
          spfl->timeSinceLastChanged() > STATICTIMEFORCOMPLETION &&
          sls->getCurrLogins() > sls->getCurrUp() + sls->getCurrDown() &&
          !spfl->hasFilesUploading())
        {
          completedlists++;
          if (!srs->isSubPathComplete(spfl)) {
            srs->subPathComplete(spfl);
          }
        }
        else {
          unfinisheddirs = true;
          continue;
        }
      }
      else {
        unfinisheddirs = true;
        continue;
      }
    }
    if (completedlists > 0 && !unfinisheddirs) {
      if (emptydirs) {
        race->reportSemiDone(srs);
      }
      else {
        int uploadslotcount = 0;
        for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = race->begin(); it != race->end(); it++) {
          if (it->second != sls && !it->first->isDone()) {
            uploadslotcount += it->second->getSite()->getMaxUp();
          }
        }
        sls->raceLocalComplete(srs, uploadslotcount);
        global->getEventLog()->log("Engine", "Spread job " + race->getName() + " completed on " +
            sls->getSite()->getName());
      }
      if (race->isDone()) {
        raceComplete(race);
      }
    }
  }
}

void Engine::raceComplete(const std::shared_ptr<Race>& race) {
  std::list<std::shared_ptr<Race> >::iterator it = currentraces.find(race);
  if (it != currentraces.end()) {
    currentraces.erase(it);
    finishedraces.push_back(race);
  }
  issueGlobalComplete(race);
  global->getEventLog()->log("Engine", "Spread job globally completed: " + race->getName());
  if (dropped) {
    global->getEventLog()->log("Engine", "Scoreboard refreshes dropped since spread job start: " + std::to_string(dropped));
  }
  return;
}

void Engine::transferJobComplete(const std::shared_ptr<TransferJob>& tj) {
  global->getEventLog()->log("Engine", tj->typeString() + " job complete: " + tj->getSrcFileName());
  currenttransferjobs.erase(tj);
  rotateTransferJobsHistory();
}

unsigned short Engine::calculateScore(ScoreBoardElement * sbe) const {
  const std::shared_ptr<Race> & race = sbe->getRace();
  SitePriority priority = sbe->getDestination()->getSite()->getPriority();
  int avgspeed = sbe->getSource()->getSite()->getAverageSpeed(sbe->getDestination()->getSite()->getName());
  return calculateScore(sbe->getPriorityType(), sbe->getFileSize(), race, sbe->getSourceFileList(), sbe->getSourceSiteRace(),
      sbe->getDestinationFileList(), sbe->getDestinationSiteRace(), avgspeed, priority,
      race->getProfile() == SPREAD_RACE);
}

unsigned short Engine::calculateScore(PrioType priotype, unsigned long long int filesize, const std::shared_ptr<Race> & itr, const std::shared_ptr<FileList>& fls, const std::shared_ptr<SiteRace> & srs,
                                      const std::shared_ptr<FileList>& fld, const std::shared_ptr<SiteRace> & srd, int avgspeed,
                                      SitePriority priority, bool racemode) const
{
  switch (priotype) {
    case PrioType::PRIO:
      return 65535;
    case PrioType::PRIO_LATER:
      if (itr->getTimeSpent() > NFO_PRIO_AFTER_SEC) {
        return 65535;
      }
      break;
    case PrioType::NORMAL:
      break;
  }
  unsigned short points = 0;
  unsigned long long int maxfilesize = srs->getMaxFileSize();
  if (avgspeed > maxavgspeed) {
    avgspeed = maxavgspeed;
  }
  if (filesize > maxfilesize) {
    maxfilesize = filesize;
  }

  if (maxfilesize) {
    unsigned long long int pointsfilesize = maxpointsfilesize;
    pointsfilesize *= filesize;
    pointsfilesize /= maxfilesize;
    points += pointsfilesize;
  }

  points += getSpeedPoints(avgspeed);

  if (racemode) {
    if (priority >= SitePriority::VERY_HIGH) {
      points += maxpointspercentageowned;
    }
    else {
      unsigned long long int pointspercentageowned = maxpointspercentageowned;
      int unownedpercentage = 100 - fld->getOwnedPercentage();
      pointspercentageowned *= unownedpercentage;
      pointspercentageowned /= 100;
      points += pointspercentageowned;
    }
  }
  else {
    if (priority >= SitePriority::VERY_HIGH) {
      points += maxpointslowprogress;
    }
    else {
      unsigned long long int pointslowprogress = maxpointslowprogress;
      int maxprogress = itr->getMaxSiteNumFilesProgress();
      if (maxprogress) {
        pointslowprogress *= fld->getNumUploadedFiles();
        pointslowprogress /= maxprogress;
        points += pointslowprogress;
      }
    }
  }

  points += getPriorityPoints(priority);
  return points;
}

void Engine::setSpeedScale() {
  maxavgspeed = 1024;
  for (std::list<std::shared_ptr<Race> >::iterator itr = currentraces.begin(); itr != currentraces.end(); itr++) {
    for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator its = (*itr)->begin(); its != (*itr)->end(); its++) {
      for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator itd = (*itr)->begin(); itd != (*itr)->end(); itd++) {
        int avgspeed = its->second->getSite()->getAverageSpeed(itd->second->getSite()->getName());
        if (avgspeed > maxavgspeed) maxavgspeed = avgspeed;
      }
    }
  }
}

void Engine::preSeedPotentialData(const std::shared_ptr<Race>& race) {
  std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator srcit;
  std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator dstit;
  int maxpointssizeandowned = getMaxPointsFileSize() + getMaxPointsPercentageOwned();
  for (srcit = race->begin(); srcit != race->end(); srcit++) {
    const std::shared_ptr<SiteRace>& srs = srcit->first;
    const std::shared_ptr<SiteLogic> & sls = srcit->second;
    if (sls->getSite()->getAllowDownload() == SITE_ALLOW_TRANSFER_NO ||
        (sls->getSite()->getAllowDownload() == SITE_ALLOW_DOWNLOAD_MATCH_ONLY && !srcit->first->isDownloadOnly()))
    {
      continue;
    }
    for (dstit = race->begin(); dstit != race->end(); dstit++) {
      const std::shared_ptr<SiteRace>& srd = dstit->first;
      const std::shared_ptr<SiteLogic> & sld = dstit->second;
      if (!raceTransferPossible(sls, srs, sld, srd, race)) {
        continue;
      }
      int priopoints = getPriorityPoints(sld->getSite()->getPriority());
      int speedpoints = getSpeedPoints(sls->getSite()->getAverageSpeed(sld->getSite()->getName()));
      for (unsigned int i = 0; i < sld->getSite()->getMaxUp(); ++i) {
        sls->pushPotential(maxpointssizeandowned + priopoints + speedpoints, "preseed", sld);
      }
    }
  }
}

bool Engine::raceTransferPossible(const std::shared_ptr<SiteLogic>& sls, const std::shared_ptr<SiteRace>& srs, const std::shared_ptr<SiteLogic>& sld, const std::shared_ptr<SiteRace>& srd, const std::shared_ptr<Race>& race) const {
  if (sls == sld) return false;
  const std::shared_ptr<Site> & srcsite = sls->getSite();
  const std::shared_ptr<Site> & dstsite = sld->getSite();
  if (srcsite->getAllowDownload() == SITE_ALLOW_TRANSFER_NO) {
    return false;
  }
  if (srs->isDownloadOnly()) {
    if (!srcsite->getMaxDownPre()) {
      return false;
    }
  }
  else if (srcsite->getAllowDownload() == SITE_ALLOW_DOWNLOAD_MATCH_ONLY) {
    return false;
  }
  else if (srs->isDone()) {
    if (!srcsite->getMaxDownComplete()) {
      return false;
    }
  }
  else if (!srcsite->getMaxDown() && !srcsite->getMaxDownComplete()) {
    return false;
  }
  if (dstsite->getAllowUpload() == SITE_ALLOW_TRANSFER_NO) {
    return false;
  }
  if (!srcsite->isAllowedTargetSite(dstsite)) {
    return false;
  }
  if (srd->isDownloadOnly()) {
    return false;
  }
  if (srcsite->hasBrokenPASV() && dstsite->hasBrokenPASV()) {
    return false;
  }
  // protocol check
  if (!transferProtocolCombinationPossible(srcsite->getTransferProtocol(), dstsite->getTransferProtocol())) {
    return false;
  }
  //ssl check
  int srcpolicy = srcsite->getSSLTransferPolicy();
  int dstpolicy = dstsite->getSSLTransferPolicy();
  if ((srcpolicy == SITE_SSL_ALWAYS_OFF &&
       dstpolicy == SITE_SSL_ALWAYS_ON) ||
      (srcpolicy == SITE_SSL_ALWAYS_ON &&
       dstpolicy == SITE_SSL_ALWAYS_OFF))
  {
    return false;
  }
  if ((srcpolicy == SITE_SSL_ALWAYS_ON || dstpolicy == SITE_SSL_ALWAYS_ON) &&
      !srcsite->supportsSSCN() && !dstsite->supportsSSCN())
  {
    return false;
  }
  return true;
}

unsigned int Engine::preparedRaces() const {
  return preparedraces.size();
}

unsigned int Engine::currentRaces() const {
  return currentraces.size();
}

unsigned int Engine::allRaces() const {
  return allraces.size();
}

unsigned int Engine::currentTransferJobs() const {
  return currenttransferjobs.size();
}

unsigned int Engine::allTransferJobs() const {
  return alltransferjobs.size();
}

std::shared_ptr<Race> Engine::getRace(unsigned int id) const {
  std::list<std::shared_ptr<Race> >::const_iterator it = allraces.find(id);
  if (it != allraces.end()) {
    return *it;
  }
  return std::shared_ptr<Race>();
}

std::shared_ptr<Race> Engine::getRace(const std::string& race) const {
  std::list<std::shared_ptr<Race> >::const_iterator it;
  for (it = allraces.begin(); it != allraces.end(); it++) {
    if ((*it)->getName() == race) {
      return *it;
    }
  }
  return std::shared_ptr<Race>();
}

std::shared_ptr<TransferJob> Engine::getTransferJob(unsigned int id) const {
  std::list<std::shared_ptr<TransferJob> >::const_iterator it = alltransferjobs.find(id);
  if (it != alltransferjobs.end()) {
    return *it;
  }
  return std::shared_ptr<TransferJob>();
}

std::shared_ptr<TransferJob> Engine::getTransferJob(const std::string& tj) const {
  std::list<std::shared_ptr<TransferJob> >::const_iterator it;
  for (it = alltransferjobs.begin(); it != alltransferjobs.end(); it++) {
    if ((*it)->getName() == tj) {
      return *it;
    }
  }
  return std::shared_ptr<TransferJob>();
}

std::list<std::shared_ptr<PreparedRace> >::const_iterator Engine::getPreparedRacesBegin() const {
  return preparedraces.begin();
}

std::list<std::shared_ptr<PreparedRace> >::const_iterator Engine::getPreparedRacesEnd() const {
  return preparedraces.end();
}

std::list<std::shared_ptr<Race> >::const_iterator Engine::getRacesBegin() const {
  return allraces.begin();
}

std::list<std::shared_ptr<Race> >::const_iterator Engine::getRacesEnd() const {
  return allraces.end();
}

std::list<std::shared_ptr<Race> >::const_iterator Engine::getCurrentRacesBegin() const {
  return currentraces.begin();
}

std::list<std::shared_ptr<Race> >::const_iterator Engine::getCurrentRacesEnd() const {
  return currentraces.end();
}

std::list<std::shared_ptr<Race> >::const_iterator Engine::getFinishedRacesBegin() const {
  return finishedraces.begin();
}

std::list<std::shared_ptr<Race> >::const_iterator Engine::getFinishedRacesEnd() const {
  return finishedraces.end();
}

std::list<std::shared_ptr<TransferJob> >::const_iterator Engine::getTransferJobsBegin() const {
  return alltransferjobs.begin();
}

std::list<std::shared_ptr<TransferJob> >::const_iterator Engine::getTransferJobsEnd() const {
  return alltransferjobs.end();
}

void Engine::tick(int message) {
  if (startnextprepared && getNextPreparedRaceStarterTimeout() != 0) {
    nextpreparedtimeremaining -= POKEINTERVAL;
    if (nextpreparedtimeremaining <= 0) {
      startnextprepared = false;
      global->getEventLog()->log("Engine", "Next prepared spread job starter timed out.");
    }
  }
  for (std::list<std::shared_ptr<Race> >::iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    std::shared_ptr<Race> race = *it;
    int timeoutafterseconds = race->timeoutCheck();
    if (timeoutafterseconds != -1) {
      if (waitingInScoreBoard(race)) {
        race->resetUpdateCheckCounter();
        continue;
      }
      if (race->failedTransfersCleared()) {
        global->getEventLog()->log("Engine", "No activity for " + std::to_string(timeoutafterseconds) +
            " seconds, aborting spread job: " + race->getName());
        for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator its = race->begin(); its != race->end(); its++) {
          its->first->timeout();
          its->second->raceLocalComplete(its->first, 0, false);
        }
        race->setTimeout();
        currentraces.erase(it);
        finishedraces.push_back(race);
        issueGlobalComplete(race);
        break;
      }
      else {
        if (race->clearTransferAttempts()) {
          restoreFromFailed(race);
          race->resetUpdateCheckCounter();
          std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it2;
          for (it2 = race->begin(); it2 != race->end(); it2++) {
            it2->second->activateOne();
          }
        }
      }
    }
    if (maxspreadjobtimeseconds > 0 && static_cast<int>(race->getTimeSpent()) > maxspreadjobtimeseconds) {
      global->getEventLog()->log("Engine", "Spread job " + race->getName() + " timed out after " + std::to_string(maxspreadjobtimeseconds) + " seconds.");
      for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator its = race->begin(); its != race->end(); its++) {
        its->first->timeout();
        its->second->raceLocalComplete(its->first, 0, false);
      }
      race->setTimeout();
      currentraces.erase(it);
      finishedraces.push_back(race);
      issueGlobalComplete(race);
      break;
    }
    for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it2 = race->begin(); it2 != race->end(); it2++) {
      int maxtimeseconds = it2->second->getSite()->getMaxSpreadJobTimeSeconds();
      if (maxtimeseconds > 0 && static_cast<int>(race->getTimeSpent()) > maxtimeseconds) {
        global->getEventLog()->log("Engine", "Spread job " + race->getName() + " timed out on " + it2->second->getSite()->getName() + " after " + std::to_string(maxtimeseconds) + " seconds.");
        it2->first->timeout();
        it2->second->raceLocalComplete(it2->first, 0, false);
        continue;
      }
      int wantedlogins = it2->second->getSite()->getMaxDown();
      if (!it2->first->isDone()) {
        wantedlogins = it2->second->getSite()->getMaxLogins();
      }
      if (it2->second->getCurrLogins() + it2->second->getCleanlyClosedConnectionsCount() < wantedlogins &&
          (it2->first->getMaxFileSize() || race->getTimeSpent() < RETRY_CONNECT_UNTIL_SEC))
      {
        it2->second->activateAll();
      }
    }
  }
  for (std::list<std::shared_ptr<TransferJob> >::const_iterator it = currenttransferjobs.begin(); it != currenttransferjobs.end(); it++) {
    if ((*it)->isDone()) {
      transferJobComplete(*it);
      break;
    }
    else {
      if (!!(*it)->getSrc() && !(*it)->getSrc()->getCurrLogins()) {
        (*it)->getSrc()->haveConnectedActivate((*it)->maxSlots());
      }
      if (!!(*it)->getDst() && !(*it)->getDst()->getCurrLogins()) {
        (*it)->getDst()->haveConnectedActivate((*it)->maxSlots());
      }
    }
  }
  std::list<unsigned int> removeids;
  for (std::list<std::shared_ptr<PreparedRace> >::const_iterator it = preparedraces.begin(); it != preparedraces.end(); it++) {
    (*it)->tick();
    if ((*it)->getRemainingTime() < 0) {
      removeids.push_back((*it)->getId());
    }
  }
  while (!removeids.empty()) {
    unsigned int id = removeids.front();
    removeids.pop_front();
    preparedraces.erase(id);
  }
  if (currentraces.empty() && currenttransferjobs.empty() && preparedraces.empty() && !startnextprepared && pokeregistered) {
    global->getTickPoke()->stopPoke(this, 0);
    pokeregistered = false;
  }
  estimateRaceSizes();
  refreshScoreBoard();
  forcescoreboard = true;
}

void Engine::issueGlobalComplete(const std::shared_ptr<Race> & race) {
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator itd = race->begin(); itd != race->end(); itd++) {
    itd->second->raceGlobalComplete(itd->first);
    wipeFromScoreBoard(itd->first);
    skiplistcachesites.insert(itd->second->getSite());
  }
  skiplistcachesections.insert(race->getSection());
  if (currentraces.empty()) {
    clearSkipListCaches();
  }
  rotateSpreadJobsHistory();
}

std::shared_ptr<ScoreBoard> Engine::getScoreBoard() const {
  return scoreboard;
}

void Engine::checkStartPoke() {
  if (!pokeregistered) {
    global->getTickPoke()->startPoke(this, "Engine", POKEINTERVAL, TICK_MSG_TICKER);
    pokeregistered = true;
  }
}

std::shared_ptr<Race> Engine::getCurrentRace(const std::string & release) const {
  for (std::list<std::shared_ptr<Race> >::const_iterator it = currentraces.begin(); it != currentraces.end(); it++) {
    if ((*it)->getName() == release) {
      return *it;
    }
  }
  return std::shared_ptr<Race>();
}

void Engine::addSiteToRace(const std::shared_ptr<Race>& race, const std::string& site, bool downloadonly) {
  const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site);
  bool affil = sl->getSite()->isAffiliated(race->getGroup());
  if (sl->getSite()->getSkipList().check((sl->getSite()->getSectionPath(race->getSection()) / race->getName()).toString(),
                                         true, false, &race->getSectionSkipList()).action != SKIPLIST_DENY ||
      affil || downloadonly)
  {
    downloadonly = downloadonly || (affil && race->getProfile() != SPREAD_DISTRIBUTE);
    std::shared_ptr<SiteRace> sr = sl->addRace(race, race->getSection(), race->getName(), downloadonly);
    race->addSite(sr, sl);
  }
}

int Engine::getMaxPointsRaceTotal() const {
  return getMaxPointsFileSize() + getMaxPointsAvgSpeed() + getMaxPointsPriority() + getMaxPointsPercentageOwned();
}

int Engine::getMaxPointsFileSize() const {
  return maxpointsfilesize;
}

int Engine::getMaxPointsAvgSpeed() const {
  return maxpointsavgspeed;
}

int Engine::getMaxPointsPriority() const {
  return maxpointspriority;
}

int Engine::getMaxPointsPercentageOwned() const {
  return maxpointspercentageowned;
}

int Engine::getMaxPointsLowProgress() const {
  return maxpointslowprogress;
}

int Engine::getPriorityPoints(SitePriority priority) const {
  switch (priority) {
    case SitePriority::VERY_LOW:
      return 0;
    case SitePriority::LOW:
      return maxpointspriority * 0.2;
    case SitePriority::NORMAL:
      return maxpointspriority * 0.4;
    case SitePriority::HIGH:
      return maxpointspriority * 0.6;
    case SitePriority::VERY_HIGH:
      return maxpointspriority;
  }
  return 0;
}

int Engine::getSpeedPoints(int avgspeed) const {
  if (maxavgspeed) {
    unsigned long long int pointsavgspeed = maxpointsavgspeed;
    pointsavgspeed *= avgspeed;
    pointsavgspeed /= maxavgspeed;
    return pointsavgspeed;
  }
  return 0;
}

int Engine::getPreparedRaceExpiryTime() const {
  return preparedraceexpirytime;
}

void Engine::setPreparedRaceExpiryTime(int expirytime) {
  preparedraceexpirytime = expirytime;
}

void Engine::setNextPreparedRaceStarterTimeout(int timeout) {
  startnextpreparedtimeout = timeout;
}

int Engine::getMaxSpreadJobsHistory() const {
  return maxspreadjobshistory;
}

int Engine::getMaxTransferJobsHistory() const {
  return maxtransferjobshistory;
}

void Engine::setMaxSpreadJobsHistory(int jobs) {
  maxspreadjobshistory = jobs;
  rotateSpreadJobsHistory();
}

void Engine::setMaxTransferJobsHistory(int jobs) {
  maxtransferjobshistory = jobs;
  rotateTransferJobsHistory();
}

void Engine::rotateSpreadJobsHistory() {
  if (maxspreadjobshistory == -1 || allraces.size() <= (unsigned int)maxspreadjobshistory) {
    return;
  }
  for (std::list<std::shared_ptr<Race>>::iterator it = finishedraces.begin(); it != finishedraces.end() && allraces.size() > (unsigned int)maxspreadjobshistory;) {
    unsigned int id = (*it)->getId();
    it = finishedraces.erase(it);
    allraces.erase(id);
  }
}

void Engine::rotateTransferJobsHistory() {
  if (maxtransferjobshistory == -1 || alltransferjobs.size() <= (unsigned int)maxtransferjobshistory) {
    return;
  }
  for (std::list<std::shared_ptr<TransferJob>>::iterator it = alltransferjobs.begin(); it != alltransferjobs.end() && alltransferjobs.size() > (unsigned int)maxtransferjobshistory;) {
    if ((*it)->isDone()) {
      std::string name = (*it)->getName();
      pendingtransfers.erase(*it);
      currenttransferjobs.erase(*it);
      it = alltransferjobs.erase(it);
    }
    else {
      ++it;
    }
  }
}

int Engine::getMaxSpreadJobTimeSeconds() const {
  return maxspreadjobtimeseconds;
}

void Engine::setMaxSpreadJobTimeSeconds(int seconds) {
  maxspreadjobtimeseconds = seconds;
}
