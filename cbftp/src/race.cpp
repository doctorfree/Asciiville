#include "race.h"

#include <vector>
#include <algorithm>

#include "core/tickpoke.h"
#include "file.h"
#include "filelist.h"
#include "site.h"
#include "siterace.h"
#include "util.h"
#include "globalcontext.h"
#include "transferstatus.h"
#include "sectionmanager.h"
#include "section.h"
#include "skiplist.h"
#include "sitelogic.h"

#define MAX_CHECKS_BEFORE_TIMEOUT 60
#define MAX_CHECKS_BEFORE_HARD_TIMEOUT 1200
#define MAX_TRANSFER_ATTEMPTS_BEFORE_SKIP 3

typedef std::pair<std::shared_ptr<FileList>, std::shared_ptr<FileList>> FailedTransferSecond;

bool SitesComparator::operator()(const std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > & a,
                                 const std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > & b) const
{
  return a.first->getSiteName() < b.first->getSiteName();
}

namespace {

bool remainingSitesAreDownloadOnly(const std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic>>, SitesComparator>& allsites,
                                   const std::unordered_set<std::shared_ptr<SiteRace>>& donesites)
{
  std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> >, SitesComparator> remainingsites = allsites;
  bool erased = true;
  while (erased) {
    erased = false;
    for (auto it = remainingsites.begin(); it != remainingsites.end(); ++it) {
      if (donesites.find(it->first) != donesites.end()) {
        erased = true;
        remainingsites.erase(it);
        break;
      }
    }
  }
  for (auto it = remainingsites.begin(); it != remainingsites.end(); ++it) {
    if (!it->first->isDownloadOnly()) {
      return false;
    }
  }
  return true;
}

} // namespace

Race::Race(unsigned int id, SpreadProfile profile, const std::string& release, const std::string& section) :
  name(release),
  group(util::getGroupNameFromRelease(release)),
  section(section),
  sectionskiplist(global->getSectionManager()->getSection(section)->getSkipList()),
  maxnumfilessiteprogress(0),
  bestunknownfilesizeestimate(50000000),
  guessedtotalfilesize(0),
  guessedtotalnumfiles(0),
  checkcount(0),
  timestamp(util::ctimeLog()),
  timespent(0),
  status(RaceStatus::RUNNING),
  worst(0),
  avg(0),
  best(0),
  transferattemptscleared(false),
  id(id),
  profile(profile),
  transferredsize(0),
  transferredfiles(0)
{
  estimatedsubpaths.insert("");
  guessedfilelists[""] = std::unordered_map<std::string, unsigned long long int>();
  setUndone();
}

Race::~Race() {
  if (!isDone()) {
    setDone();
  }
}

CallbackType Race::callbackType() const {
  return CallbackType::RACE;
}

void Race::addSite(const std::shared_ptr<SiteRace>& siterace, const std::shared_ptr<SiteLogic>& sl) {
  sites.insert(std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> >(siterace, sl));
}

void Race::removeSite(const std::shared_ptr<SiteLogic>& sl) {
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::iterator it = sites.begin(); it != sites.end(); it++) {
    if (it->second == sl) {
      removeSite(it->first);
      break;
    }
  }
}

void Race::removeSite(const std::shared_ptr<SiteRace>& siterace) {
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::iterator it = sites.begin(); it != sites.end(); it++) {
    if (it->first == siterace) {
      sites.erase(it);
      break;
    }
  }
  semidonesites.erase(siterace);
  donesites.erase(siterace);
  sizes.erase(siterace);
  for (std::unordered_map<std::string, std::unordered_set<std::shared_ptr<SiteRace>> >::iterator it = sfvreports.begin(); it != sfvreports.end(); it++) {
    it->second.erase(siterace);
  }
}

std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator Race::begin() const {
  return sites.begin();
}

std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator Race::end() const {
  return sites.end();
}

std::string Race::getName() const {
  return name;
}

std::string Race::getGroup() const {
  return group;
}

std::string Race::getSection() const {
  return section;
}

int Race::numSitesDone() const {
  return donesites.size();
}

int Race::numSites() const {
  return sites.size();
}

bool Race::sizeEstimated(const std::string& subpath) const {
  return estimatedsize.find(subpath) != estimatedsize.end();
}

unsigned int Race::estimatedSize(const std::string& subpath) const {
  std::unordered_map<std::string, unsigned int>::const_iterator it = estimatedsize.find(subpath);
  if (it != estimatedsize.end()) {
    return it->second;
  }
  return 0;
}

