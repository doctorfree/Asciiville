#include "siterace.h"

#include "race.h"
#include "filelist.h"
#include "file.h"
#include "globalcontext.h"
#include "skiplist.h"
#include "util.h"
#include "timereference.h"

SiteRace::SiteRace(const std::shared_ptr<Race>& race, const std::string& sitename, const Path& section, const std::string& jobname, const std::string& username, const SkipList& skiplist, bool downloadonly) :
  race(race),
  section(section),
  jobname(jobname),
  path(section / jobname),
  group(util::getGroupNameFromRelease(jobname)),
  username(username),
  sitename(sitename),
  status(RaceStatus::RUNNING),
  maxfilesize(0),
  totalfilesize(0),
  numuploadedfiles(0),
  profile(race->getProfile()),
  skiplist(skiplist),
  sizedown(0),
  filesdown(0),
  speeddown(1),
  sizeup(0),
  filesup(0),
  speedup(1),
  downloadonly(downloadonly),
  id(race->getId())
{
  recentlyvisited.push_back("");
  filelists[""] = std::make_shared<FileList>(username, path);
}

SiteRace::~SiteRace() {
}

std::string SiteRace::getName() const {
  return jobname;
}

int SiteRace::classType() const {
  return COMMANDOWNER_SITERACE;
}

std::string SiteRace::getSiteName() const {
  return sitename;
}

const Path& SiteRace::getSection() const {
  return section;
}

std::string SiteRace::getGroup() const {
  return group;
}

const Path& SiteRace::getPath() const {
  return path;
}

unsigned int SiteRace::getId() const {
  return id;
}

bool SiteRace::addSubDirectory(const std::string& subpath, bool knownexists) {
  std::shared_ptr<Race> srace = race.lock();
  if (!srace) {
    return false;
  }
  SkipListMatch match = skiplist.check(subpath, true, true, &srace->getSectionSkipList());
  if (match.action == SKIPLIST_DENY) {
    return false;
  }
  if (getFileListForPath(subpath) != NULL) {
    return true;
  }
  if (match.action == SKIPLIST_UNIQUE && filelists[""]->containsPatternBefore(match.matchpattern, true, subpath)) {
    return false;
  }
  std::shared_ptr<FileList> subdir;
  if (knownexists) {
    subdir = std::make_shared<FileList>(username, path / subpath, FileListState::EXISTS);
  }
  else {
    subdir = std::make_shared<FileList>(username, path / subpath);
  }
  filelists[subpath] = subdir;
  recentlyvisited.push_back(subpath);
  if (knownexists) {
    srace->reportNewSubDir(shared_from_this(), subpath);
  }
  return true;
}

std::string SiteRace::getSubPath(const std::shared_ptr<FileList>& fl) const {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == fl) {
      return it->first;
    }
  }
  // internal error
  return "";
}

std::string SiteRace::getRelevantSubPath() {
  for (const std::pair<const std::string, std::shared_ptr<FileList>>& flpair : filelists) {
    if (flpair.second->getUpdateState() == UpdateState::NONE) {
      return flpair.first;
    }
  }
  std::shared_ptr<FileList> base = filelists.at("");
  if (base->getState() == FileListState::UNKNOWN || base->getState() == FileListState::NONEXISTENT || base->getState() == FileListState::FAILED) {
    return "";
  }
  for (unsigned int i = 0; i < recentlyvisited.size() && recentlyvisited.size(); i++) {
    std::string leastrecentlyvisited = recentlyvisited.front();
    recentlyvisited.pop_front();
    if (!isSubPathComplete(leastrecentlyvisited)) {
      recentlyvisited.push_back(leastrecentlyvisited);
      std::shared_ptr<FileList> fl = filelists.at(leastrecentlyvisited);
      if (fl->getState() != FileListState::NONEXISTENT && fl->getState() != FileListState::FAILED) {
        return leastrecentlyvisited;
      }
    }
  }
  return "";
}

bool SiteRace::anyFileListNotNonExistent() const {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->getState() != FileListState::NONEXISTENT && it->second->getState() != FileListState::FAILED) {
      return true;
    }
  }
  return false;
}

std::shared_ptr<FileList> SiteRace::getFileListForPath(const std::string& subpath) const {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it = filelists.find(subpath);
  if (it != filelists.end()) {
    return it->second;
  }
  return nullptr;
}

