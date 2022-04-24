#include "filelist.h"

#include <list>

#include "file.h"
#include "site.h"
#include "globalcontext.h"
#include "timereference.h"
#include "util.h"
#include "commandowner.h"

#include <cassert>

#define MAX_LIST_FAILURES_BEFORE_STATE_FAILED 3

FileList::FileList(const std::string & username, const Path & path) {
  init(username, path, FileListState::UNKNOWN);
}

FileList::FileList(const std::string & username, const Path & path, FileListState state) {
  init(username, path, state);
}

FileList::~FileList() {
  for (File * f : filesfirstseen) {
    delete f;
  }
}

void FileList::init(const std::string & username, const Path & path, FileListState state) {
  this->username = username;
  this->path = path;
  this->state = state;
  owned = 0;
  ownpercentage = 100;
  maxfilesize = 0;
  uploadedfiles = 0;
  locked = false;
  updatestate = UpdateState::NONE;
  scoreboardupdatestate = UpdateState::NONE;
  lastchangedstamp = 0;
  lastmetachangedstamp = 0;
  lastrefreshedstamp = 0;
  totalfilesize = 0;
  uploading = 0;
  listfailures = 0;
  inscoreboard = false;
  similarchecked = false;
  firstsimilar = nullptr;
}

bool FileList::updateFile(const std::string & start, int touch) {
  File * file = new File(start, touch);
  if (!file->isValid()) {
    delete file;
    return false;
  }
  std::string name = file->getName();
  if (name == "." || name == "..") {
    delete file;
    return false;
  }
  File * updatefile;
  if ((updatefile = getFileCaseSensitive(name)) != NULL) {
    unsigned long long int oldsize = updatefile->getSize();
    unsigned long long int newsize = file->getSize();
    if (updatefile->setSize(newsize)) {
      if (!updatefile->isDirectory()) {
        totalfilesize += newsize - oldsize;
        if (oldsize == 0 && newsize > 0) {
          uploadedfiles++;
        }
        else if (oldsize > 0 && newsize == 0) {
          uploadedfiles--;
        }
      }
      setChanged();
      scoreboardchangedfiles.insert(name);
    }
    if (newsize > maxfilesize) maxfilesize = newsize;
    if (!util::eightCharUserCompare(file->getOwner(), updatefile->getOwner())) {
      if (util::eightCharUserCompare(username, file->getOwner())) {
        owned++;
      }
      else if (util::eightCharUserCompare(username, updatefile->getOwner()))
      {
        owned--;
      }
      recalcOwnedPercentage();
    }
    if (updatefile->setOwner(file->getOwner())) {
      setMetaChanged();
      scoreboardchangedfiles.insert(name);
    }
    if (updatefile->setGroup(file->getGroup())) {
      setMetaChanged();
      // doesn't matter for the scoreboard
    }
    if (updatefile->setLastModified(file->getLastModified())) {
      setMetaChanged();
      // doesn't matter for the scoreboard
    }
    updatefile->setTouch(file->getTouch());
    if (updatefile->updateFlagSet()) {
      unsigned long long int size = oldsize;
      unsigned int speed = updatefile->getUpdateSpeed();
      const std::shared_ptr<Site> & src = updatefile->getUpdateSrc();
      const std::shared_ptr<Site> & dst = updatefile->getUpdateDst();
      if (util::eightCharUserCompare(username, updatefile->getOwner())) {
        size = newsize;
        std::string extension = updatefile->getExtension();
        if (extension != "sfv" && extension != "nfo") {
          updatefile->getUpdateSrc()->pushTransferSpeed(dst->getName(), speed, size);
        }
      }
      src->addTransferStatsFile(STATS_DOWN, dst->getName(), size);
      dst->addTransferStatsFile(STATS_UP, src->getName(), size);
      updatefile->getUpdateSrcCommandOwner()->addTransferStatsFile(STATS_DOWN, dst->getName(), size, speed);
      updatefile->getUpdateDstCommandOwner()->addTransferStatsFile(STATS_UP, src->getName(), size, speed);
      global->getStatistics()->addTransferStatsFile(STATS_FXP, size);
      updatefile->unsetUpdateFlag();
    }
    delete file;
  }
  else {
    filesfirstseen.push_back(file);
    files[name] = std::make_pair(file, --filesfirstseen.end());
    lowercasefilemap[util::toLower(name)] = name;
    if (similarchecked && !firstsimilar) {
      checkSimilarFile(file);
    }
    unsigned long long int filesize = file->getSize();
    if (filesize > 0 && !file->isDirectory()) {
      uploadedfiles++;
      totalfilesize += filesize;
    }
    if (filesize > maxfilesize) maxfilesize = filesize;
    if (util::eightCharUserCompare(username, file->getOwner())) {
      owned++;
    }
    recalcOwnedPercentage();
    setChanged();
    scoreboardchangedfiles.insert(name);
  }
  return true;
}