unsigned int Race::guessedSize(const std::string& subpath) const {
  std::unordered_map<std::string, std::unordered_map<std::string, unsigned long long int> >::const_iterator it =
      guessedfilelists.find(subpath);
  if (it != guessedfilelists.end()) {
    return it->second.size();
  }
  return 10;
}

unsigned long long int Race::estimatedTotalSize() const {
  return guessedtotalfilesize;
}

unsigned long long int Race::guessedFileSize(const std::string& subpath, const std::string& file) const {
  std::unordered_map<std::string, std::unordered_map<std::string, SizeLocationTrack> >::const_iterator it =
      sizelocationtrackers.find(subpath);
  if (it == sizelocationtrackers.end()) {
    return bestunknownfilesizeestimate;
  }
  std::unordered_map<std::string, SizeLocationTrack>::const_iterator it2 = it->second.find(file);
  if (it2 == it->second.end()) {
    return bestunknownfilesizeestimate;
  }
  unsigned long long int thisestimatedsize = it2->second.getEstimatedSize();
  if (!thisestimatedsize) {
    return bestunknownfilesizeestimate;
  }
  return thisestimatedsize;
}

std::unordered_map<std::string, unsigned long long int>::const_iterator Race::guessedFileListBegin(const std::string & subpath) const {
  std::unordered_map<std::string, std::unordered_map<std::string, unsigned long long int> >::const_iterator it;
  if ((it = guessedfilelists.find(subpath)) != guessedfilelists.end()) {
    return it->second.begin();
  }
  return guessedfilelists.at("").end();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator Race::guessedFileListEnd(const std::string & subpath) const {
  std::unordered_map<std::string, std::unordered_map<std::string, unsigned long long int> >::const_iterator it;
  if ((it = guessedfilelists.find(subpath)) != guessedfilelists.end()) {
    return it->second.end();
  }
  return guessedfilelists.at("").end();
}

bool Race::SFVReported(const std::string& subpath) const {
  std::unordered_map<std::string, std::unordered_set<std::shared_ptr<SiteRace>> >::const_iterator it = sfvreports.find(subpath);
  return it != sfvreports.end();
}

std::unordered_set<std::string> Race::getSubPaths() const {
  return estimatedsubpaths;
}

void Race::updateSiteProgress(unsigned int numuploadedfiles) {
  if (maxnumfilessiteprogress < numuploadedfiles) maxnumfilessiteprogress = numuploadedfiles;
}

unsigned int Race::getMaxSiteNumFilesProgress() const {
  return maxnumfilessiteprogress;
}

bool Race::isDone() const {
  return status != RaceStatus::RUNNING;
}

void Race::reportNewSubDir(const std::shared_ptr<SiteRace>& sr, const std::string& subdir) {
  if (estimatedsubpaths.find(subdir) == estimatedsubpaths.end()) {
    estimatedsubpaths.insert(subdir);
    guessedfilelists[subdir] = std::unordered_map<std::string, unsigned long long int>();
  }
}

void Race::reportSFV(const std::shared_ptr<SiteRace>& sr, const std::string& subpath) {
  if (sfvreports.find(subpath) == sfvreports.end()) {
    sfvreports[subpath] = std::unordered_set<std::shared_ptr<SiteRace>>();
  }
  std::unordered_set<std::shared_ptr<SiteRace>>& sfvreportssubpath = sfvreports.at(subpath);
  sfvreportssubpath.insert(sr);
}

void Race::reportDone(const std::shared_ptr<SiteRace>& sr) {
  if (donesites.find(sr) != donesites.end()) {
    return;
  }
  reportSemiDone(sr);
  donesites.insert(sr);
  if (donesites.size() == sites.size() || remainingSitesAreDownloadOnly(sites, donesites)) {
    setDone();
  }
}

void Race::reportSemiDone(const std::shared_ptr<SiteRace>& sr) {
  if (semidonesites.find(sr) != semidonesites.end()) {
    return;
  }
  semidonesites.insert(sr);
  if (semidonesites.size() == sites.size() || remainingSitesAreDownloadOnly(sites, semidonesites)) {
    setDone();
    for (auto it = sites.begin(); it != sites.end(); it++) {
      it->first->complete(false);
    }
  }
}

void Race::setUndone() {
  status = RaceStatus::RUNNING;
  clearTransferAttempts(false);
  resetUpdateCheckCounter();
  global->getTickPoke()->startPoke(this, "Race", RACE_UPDATE_INTERVAL, 0);
}

void Race::reset() {
  clearTransferAttempts(false);
  resetUpdateCheckCounter();
  if (isDone()) {
    setUndone();
  }
  semidonesites.clear();
  donesites.clear();
  sizes.clear();
  estimatedsubpaths.clear();
  estimatedsubpaths.insert("");
  guessedfilelists.clear();
  guessedfilelists[""] = std::unordered_map<std::string, unsigned long long int>();
  sfvreports.clear();
  estimatedsize.clear();
  estimatedfilesizes.clear();
  guessedfileliststotalfilesize.clear();
  sizelocationtrackers.clear();
  maxnumfilessiteprogress = 0;
  worst = 0;
  avg = 0;
  best = 0;
  guessedtotalfilesize = 0;
  bestunknownfilesizeestimate = 50000000;
}

void Race::abort() {
  setDone();
  status = RaceStatus::ABORTED;
}

void Race::setTimeout() {
  setDone();
  status = RaceStatus::TIMEOUT;
}

void Race::reportSize(const std::shared_ptr<SiteRace>& sr, const std::shared_ptr<FileList>& fl, const std::string& subpath, const std::unordered_set<std::string >& uniques, bool final) {
  std::unordered_set<std::string>::const_iterator itu;
  File* file;
  if (sizelocationtrackers.find(subpath) == sizelocationtrackers.end()) {
    sizelocationtrackers[subpath] = std::unordered_map<std::string, SizeLocationTrack>();
  }
  bool recalc = false;
  for (itu = uniques.begin(); itu != uniques.end(); itu++) {
    const std::string& unique = *itu;
    if (sizelocationtrackers[subpath].find(unique) == sizelocationtrackers[subpath].end()) {
      sizelocationtrackers[subpath][unique] = SizeLocationTrack();
    }
    if ((file = fl->getFile(unique)) != nullptr) {
      if (sizelocationtrackers[subpath][unique].add(sr, file->getSize())) {
        estimatedfilesizes[unique] = sizelocationtrackers[subpath][unique].getEstimatedSize();
        recalc = true;
      }
    }
    else {
      sizelocationtrackers[subpath][unique].add(sr, 0);
    }
  }
  if (recalc) {
    recalculateBestUnknownFileSizeEstimate();
  }
  if (sizes.find(sr) == sizes.end()) {
    sizes[sr] = std::unordered_map<std::string, unsigned int>();
  }
  if (guessedfilelists.find(subpath) != guessedfilelists.end()) {
    guessedfilelists[subpath].clear();
    std::unordered_map<std::string, SizeLocationTrack>::iterator it;
    int highestnumsites = 0;
    for (it = sizelocationtrackers[subpath].begin(); it != sizelocationtrackers[subpath].end(); it++) {
      int thisnumsites = it->second.numSites();
      if (thisnumsites > highestnumsites) {
        highestnumsites = thisnumsites;
      }
    }
    int minnumsites = highestnumsites / 2;
    unsigned long long int aggregatedsize = 0;
    for (it = sizelocationtrackers[subpath].begin(); it != sizelocationtrackers[subpath].end(); it++) {
      if (it->second.numSites() > minnumsites || sites.size() == 2) {
        unsigned long long int estimatedsize = it->second.getEstimatedSize();
        if (!estimatedsize) {
          estimatedsize = bestunknownfilesizeestimate;
        }
        guessedfilelists[subpath][it->first] = estimatedsize;
        aggregatedsize += estimatedsize;
      }
    }
    if (guessedfileliststotalfilesize[subpath] != aggregatedsize) {
      guessedfileliststotalfilesize[subpath] = aggregatedsize;
      calculateTotal();
    }
    if (final) {
      sizes[sr][subpath] = uniques.size();
      std::unordered_map<std::shared_ptr<SiteRace>, std::unordered_map<std::string, unsigned int> >::iterator it;
      std::unordered_map<std::string, unsigned int>::iterator it2;
      std::vector<unsigned int> subpathsizes;
      for (it = sizes.begin(); it != sizes.end(); it++) {
        it2 = it->second.find(subpath);
        if (it2 != it->second.end()) {
          subpathsizes.push_back(it2->second);
        }
      }
      if (subpathsizes.size() == sites.size() ||
        (subpathsizes.size() >= sites.size() * 0.8 && sites.size() > 2) ||
        getTimeSpent() >= 20)
      {
        estimatedsize[subpath] = guessedfilelists[subpath].size();
      }
    }
  }
}

void Race::recalculateBestUnknownFileSizeEstimate() {
  std::unordered_map<std::string, unsigned long long int>::iterator it;
  unsigned long long int size;
  std::unordered_map<unsigned long long int, int> commonsizes;
  std::unordered_map<unsigned long long int, int>::iterator it2;
  for (it = estimatedfilesizes.begin(); it != estimatedfilesizes.end(); it++) {
    size = it->second;
    if (!size) {
      continue;
    }
    it2 = commonsizes.find(size);
    if (it2 != commonsizes.end()) {
      commonsizes[size] = it2->second + 1;
    }
    else {
      commonsizes[size] = 1;
    }
  }
  unsigned long long int mostcommon = 0;
  int mostcommoncount = 0;
  unsigned long long int largest = 0;
  int largestcount = 1;
  for (it2 = commonsizes.begin(); it2 != commonsizes.end(); it2++) {
    if (it2->second > mostcommoncount) {
      mostcommon = it2->first;
      mostcommoncount = it2->second;
    }
    if (it2->first > largest) {
      largest = it2->first;
      largestcount = 1;
    }
    else if (it2->first == largest) {
      largestcount++;
    }
  }
  if (largestcount >= 2 || largestcount == mostcommoncount ||
      (mostcommon == 0 && mostcommoncount + 1 < (int)commonsizes.size())) {
    bestunknownfilesizeestimate = largest;
  }
  else {
    bestunknownfilesizeestimate = mostcommon;
  }
}

int Race::timeoutCheck() {
  if (checkcount >= MAX_CHECKS_BEFORE_HARD_TIMEOUT) {
    return MAX_CHECKS_BEFORE_HARD_TIMEOUT;
  }
  bool allrefreshed = true;
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::iterator it = sites.begin(); it != sites.end(); it++) {
    if (it->first->isDone()) {
      continue;
    }
    if (it->first->listsChangedSinceLastCheck()) {
      checkcount = 0;
    }
    else if (checkcount % 10 == 0 && checkcount <= MAX_CHECKS_BEFORE_TIMEOUT) {
      it->first->resetListsRefreshed();
    }
    else if (checkcount > MAX_CHECKS_BEFORE_TIMEOUT) {
      if (it->second->getCurrLogins() && !it->first->allListsRefreshed()) {
        allrefreshed = false;
      }
    }
  }
  if (checkcount > MAX_CHECKS_BEFORE_TIMEOUT && allrefreshed) {
    return MAX_CHECKS_BEFORE_TIMEOUT;
  }
  checkcount++;
  return -1;
}