std::shared_ptr<FileList> SiteRace::getFileListForFullPath(SiteLogic*, const Path& path) const {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->getPath() == path) {
      return it->second;
    }
  }
  return nullptr;
}

std::string SiteRace::getSubPathForFileList(const std::shared_ptr<FileList>& fl) const {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == fl) {
      return it->first;
    }
  }
  return "";
}

std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator SiteRace::fileListsBegin() const {
  return filelists.begin();
}

std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator SiteRace::fileListsEnd() const {
  return filelists.end();
}

void SiteRace::fileListUpdated(SiteLogic*, const std::shared_ptr<FileList>& fl) {
  updateNumFilesUploaded(fl);
  addNewDirectories(fl);
  markNonExistent(fl);
}

void SiteRace::updateNumFilesUploaded(const std::shared_ptr<FileList>& fl) {
  std::shared_ptr<Race> srace = race.lock();
  if (!srace) {
    return;
  }
  std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it;
  unsigned int sum = 0;
  unsigned long long int maxsize = 0;
  unsigned long long int maxsizewithfiles = 0;
  unsigned long long int aggregatedfilesize = 0;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    const std::shared_ptr<FileList>& currfl = it->second;
    sum += currfl->getNumUploadedFiles();
    aggregatedfilesize += currfl->getTotalFileSize();
    if (fl == currfl && currfl->hasExtension("sfv")) {
      srace->reportSFV(shared_from_this(), it->first);
    }
    unsigned long long int max = currfl->getMaxFileSize();
    if (max > maxsize) {
      maxsize = max;
      if (currfl->getSize() >= 5) {
        maxsizewithfiles = max;
      }
    }
  }
  if (maxsizewithfiles > 0) {
    this->maxfilesize = maxsizewithfiles;
  }
  else {
    this->maxfilesize = maxsize;
  }
  numuploadedfiles = sum;
  totalfilesize = aggregatedfilesize;
  srace->updateSiteProgress(sum);
}

void SiteRace::addNewDirectories(const std::shared_ptr<FileList>& fl) {
  std::shared_ptr<Race> srace = race.lock();
  if (!srace) {
    return;
  }
  if (fl != getFileListForPath("") || fl->getState() != FileListState::LISTED) {
    return;
  }
  std::list<File*>::iterator it;
  for(it = fl->begin(); it != fl->end(); it++) {
    File * file = *it;
    const std::string& filename = file->getName();
    if (file->isDirectory()) {
      SkipListMatch match = skiplist.check(filename, true, true, &srace->getSectionSkipList());
      if (match.action == SKIPLIST_DENY || (match.action == SKIPLIST_UNIQUE && fl->containsPatternBefore(match.matchpattern, true, filename)))
      {
        continue;
      }
      std::shared_ptr<FileList> sublist;
      if (!(sublist = getFileListForPath(filename))) {
        addSubDirectory(filename, true);
      }
      else if (sublist->getState() == FileListState::UNKNOWN || sublist->getState() == FileListState::NONEXISTENT ||
          sublist->getState() == FileListState::FAILED)
      {
        sublist->setExists();
        srace->reportNewSubDir(shared_from_this(), filename);
      }
    }
  }
}

void SiteRace::markNonExistent(const std::shared_ptr<FileList>& fl) {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it;
  bool found = false;
  for (it = filelists.begin() ;it != filelists.end(); it++) {
    if (fl == it->second) {
      found = true;
    }
  }
  if (!found) {
    return;
  }
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == fl) {
      continue;
    }
    if (!it->second->getPath().contains(fl->getPath())) {
      continue;
    }
    if (fl->getState() == FileListState::NONEXISTENT ||
        !fl->contains((it->second->getPath() - fl->getPath()).level(1).toString()))
    {
      if (it->second->getState() == FileListState::UNKNOWN) {
        it->second->setNonExistent();
      }
      if (it->second->getState() == FileListState::NONEXISTENT) {
        it->second->bumpUpdateState(UpdateState::REFRESHED);
        markNonExistent(it->second);
      }
    }
  }
}

unsigned int SiteRace::getNumUploadedFiles() const {
  return numuploadedfiles;
}

bool SiteRace::sizeEstimated(const std::shared_ptr<FileList>& fl) const {
  return sizeestimated.find(fl) != sizeestimated.end();
}