void FileList::touchFile(const std::string & name, const std::string & user, bool upload) {
  File * file;
  if ((file = getFileCaseSensitive(name)) != NULL) {
    file->unsetUpdateFlag();
  }
  else {
    file = new File(name, user);
    filesfirstseen.push_back(file);
    files[name] = std::make_pair(file, --filesfirstseen.end());
    lowercasefilemap[util::toLower(name)] = name;
    if (similarchecked && !firstsimilar) {
      checkSimilarFile(file);
    }
    if (util::eightCharUserCompare(username, user)) {
      owned++;
    }
    recalcOwnedPercentage();
    setChanged();
    scoreboardchangedfiles.insert(name);
  }
  if (upload && !file->isUploading()) {
    file->upload();
    uploading++;
  }
}

void FileList::removeFile(const std::string & name) {
  std::unordered_map<std::string, std::pair<File *, std::list<File *>::iterator>>::iterator it = files.find(name);
  if (it == files.end()) {
    return;
  }
  File * f = it->second.first;
  if (util::eightCharUserCompare(username, f->getOwner())) {
    owned--;
  }
  if (f->getSize() > 0 && !f->isDirectory()) {
    uploadedfiles--;
    totalfilesize -= f->getSize();
  }
  if (f->isUploading()) {
    uploading--;
  }
  filesfirstseen.erase(it->second.second);
  files.erase(it);
  lowercasefilemap.erase(util::toLower(name));
  if (similarchecked && f == firstsimilar) {
    similarchecked = false;
    f = nullptr;
    checkSimilar(siteskip, sectionskip);
  }
  delete f;
  recalcOwnedPercentage();
  setChanged();
  scoreboardchangedfiles.insert(name);
}

void FileList::setFileUpdateFlag(const std::string & name,
    unsigned long long int size, unsigned int speed, const std::shared_ptr<Site> & src,
    const std::shared_ptr<Site> & dst, const std::shared_ptr<CommandOwner> & srcco, const std::shared_ptr<CommandOwner> & dstco)
{
  File * file;
  if ((file = getFileCaseSensitive(name)) != NULL) {
    unsigned long long int oldsize = file->getSize();
    if (file->setSize(size)) {
      if (!oldsize) {
        uploadedfiles++;
      }
      totalfilesize += size - oldsize;
      if (size > maxfilesize) {
        maxfilesize = size;
      }
      setChanged();
      scoreboardchangedfiles.insert(name);
    }
    file->setUpdateFlag(src, dst, srcco, dstco, speed);
  }
}

File * FileList::getFile(const std::string & name) const {
  std::unordered_map<std::string, std::pair<File *, std::list<File *>::iterator>>::const_iterator it = files.find(name);
  if (it == files.end()) {
    std::unordered_map<std::string, std::string>::const_iterator it2 = lowercasefilemap.find(util::toLower(name));
    if (it2 != lowercasefilemap.end()) {
      it = files.find(it2->second);
    }
    if (it == files.end()) {
      return NULL;
    }
  }
  return it->second.first;
}

File * FileList::getFileCaseSensitive(const std::string & name) const {
  std::unordered_map<std::string, std::pair<File *, std::list<File *>::iterator>>::const_iterator it = files.find(name);
  if (it == files.end()) {
    return NULL;
  }
  return it->second.first;
}