void Race::resetUpdateCheckCounter() {
  checkcount = 0;
}

std::string Race::getTimeStamp() const {
  return timestamp;
}

void Race::setDone() {
  status = RaceStatus::DONE;
  global->getTickPoke()->stopPoke(this, 0);
  calculatePercentages();
}

void Race::tick(int message) {
  timespent += RACE_UPDATE_INTERVAL;
  calculatePercentages();
}

void Race::calculatePercentages() {
  unsigned int totalpercentage = 0;
  unsigned int localworst = 100;
  unsigned int localbest = 0;
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = begin(); it != end(); it++) {
    unsigned int percentagecomplete = 0;
    if (status == RaceStatus::RUNNING) {
      if (guessedtotalfilesize) {
        percentagecomplete = (it->first->getTotalFileSize() * 100) / guessedtotalfilesize;
      }
    }
    else {
      if (guessedtotalnumfiles) {
        percentagecomplete = (it->first->getNumUploadedFiles() * 100) / guessedtotalnumfiles;
      }
    }
    if (percentagecomplete > 100) {
      percentagecomplete = 100;
    }
    totalpercentage += percentagecomplete;
    if (percentagecomplete < localworst) {
      localworst = percentagecomplete;
    }
    if (percentagecomplete > localbest) {
      localbest = percentagecomplete;
    }
  }
  if (sites.size()) {
    avg = totalpercentage / sites.size();
  }
  worst = localworst;
  best = localbest;
}