unsigned long long int SiteRace::getMaxFileSize() const {
  return maxfilesize;
}

unsigned long long int SiteRace::getTotalFileSize() const {
  return totalfilesize;
}

bool SiteRace::isDone() const {
  return status != RaceStatus::RUNNING;
}

RaceStatus SiteRace::getStatus() const {
  return status;
}

bool SiteRace::isGlobalDone() const {
  std::shared_ptr<Race> srace = race.lock();
  if (!srace) {
    return true;
  }
  return srace->isDone();
}

void SiteRace::complete(bool report) {
  std::shared_ptr<Race> srace = race.lock();
  if (!srace) {
    return;
  }
  if (status == RaceStatus::RUNNING) {
    status = RaceStatus::DONE;
  }
  if (report) {
    srace->reportDone(shared_from_this());
  }
}

void SiteRace::abort() {
  if (status == RaceStatus::RUNNING) {
    status = RaceStatus::ABORTED;
  }
}

void SiteRace::timeout() {
  if (status == RaceStatus::RUNNING) {
    status = RaceStatus::TIMEOUT;
  }
}

void SiteRace::softReset() {
  recentlyvisited.clear();
  for (std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it = filelists.begin(); it != filelists.end(); ++it) {
    recentlyvisited.push_back(it->first);
    FileListState state = it->second->getState();
    if (state == FileListState::LISTED) {
      state = FileListState::EXISTS;
    }
    it->second = std::make_shared<FileList>(username, path / it->first, state);
  }
  reset();
}

void SiteRace::hardReset() {
  filelists.clear();
  filelists[""] = std::make_shared<FileList>(username, path);
  recentlyvisited.clear();
  recentlyvisited.push_back("");
  reset();
}

void SiteRace::reset() {
  status = RaceStatus::RUNNING;
  sfvobservestarts.clear();
  observestarts.clear();
  sizeestimated.clear();
  completesubdirs.clear();
  maxfilesize = 0;
  totalfilesize = 0;
  numuploadedfiles = 0;
}

void SiteRace::subPathComplete(const std::shared_ptr<FileList>& fl) {
  std::string subpath = getSubPathForFileList(fl);
  if (isSubPathComplete(subpath)) {
    return;
  }
  fl->bumpUpdateState(UpdateState::REFRESHED);
  completesubdirs.push_back(subpath);
}

void SiteRace::resetListsRefreshed() {
  for (const std::pair<const std::string, std::shared_ptr<FileList>>& filelist : filelists) {
    if (!isSubPathComplete(getSubPath(filelist.second))) {
      filelist.second->resetUpdateState();
    }
  }
}

bool SiteRace::isSubPathComplete(const std::string& subpath) const {
  std::list<std::string>::const_iterator it;
  for (it = completesubdirs.begin(); it != completesubdirs.end(); it++) {
    if (*it == subpath) {
      return true;
    }
  }
  return false;
}

bool SiteRace::isSubPathComplete(const std::shared_ptr<FileList>& fl) const {
  std::string subpath = getSubPathForFileList(fl);
  return isSubPathComplete(subpath);
}

std::weak_ptr<Race> SiteRace::getRace() const {
  return race;
}

int SiteRace::getProfile() const {
  return profile;
}

void SiteRace::reportSize(const std::shared_ptr<FileList>& fl, const std::unordered_set<std::string>& uniques, bool final) {
  std::shared_ptr<Race> srace = race.lock();
  if (!srace) {
    return;
  }
  std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == fl) {
      srace->reportSize(shared_from_this(), fl, it->first, uniques, final);
      if (final) {
        sizeestimated.insert(fl);
      }
      return;
    }
  }
}

int SiteRace::getObservedTime(const std::shared_ptr<FileList>& fl) {
  std::unordered_map<std::shared_ptr<FileList>, unsigned long long int>::iterator it;
  for (it = observestarts.begin(); it != observestarts.end(); it++) {
    if (it->first == fl) {
      return global->getTimeReference()->timePassedSince(it->second);
    }
  }
  if (fl->getSize() > 0) {
    observestarts[fl] = global->getTimeReference()->timeReference();
  }
  return 0;
}