FileListState FileList::getState() const {
  return state;
}

void FileList::setNonExistent() {
  assert(state == FileListState::UNKNOWN);
  state = FileListState::NONEXISTENT;
}

void FileList::setExists() {
  state = FileListState::EXISTS;
}

void FileList::setFilled() {
  state = FileListState::LISTED;
}

void FileList::setPreFailed() {
  state = FileListState::PRE_FAIL;
}

void FileList::setFailed() {
  if (state != FileListState::FAILED) {
    setChanged();
  }
  state = FileListState::FAILED;
}

void FileList::setRefreshed() {
  lastrefreshedstamp = global->getTimeReference()->timeReference();
  bumpUpdateState(UpdateState::REFRESHED);
}

std::list<File *>::iterator FileList::begin() {
  return filesfirstseen.begin();
}

std::list<File *>::iterator FileList::end() {
  return filesfirstseen.end();
}

std::list<File *>::const_iterator FileList::begin() const {
  return filesfirstseen.begin();
}

std::list<File *>::const_iterator FileList::end() const {
  return filesfirstseen.end();
}

bool FileList::contains(const std::string& name) const {
  if (files.find(name) != files.end()) {
    return true;
  }
  std::unordered_map<std::string, std::string>::const_iterator it = lowercasefilemap.find(util::toLower(name));
  if (it != lowercasefilemap.end() && files.find(it->second) != files.end()) {
    return true;
  }
  return false;
}

bool FileList::containsPattern(const std::string& pattern, const Path& path, bool dir) const {
  if (path.empty()) {
    for (File* f : filesfirstseen) {
      if (f->isDirectory() == dir && util::wildcmp(pattern.c_str(), f->getName().c_str())) {
        return true;
      }
    }
  }
  else {
    for (File* f : filesfirstseen) {
      if (f->isDirectory() == dir && util::wildcmp(pattern.c_str(), (path.dirName() + "/" + f->getName()).c_str())) {
        return true;
      }
    }
  }
  return false;
}

bool FileList::containsRegexPattern(const std::regex& pattern, const Path& path, bool dir) const {
  if (path.empty()) {
    for (File* f : filesfirstseen) {
      if (f->isDirectory() == dir && std::regex_match(f->getName(), pattern)) {
        return true;
      }
    }
  }
  else {
    for (File* f : filesfirstseen) {
      if (f->isDirectory() == dir && std::regex_match(path.dirName() + "/" + f->getName(), pattern)) {
        return true;
      }
    }
  }
  return false;
}

bool FileList::containsPatternBefore(const std::string& pattern, bool dir, const std::string& item) const {
  size_t slashpos = pattern.rfind('/');
  const std::string* usedpattern = &pattern;
  std::string newpattern;
  if (slashpos != std::string::npos) {
    newpattern = pattern.substr(slashpos + 1);
    usedpattern = &newpattern;
  }
  for (File* f : filesfirstseen) {
    const std::string& name = f->getName();
    if (f->isDirectory() != dir) {
      continue;
    }
    if (name == item) {
      return false;
    }
    if (util::wildcmp(usedpattern->c_str(), name.c_str())) {
      return true;
    }
  }
  return false;
}

bool FileList::containsRegexPatternBefore(const std::regex& pattern, bool dir, const std::string& item) const {
  for (File * f : filesfirstseen) {
    const std::string& name = f->getName();
    if (f->isDirectory() != dir) {
      continue;
    }
    if (name == item) {
      return false;
    }
    if (std::regex_match(name, pattern)) {
      return true;
    }
  }
  return false;
}

bool FileList::containsUnsimilar(const std::string & item) const {
  assert (similarchecked);
  if (!firstsimilar) {
    return false;
  }
  const std::string & similar = firstsimilar->getName();
  return checkUnsimilar(item, similar);
}