void Race::calculateTotal() {
  unsigned long long int aggregatedsize = 0;
  for (std::unordered_map<std::string, unsigned long long int>::const_iterator it =
      guessedfileliststotalfilesize.begin(); it != guessedfileliststotalfilesize.end(); it++) {
    aggregatedsize += it->second;
  }
  guessedtotalfilesize = aggregatedsize;
  std::unordered_map<std::string, std::unordered_map<std::string, unsigned long long int> >::iterator it;
  guessedtotalnumfiles = 0;
  for (it = guessedfilelists.begin(); it != guessedfilelists.end(); it++) {
    guessedtotalnumfiles += it->second.size();
  }
}

unsigned int Race::getTimeSpent() const {
  return timespent / 1000;
}

std::string Race::getSiteListText(const SiteListType listtype) const {
  std::string sitestr = "";
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = begin(); it != end(); it++) {
    std::string sitename = it->first->getSiteName();
    switch (listtype) {
      case SiteListType::ALL: {
        sitestr += sitename + ",";
        break;
      }
      case SiteListType::DLONLY: {
        if (it->first->isDownloadOnly()) {
          sitestr += sitename + ",";
        }
        break;
      }
      case SiteListType::INCOMPLETE: {
        if (it->first->getStatus() != RaceStatus::DONE) {
          sitestr += sitename + ",";
        }
        break;
      }
      case SiteListType::COMPLETE: {
        if (it->first->isDone()) {
          sitestr += sitename + ",";
        }
        break;
      }
    }
  }
  if (sitestr.length() > 0) {
    sitestr = sitestr.substr(0, sitestr.length() - 1);
  }
  return sitestr;
}