int SiteRace::getSFVObservedTime(const std::shared_ptr<FileList>& fl) {
  std::unordered_map<std::shared_ptr<FileList>, unsigned long long int>::iterator it;
  for (it = sfvobservestarts.begin(); it != sfvobservestarts.end(); it++) {
    if (it->first == fl) {
      return global->getTimeReference()->timePassedSince(it->second);
    }
  }
  sfvobservestarts[fl] = global->getTimeReference()->timeReference();
  return 0;
}

bool SiteRace::listsChangedSinceLastCheck() {
  bool changed = false;
  std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->getUpdateState() == UpdateState::CHANGED) {
      changed = true;
      it->second->resetUpdateState();
    }
    else if (it->second->hasFilesUploading()) {
      changed = true;
    }
  }
  return changed;
}

void SiteRace::addTransferStatsFile(StatsDirection direction, const std::string& other, unsigned long long int size, unsigned int speed) {
  std::shared_ptr<Race> srace = race.lock();
  if (!srace) {
    return;
  }
  if (direction == STATS_DOWN) {
    srace->addTransferStatsFile(size);
    if (sitessizedown.find(other) == sitessizedown.end()) {
      sitessizedown[other] = 0;
      sitesfilesdown[other] = 0;
      sitesspeeddown[other] = 1;
    }
    if (speed && size && size > speed) {
      sitesspeeddown[other] = (sitessizedown[other] + size) / (sitessizedown[other] / sitesspeeddown[other] + size / speed);
      if (!sitesspeeddown[other]) {
        ++sitesspeeddown[other];
      }
      speeddown = (sizedown + size) / (sizedown / speeddown + size / speed);
      if (!speeddown) {
        ++speeddown;
      }
    }
    sitessizedown[other] += size;
    sitesfilesdown[other] += 1;
    sizedown += size;
    ++filesdown;
  }
  else {
    if (sitessizeup.find(other) == sitessizeup.end()) {
      sitessizeup[other] = 0;
      sitesfilesup[other] = 0;
      sitesspeedup[other] = 1;
    }
    if (speed && size && size > speed) {
      sitesspeedup[other] = (sitessizeup[other] + size) / (sitessizeup[other] / sitesspeedup[other] + size / speed);
      if (!sitesspeedup[other]) {
        ++sitesspeedup[other];
      }
      speedup = (sizeup + size) / (sizeup / speedup + size / speed);
      if (!speedup) {
        ++speedup;
      }
    }
    sitessizeup[other] += size;
    sitesfilesup[other] += 1;
    sizeup += size;
    ++filesup;
  }
}

unsigned long long int SiteRace::getSizeDown() const {
  return sizedown;
}

unsigned int SiteRace::getFilesDown() const {
  return filesdown;
}

unsigned int SiteRace::getSpeedDown() const {
  return speeddown;
}

unsigned long long int SiteRace::getSizeUp() const {
  return sizeup;
}

unsigned int SiteRace::getFilesUp() const {
  return filesup;
}

unsigned int SiteRace::getSpeedUp() const {
  return speedup;
}

bool SiteRace::allListsRefreshed() const {
  for (const std::pair<const std::string, std::shared_ptr<FileList>>& filelist : filelists) {
    if (!isSubPathComplete(filelist.second) && filelist.second->getUpdateState() < UpdateState::REFRESHED) {
      return false;
    }
  }
  return true;
}

bool SiteRace::isDownloadOnly() const {
  return downloadonly;
}

std::unordered_map<std::string, unsigned long long int>::const_iterator SiteRace::sizeUpBegin() const {
  return sitessizeup.begin();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::filesUpBegin() const {
  return sitesfilesup.begin();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::speedUpBegin() const {
  return sitesspeedup.begin();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator SiteRace::sizeDownBegin() const {
  return sitessizedown.begin();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::filesDownBegin() const {
  return sitesfilesdown.begin();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::speedDownBegin() const {
  return sitesspeeddown.begin();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator SiteRace::sizeUpEnd() const {
  return sitessizeup.end();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::filesUpEnd() const {
  return sitesfilesup.end();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::speedUpEnd() const {
  return sitesspeedup.end();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator SiteRace::sizeDownEnd() const {
  return sitessizedown.end();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::filesDownEnd() const {
  return sitesfilesdown.end();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::speedDownEnd() const {
  return sitesspeeddown.end();
}