bool FileList::checkUnsimilar(const std::string & item, const std::string & similar) {
  if (item.empty() || similar.empty()) {
    return false;
  }
  size_t itemlen = item.length();
  size_t similarlen = similar.length();
  size_t itemextensionpos = item.rfind(".");
  size_t similarextensionpos = similar.rfind(".");
  if (itemextensionpos == std::string::npos) {
    itemextensionpos = itemlen;
  }
  if (similarextensionpos == std::string::npos) {
    similarextensionpos = similarlen;
  }
  bool numberingdiff = false;
  bool numberingdiffpassed = false;
  size_t itempos = 0;
  size_t similarpos = 0;
  while (itempos < itemextensionpos || similarpos < similarextensionpos) {
    if (itempos == itemextensionpos) {
      if (!numberingdiffpassed && isdigit(similar[similarpos])) {
        numberingdiff = true;
        ++similarpos;
        continue;
      }
      return true;
    }
    if (similarpos == similarextensionpos) {
      if (!numberingdiffpassed && isdigit(item[itempos])) {
        numberingdiff = true;
        ++itempos;
        continue;
      }
      return true;
    }
    if (isdigit(item[itempos]) && numberingdiffpassed) {
      return true;
    }
    if (item[itempos] == similar[similarpos]) {
      itempos++;
      similarpos++;
    }
    else {
      if (numberingdiffpassed) {
        return true;
      }
      bool either = false;
      if (isdigit(item[itempos])) {
        itempos++;
        either = true;
      }
      if (isdigit(similar[similarpos])) {
        similarpos++;
        either = true;
      }
      if (either && !numberingdiff) {
        numberingdiff = true;
      }
      if (!either) {
        if (numberingdiff) {
          numberingdiffpassed = true;
        }
        else if (itempos + 1 == itemextensionpos &&
            similarpos + 1 == similarextensionpos)
        {
          numberingdiff = true;
          itempos++;
          similarpos++;
        }
        else {
          return true;
        }
      }
    }
  }
  bool extensiondiff = false;
  while (itempos < itemlen && similarpos < similarlen) {
    if (item[itempos++] != similar[similarpos++]) {
      extensiondiff = true;
      break;
    }
  }
  if (!extensiondiff && (itempos < itemlen || similarpos < similarlen)) {
    extensiondiff = true;
  }
  if (numberingdiff && extensiondiff) {
    return true;
  }
  return false;
}

unsigned int FileList::getSize() const {
  return files.size();
}

unsigned long long int FileList::getTotalFileSize() const {
  return totalfilesize;
}

unsigned int FileList::getNumUploadedFiles() const {
  return uploadedfiles;
}

bool FileList::hasExtension(const std::string& extension) const {
  std::list<File *>::const_iterator it;
  for (File * file : filesfirstseen) {
    if(file->getExtension() == extension && file->getSize() > 0) {
      return true;
    }
  }
  return false;
}

int FileList::getOwnedPercentage() const {
  return ownpercentage;
}

unsigned long long int FileList::getMaxFileSize() const {
  return maxfilesize;
}

const Path & FileList::getPath() const {
  return path;
}

void FileList::cleanSweep(int touch) {
  std::list<std::string> eraselist;
  for (File * f : filesfirstseen) {
    if (f->getTouch() != touch && !f->isUploading()) {
      eraselist.push_back(f->getName());
    }
  }
  if (!eraselist.empty()) {
    for (const std::string& file : eraselist) {
      removeFile(file);
    }
    maxfilesize = 0;
    for (File * f : filesfirstseen) {
      if (!f->isDirectory() && f->getSize() > maxfilesize) {
        maxfilesize = f->getSize();
      }
    }
    setChanged();
  }
}

void FileList::flush() {
  for (File * f : filesfirstseen) {
    delete f;
  }
  files.clear();
  lowercasefilemap.clear();
  filesfirstseen.clear();
  maxfilesize = 0;
  totalfilesize = 0;
  uploadedfiles = 0;  
  owned = 0;
  ownpercentage = 100;
  uploading = 0;
  similarchecked = false;
  firstsimilar = nullptr;
}

void FileList::recalcOwnedPercentage() {
 if (!files.empty()) {
   ownpercentage = (owned * 100) / files.size();
 }
 else {
   ownpercentage = 100;
 }
}