RaceStatus Race::getStatus() const {
  return status;
}

unsigned int Race::getId() const {
  return id;
}

SpreadProfile Race::getProfile() const {
  return profile;
}

std::shared_ptr<SiteRace> Race::getSiteRace(const std::string & site) const {
  for (std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator it = begin(); it != end(); it++) {
    if (it->first->getSiteName() == site) {
      return it->first;
    }
  }
  return std::shared_ptr<SiteRace>();
}

unsigned int Race::getWorstCompletionPercentage() const {
  return worst;
}

unsigned int Race::getAverageCompletionPercentage() const {
  return avg;
}

unsigned int Race::getBestCompletionPercentage() const {
  return best;
}

bool Race::hasFailedTransfer(const std::string & filename, const std::shared_ptr<FileList>& fls, const std::shared_ptr<FileList>& fld) const {
  FailedTransfer matchall = FailedTransfer(filename, FailedTransferSecond(nullptr, fld));
  auto it = transferattempts.find(matchall);
  if (it != transferattempts.end() && it->second >= MAX_TRANSFER_ATTEMPTS_BEFORE_SKIP) {
    return true;
  }
  FailedTransfer match = FailedTransfer(filename, FailedTransferSecond(fls, fld));
  it = transferattempts.find(match);
  size_t numsites = sites.size();
  return it != transferattempts.end() && ((numsites == 3 && it->second >= 2) ||
                                          (numsites > 3 && it->second >= 1));
}

bool Race::failedTransfersCleared() const {
  return transferattemptscleared;
}

const SkipList & Race::getSectionSkipList() const {
  return sectionskiplist;
}

void Race::addTransfer(const std::shared_ptr<TransferStatus>& ts) {
  if (!!ts) {
    ts->setCallback(this);
  }
}

bool Race::clearTransferAttempts(bool clearstate) {
  bool ret = transferattempts.size();
  transferattempts.clear();
  transferattemptscleared = clearstate;
  return ret;
}

void Race::transferSuccessful(const std::shared_ptr<TransferStatus>& ts) {
  addTransferAttempt(ts);
}

void Race::transferFailed(const std::shared_ptr<TransferStatus>& ts, int) {
  addTransferAttempt(ts);
}

void Race::addTransferAttempt(const std::shared_ptr<TransferStatus>& ts) {
  std::shared_ptr<FileList> srcfl = ts->getSourceFileList();
  std::shared_ptr<FileList> dstfl = ts->getTargetFileList();
  std::string file = ts->getFile();
  FailedTransfer match = FailedTransfer(file, FailedTransferSecond(srcfl, dstfl));
  FailedTransfer matchall = FailedTransfer(file, FailedTransferSecond(nullptr, dstfl));
  auto it = transferattempts.find(matchall);
  if (it == transferattempts.end()) {
    transferattempts[matchall] = 1;
  }
  else {
    it->second++;
  }
  it = transferattempts.find(match);
  if (it == transferattempts.end()) {
    transferattempts[match] = 1;
  }
  else {
    it->second++;
  }
}

void Race::addTransferStatsFile(unsigned long long int size) {
  transferredsize += size;
  ++transferredfiles;
}

unsigned long long int Race::getTransferredSize() const {
  return transferredsize;
}

unsigned int Race::getTransferredFiles() const {
  return transferredfiles;
}