UpdateState FileList::getUpdateState() const {
  return updatestate;
}

UpdateState FileList::getScoreBoardUpdateState() const {
  return scoreboardupdatestate;
}

void FileList::resetUpdateState() {
  updatestate = UpdateState::NONE;
}

void FileList::resetScoreBoardUpdates() {
  scoreboardupdatestate = UpdateState::NONE;
  scoreboardchangedfiles.clear();
}

bool FileList::bumpUpdateState(const UpdateState newstate) {
  if (newstate > scoreboardupdatestate) {
    scoreboardupdatestate = newstate;
  }
  if (newstate > updatestate) {
    updatestate = newstate;
    return true;
  }
  return false;
}

void FileList::setChanged() {
  bumpUpdateState(UpdateState::CHANGED);
  lastchangedstamp = global->getTimeReference()->timeReference();
  lastmetachangedstamp = lastchangedstamp;
  listfailures = 0;
}

void FileList::setMetaChanged() {
  bumpUpdateState(UpdateState::META_CHANGED);
  lastmetachangedstamp = global->getTimeReference()->timeReference();
}

unsigned long long FileList::timeSinceLastChanged() const {
  return global->getTimeReference()->timePassedSince(lastchangedstamp);
}

unsigned long long FileList::timeSinceLastMetaChanged() const {
  return global->getTimeReference()->timePassedSince(lastmetachangedstamp);
}

unsigned long long FileList::timeSinceLastRefreshed() const {
  return global->getTimeReference()->timePassedSince(lastrefreshedstamp);
}

std::string FileList::getUser() const {
  return username;
}

void FileList::finishUpload(const std::string & file) {
  File * fileobj = getFileCaseSensitive(file);
  if (fileobj != NULL && fileobj->isUploading()) {
    fileobj->finishUpload();
    uploading--;
    setChanged();
    // irrelevant to the scoreboard
  }
}

void FileList::finishDownload(const std::string & file) {
  File * fileobj = getFileCaseSensitive(file);
  if (fileobj != NULL && fileobj->isDownloading()) {
    fileobj->finishDownload();
  }
}

void FileList::download(const std::string & file) {
  File * fileobj = getFileCaseSensitive(file);
  if (fileobj != NULL && !fileobj->isDownloading()) {
    fileobj->download();
  }
}

bool FileList::hasFilesUploading() const {
  return uploading > 0;
}

bool FileList::addListFailure() {
  if (++listfailures >= MAX_LIST_FAILURES_BEFORE_STATE_FAILED) {
    setFailed();
    return true;
  }
  return false;
}

std::unordered_set<std::string>::const_iterator FileList::scoreBoardChangedFilesBegin() const {
  return scoreboardchangedfiles.begin();
}

std::unordered_set<std::string>::const_iterator FileList::scoreBoardChangedFilesEnd() const {
  return scoreboardchangedfiles.end();
}

bool FileList::scoreBoardChangedFilesEmpty() const {
  return scoreboardchangedfiles.empty();
}

bool FileList::inScoreBoard() const {
  return inscoreboard;
}

void FileList::setInScoreBoard() {
  inscoreboard = true;
}

void FileList::unsetInScoreBoard() {
  inscoreboard = false;
}

bool FileList::similarChecked() const {
  return similarchecked;
}

void FileList::checkSimilar(const SkipList * siteskip, const SkipList * sectionskip) {
  similarchecked = true;
  this->siteskip = siteskip;
  this->sectionskip = sectionskip;
  for (File * f : filesfirstseen) {
    if (checkSimilarFile(f)) {
      return;
    }
  }
}

bool FileList::checkSimilarFile(File * f) {
  if (!siteskip) {
    return false;
  }
  SkipListMatch match = siteskip->check(f->getName(), f->isDirectory(), true, sectionskip);
  if (match.action == SKIPLIST_SIMILAR) {
    firstsimilar = f;
    return true;
  }
  return false;
}

std::string FileList::getFirstSimilar() const {
  if (firstsimilar) {
    return firstsimilar->getName();
  }
  return "";
}
