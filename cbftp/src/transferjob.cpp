#include "transferjob.h"

#include <cassert>

#include "core/tickpoke.h"
#include "filelist.h"
#include "file.h"
#include "site.h"
#include "globalcontext.h"
#include "skiplist.h"
#include "transferstatus.h"
#include "sitelogic.h"
#include "util.h"
#include "localstorage.h"
#include "localfilelist.h"
#include "localfile.h"
#include "filesystem.h"
#include "section.h"
#include "sectionmanager.h"
#include "sitetransferjob.h"
#include "eventlog.h"

#define MAX_TRANSFER_ATTEMPTS_BEFORE_SKIP 3

enum RefreshState {
  REFRESH_NOW,
  REFRESH_OK,
  REFRESH_FINAL_NOW,
  REFRESH_FINAL_OK
};

TransferJob::TransferJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcfile, const Path& dstpath, const std::string& dstfile) {
  downloadJob(id, srcsl, srcfilelist, "", srcfile, dstpath, dstfile);
}

TransferJob::TransferJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const Path& srcpath, const std::string& srcsection, const std::string& srcfile, const Path& dstpath, const std::string& dstfile) {
  std::shared_ptr<FileList> srcfilelist = std::make_shared<FileList>(srcsl->getSite()->getUser(), srcpath);
  downloadJob(id, srcsl, srcfilelist, srcsection, srcfile, dstpath, dstfile);
}

void TransferJob::downloadJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcsection, const std::string& srcfile, const Path& dstpath, const std::string& dstfile) {
  init(id, TRANSFERJOB_DOWNLOAD, srcsl, std::shared_ptr<SiteLogic>(), srcfilelist->getPath(), dstpath, srcsection, "", srcfile, dstfile);
  srcfilelists[""] = srcfilelist;
  updateLocalFileLists();
  if (srcfilelist->getState() == FileListState::LISTED) {
    fileListUpdated(true, srcfilelist);
  }
  else {
    filelistsrefreshed[srcfilelist] = REFRESH_NOW;
    srcfilelist->resetUpdateState();
  }
}

TransferJob::TransferJob(unsigned int id, const Path& srcpath, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstfile) {
  uploadJob(id, srcpath, srcfile, dstsl, dstfilelist, "", dstfile);
}

TransferJob::TransferJob(unsigned int id, const Path& srcpath, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const Path& dstpath, const std::string& dstsection, const std::string& dstfile) {
  std::shared_ptr<FileList> dstfilelist = std::make_shared<FileList>(dstsl->getSite()->getUser(), dstpath);
  uploadJob(id, srcpath, srcfile, dstsl, dstfilelist, dstsection, dstfile);
}

void TransferJob::uploadJob(unsigned int id, const Path& srcpath, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstsection, const std::string& dstfile) {
  init(id, TRANSFERJOB_UPLOAD, std::shared_ptr<SiteLogic>(), dstsl, srcpath, dstfilelist->getPath(), "", dstsection, srcfile, dstfile);
  dstfilelists[""] = dstfilelist;
  updateLocalFileLists();
  if (dstfilelist->getState() == FileListState::LISTED) {
    fileListUpdated(false, dstfilelist);
  }
  else {
    filelistsrefreshed[dstfilelist] = REFRESH_NOW;
    dstfilelist->resetUpdateState();
  }
}

TransferJob::TransferJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstfile) {
  fxpJob(id, srcsl, srcfilelist, "", srcfile, dstsl, dstfilelist, "", dstfile);
}

TransferJob::TransferJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const Path& srcpath, const std::string& srcsection, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const Path& dstpath, const std::string& dstsection, const std::string& dstfile) {
  std::shared_ptr<FileList> srcfilelist = std::make_shared<FileList>(srcsl->getSite()->getUser(), srcpath);
  std::shared_ptr<FileList> dstfilelist = std::make_shared<FileList>(dstsl->getSite()->getUser(), dstpath);
  fxpJob(id, srcsl, srcfilelist, srcsection, srcfile, dstsl, dstfilelist, dstsection, dstfile);
}

void TransferJob::fxpJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcsection, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstsection, const std::string& dstfile) {
  init(id, TRANSFERJOB_FXP, srcsl, dstsl, srcfilelist->getPath(), dstfilelist->getPath(), srcsection, dstsection, srcfile, dstfile);
  srcfilelists[""] = srcfilelist;
  dstfilelists[""] = dstfilelist;
  if (srcfilelist->getState() == FileListState::LISTED) {
    fileListUpdated(true, srcfilelist);
  }
  else {
    filelistsrefreshed[srcfilelist] = REFRESH_NOW;
    srcfilelist->resetUpdateState();

  }
  if (dstfilelist->getState() == FileListState::LISTED) {
    fileListUpdated(false, dstfilelist);
  }
  else {
    filelistsrefreshed[dstfilelist] = REFRESH_NOW;
    dstfilelist->resetUpdateState();
  }
}

TransferJob::~TransferJob() {
  if (!isDone()) {
    setDone();
  }
}

void TransferJob::createSiteTransferJobs(const std::shared_ptr<TransferJob>& tj) {
  if (!!src) {
    srcsitetransferjob = std::make_shared<SiteTransferJob>(tj, true);
  }
  if (!!dst) {
    dstsitetransferjob = std::make_shared<SiteTransferJob>(tj, false);
  }
}

std::string TransferJob::getName() const {
  return dstfile;
}

const Path & TransferJob::getSrcPath() const {
  return srcpath;
}

const Path & TransferJob::getDstPath() const {
  return dstpath;
}

const Path & TransferJob::getPath(bool source) const {
  if (source) {
    return srcpath;
  }
  else {
    return dstpath;
  }
}

std::string TransferJob::getSrcSection() const {
  return srcsection;
}

std::string TransferJob::getDstSection() const {
  return dstsection;
}

std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator TransferJob::srcFileListsBegin() const {
  return srcfilelists.begin();
}

std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator TransferJob::srcFileListsEnd() const {
  return srcfilelists.end();
}

std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator TransferJob::dstFileListsBegin() const {
  return dstfilelists.begin();
}

std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator TransferJob::dstFileListsEnd() const {
  return dstfilelists.end();
}

std::unordered_map<std::string, std::shared_ptr<LocalFileList>>::const_iterator TransferJob::localFileListsBegin() const {
  return localfilelists.begin();
}

std::unordered_map<std::string, std::shared_ptr<LocalFileList>>::const_iterator TransferJob::localFileListsEnd() const {
  return localfilelists.end();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator TransferJob::pendingTransfersBegin() const {
  return pendingtransfers.begin();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator TransferJob::pendingTransfersEnd() const {
  return pendingtransfers.end();
}

std::list<std::shared_ptr<TransferStatus>>::const_iterator TransferJob::transfersBegin() const {
  return transfers.begin();
}

std::list<std::shared_ptr<TransferStatus>>::const_iterator TransferJob::transfersEnd() const {
  return transfers.end();
}

void TransferJob::init(unsigned int id, TransferJobType type, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<SiteLogic>& dstsl, const Path& srcpath, const Path& dstpath, const std::string& srcsection, const std::string& dstsection, const std::string& srcfile, const std::string& dstfile) {
  this->id = id;
  this->type = type;
  this->src = srcsl;
  this->dst = dstsl;
  this->srcpath = srcpath;
  this->dstpath = dstpath;
  this->srcsection = srcsection;
  this->dstsection = dstsection;
  this->srcfile = srcfile;
  this->dstfile = dstfile;
  resetValues();
  dstsectionskiplist = nullptr;
  if (!dstsection.empty()) {
    Section* section = global->getSectionManager()->getSection(dstsection);
    if (section != nullptr) {
      dstsectionskiplist = &section->getSkipList();
    }
  }

}

std::string TransferJob::getSrcFileName() const {
  return srcfile;
}

std::string TransferJob::getDstFileName() const {
  return dstfile;
}

CallbackType TransferJob::callbackType() const {
  return CallbackType::TRANSFERJOB;
}

int TransferJob::getType() const {
  return type;
}

TransferJobStatus TransferJob::getStatus() const {
  return status;
}

bool TransferJob::isDone() const {
  return status == TRANSFERJOB_DONE || status == TRANSFERJOB_ABORTED;
}

bool TransferJob::isDirectory() const {
  if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_DOWNLOAD) {
    File* f = srcfilelists.at("")->getFile(srcfile);
    return f != nullptr && f->isDirectory();
  }
  auto it = localfilelists.at("")->find(dstfile);
  return it != localfilelists.at("")->end() && it->second.isDirectory();
}

bool TransferJob::wantsList(bool source) {
  if (source && (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP)) {
    for (std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
      std::unordered_map<std::shared_ptr<FileList>, int>::iterator it2 = filelistsrefreshed.find(it->second);
      FileListState state = it->second->getState();
      if (state != FileListState::FAILED &&
          ((state != FileListState::LISTED && state != FileListState::NONEXISTENT) || (it2 != filelistsrefreshed.end() &&
          (it2->second == REFRESH_NOW || it2->second == REFRESH_FINAL_NOW))))
      {
        srclisttarget = it->second;
        return true;
      }
    }
  }
  else if (!source && (type == TRANSFERJOB_UPLOAD || type == TRANSFERJOB_FXP)) {
    for (std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
      std::unordered_map<std::shared_ptr<FileList>, int>::iterator it2 = filelistsrefreshed.find(it->second);
      FileListState state = it->second->getState();
      if (state != FileListState::FAILED &&
          ((state != FileListState::LISTED && state != FileListState::NONEXISTENT) || (it2 != filelistsrefreshed.end() &&
          (it2->second == REFRESH_NOW || it2->second == REFRESH_FINAL_NOW))))
      {
        dstlisttarget = it->second;
        return true;
      }
    }
  }
  if (!filestotal) {
    countTotalFiles();
  }
  return false;
}

std::shared_ptr<LocalFileList> TransferJob::wantedLocalDstList(const std::string& subdir) {
  if (localfilelists.find(subdir) == localfilelists.end()) {
    localfilelists[subdir] = std::make_shared<LocalFileList>(dstpath / subdir);
  }
  return localfilelists.at(subdir);
}

std::shared_ptr<FileList> TransferJob::getListTarget(bool source) const {
  if (source) {
    return srclisttarget;
  }
  return dstlisttarget;
}

bool TransferJob::checkFileListExists(const std::shared_ptr<FileList>& fl) const {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it;
  for (it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
    if (it->second == fl) {
      return true;
    }
  }
  for (it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
    if (it->second == fl) {
      return true;
    }
  }
  return false;
}

void TransferJob::fileListUpdated(bool source, const std::shared_ptr<FileList>& fl) {
  if (!checkFileListExists(fl)) {
    return;
  }
  std::unordered_map<std::shared_ptr<FileList>, int>::iterator it = filelistsrefreshed.find(fl);
  if (it == filelistsrefreshed.end()) {

    filelistsrefreshed[fl] = REFRESH_NOW;
    it = filelistsrefreshed.find(fl);
  }
  if (fl->getState() == FileListState::NONEXISTENT || fl->getState() == FileListState::LISTED || fl->getState() == FileListState::FAILED) {
    if (it->second == REFRESH_FINAL_NOW) {
      it->second = REFRESH_FINAL_OK;
    }
    else if (it->second == REFRESH_NOW) {
      it->second = REFRESH_OK;
    }
  }
  if (!anyListNeedsRefreshing()) {
    countTotalFiles();
  }
  Path subpath = fl->getPath() - (source ? srcpath : dstpath);
  if (subpath == "") {
    File * file = fl->getFile(source ? srcfile : dstfile);
    if (file != NULL) {
      if (source && file->isLink()) {
        Path linktarget(file->getLinkTarget());
        if (linktarget.isRelative()) {
          linktarget = fl->getPath() / linktarget;
        }
        srcpath = linktarget.dirName();
        srcfile = linktarget.baseName();
        std::shared_ptr<FileList> srcfilelist = std::make_shared<FileList>(src->getSite()->getUser(), srcpath);
        srcfilelists[""] = srcfilelist;
        filelistsrefreshed.clear();
        filelistsrefreshed[srcfilelist] = REFRESH_NOW;
        return;
      }
      if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_DOWNLOAD) {
        addSubDirectoryFileLists(srcfilelists, fl, subpath, file);
      }
      if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_UPLOAD) {
        addSubDirectoryFileLists(dstfilelists, fl, subpath, file);
      }
    }
  }
  else {
    if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_DOWNLOAD) {
      addSubDirectoryFileLists(srcfilelists, fl, subpath);
    }
    if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_UPLOAD) {
      addSubDirectoryFileLists(dstfilelists, fl, subpath);
    }
  }
}

std::shared_ptr<FileList> TransferJob::findDstList(const std::string& sub) const {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it = dstfilelists.find(sub);
  if (it != dstfilelists.end()) {
    return it->second;
  }
  return NULL;
}

std::shared_ptr<FileList> TransferJob::getFileListForFullPath(bool source, const Path& path) const {
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator it;
  if (source) {
    for (it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
      if (it->second->getPath() == path) {
        return it->second;
      }
    }
  }
  else {
    for (it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
      if (it->second->getPath() == path) {
        return it->second;
      }
    }
  }
  return nullptr;
}

std::shared_ptr<LocalFileList> TransferJob::findLocalFileList(const std::string& subpath) const {
  std::unordered_map<std::string, std::shared_ptr<LocalFileList> >::const_iterator it = localfilelists.find(subpath);
  if (it != localfilelists.end()) {
    return it->second;
  }
  return nullptr;
}

void TransferJob::addSubDirectoryFileLists(std::unordered_map<std::string, std::shared_ptr<FileList>>& filelists, const std::shared_ptr<FileList>& fl, const Path& subpath) {
  std::list<File*>::iterator it;
  for(it = fl->begin(); it != fl->end(); it++) {
    File* file = *it;
    if (!file->isDirectory()) {
      continue;
    }
    SkipListMatch match = !!dst ? dst->getSite()->getSkipList().check(file->getName(), true, false, dstsectionskiplist)
                                : global->getSkipList()->check(file->getName(), true, false);
    if (match.action == SKIPLIST_DENY || (match.action == SKIPLIST_UNIQUE &&
                                          fl->containsPatternBefore(match.matchpattern, true, file->getName())))
    {
      continue;
    }
    addSubDirectoryFileLists(filelists, fl, subpath, file);
  }
}

void TransferJob::addSubDirectoryFileLists(std::unordered_map<std::string, std::shared_ptr<FileList>>& filelists, const std::shared_ptr<FileList>& fl, const Path& subpath, File* file) {
  if (!file->isDirectory()) {
    return;
  }
  std::string subpathfile = (subpath / file->getName()).toString();
  if (filelists.find(subpathfile) == filelists.end()) {
    std::shared_ptr<FileList> newfl = std::make_shared<FileList>(filelists[""]->getUser(), filelists[""]->getPath() / subpathfile);
    filelists[subpathfile] = newfl;
    filelistsrefreshed[newfl] = REFRESH_NOW;
  }
}

const std::shared_ptr<SiteLogic>& TransferJob::getSrc() const {
  return src;
}

const std::shared_ptr<SiteLogic>& TransferJob::getDst() const {
  return dst;
}

int TransferJob::maxSlots() const {
  return slots;
}

void TransferJob::setSlots(int slots) {
  this->slots = slots;
}

int TransferJob::maxPossibleSlots() const {
  switch (type) {
    case TRANSFERJOB_UPLOAD:
      return dst->getSite()->getMaxUp();
    case TRANSFERJOB_FXP:
    case TRANSFERJOB_DOWNLOAD:
      return src->getSite()->getMaxDownTransferJob();
  }
  return 0;
}

bool TransferJob::refreshOrAlmostDone() {
  if (almostdone) {
    return false;
  }
  bool allfinaldone = true;
  bool finalset = false;
  for (std::unordered_map<std::shared_ptr<FileList>, int>::iterator it = filelistsrefreshed.begin(); it != filelistsrefreshed.end(); it++) {
    if (it->second != REFRESH_FINAL_OK && it->first->getState() != FileListState::FAILED) {
      allfinaldone = false;
    }
    if (it->second == REFRESH_FINAL_OK) {
      finalset = true;
    }
  }
  if (finalset && allfinaldone) {
    almostdone = true;
    if (status == TRANSFERJOB_QUEUED) {
      start();
    }
    return false;
  }
  for (std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
    std::unordered_map<std::shared_ptr<FileList>, int>::iterator it2 = filelistsrefreshed.find(it->second);
    if (it2 == filelistsrefreshed.end() || it2->second != REFRESH_FINAL_OK) {
      filelistsrefreshed[it->second] = REFRESH_FINAL_NOW;
      it->second->resetUpdateState();
    }
  }
  for (std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
      std::unordered_map<std::shared_ptr<FileList>, int>::iterator it2 = filelistsrefreshed.find(it->second);
    if (it2 == filelistsrefreshed.end() || it2->second != REFRESH_FINAL_OK) {
      filelistsrefreshed[it->second] = REFRESH_FINAL_NOW;
      it->second->resetUpdateState();
    }
  }
  if (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_UPLOAD) {
    updateLocalFileLists();
  }
  return true;
}

bool TransferJob::anyListNeedsRefreshing() const {
  for (std::unordered_map<std::shared_ptr<FileList>, int>::const_iterator it = filelistsrefreshed.begin(); it != filelistsrefreshed.end(); it++) {
    if (it->first->getState() != FileListState::FAILED && (it->second == REFRESH_NOW || it->second == REFRESH_FINAL_NOW)) {
      return true;
    }
  }
  return false;
}

void TransferJob::clearRefreshLists() {
  filelistsrefreshed.clear();
}

void TransferJob::start() {
  if (status != TRANSFERJOB_QUEUED) {
    return;
  }
  timestarted = util::ctimeLog();
  status = TRANSFERJOB_RUNNING;
  global->getTickPoke()->startPoke(this, "TransferJob", TRANSFERJOB_UPDATE_INTERVAL, 0);
}

void TransferJob::addPendingTransfer(const Path& name, unsigned long long int size) {
  pendingtransfers[name.toString()] = size;
}

void TransferJob::addTransfer(const std::shared_ptr<TransferStatus>& ts) {
  if (!!ts) {
    idletime = 0;
    transfers.push_front(ts);
    if (ts->getState() != TRANSFERSTATUS_STATE_FAILED) {
      ts->setCallback(this);
      Path subpathfile = (ts->getSourcePath() - srcpath) / ts->getFile();
      if (pendingtransfers.find(subpathfile.toString()) != pendingtransfers.end()) {
        pendingtransfers.erase(subpathfile.toString());
      }
    }
  }
}

void TransferJob::targetExists(const Path& target) {
  existingtargets.insert(target.toString());
}

void TransferJob::tick(int message) {
  updateStatus();
  idletime += TRANSFERJOB_UPDATE_INTERVAL;
  timespentmillis += TRANSFERJOB_UPDATE_INTERVAL;
  timespentsecs = timespentmillis / 1000;
}

void TransferJob::updateStatus() {
  unsigned long long int aggregatedsize = 0;
  unsigned long long int aggregatedsizetransferred = 0;
  bool ongoingtransfers = false;
  std::unordered_set<std::string> dstpaths;
  std::unordered_set<std::string> filescompleted;
  std::unordered_set<std::string> filesfailed;
  for (std::list<std::shared_ptr<TransferStatus> >::const_iterator it = transfersBegin(); it != transfersEnd(); it++) {
    if (pendingtransfers.find((*it)->getFile()) != pendingtransfers.end()) {
      pendingtransfers.erase((*it)->getFile());
    }
    int state = (*it)->getState();
    if (state == TRANSFERSTATUS_STATE_IN_PROGRESS) {
      ongoingtransfers = true;
    }
    std::string dstpath = ((*it)->getTargetPath() / (*it)->getFile()).toString();
    if (dstpaths.find(dstpath) != dstpaths.end()) {
      continue;
    }
    dstpaths.insert(dstpath);
    if (state == TRANSFERSTATUS_STATE_IN_PROGRESS || state == TRANSFERSTATUS_STATE_SUCCESSFUL) {
      aggregatedsize += (*it)->sourceSize();
      aggregatedsizetransferred += (*it)->targetSize();
    }
    Path subpathfile = ((*it)->getSourcePath() - srcpath) / (*it)->getFile();
    if (state == TRANSFERSTATUS_STATE_SUCCESSFUL) {
      if (existingtargets.find(subpathfile.toString()) == existingtargets.end()) {
        filescompleted.insert(subpathfile.toString());
      }
    }
    if (state == TRANSFERSTATUS_STATE_FAILED || state == TRANSFERSTATUS_STATE_DUPE) {
      filesfailed.insert(subpathfile.toString());
    }
  }
  for (std::unordered_map<std::string, unsigned long long int>::const_iterator it = pendingTransfersBegin(); it != pendingTransfersEnd(); it++) {
    aggregatedsize += it->second;
  }
  for (std::unordered_set<std::string>::const_iterator it = filescompleted.begin(); it != filescompleted.end(); it++) {
    if (filesfailed.find(*it) != filesfailed.end()) {
      filesfailed.erase(*it);
    }
  }
  expectedfinalsize = aggregatedsize;
  sizeprogress = aggregatedsizetransferred;
  if (expectedfinalsize) {
    progress = (100 * sizeprogress) / expectedfinalsize;
    milliprogress = (1000 * sizeprogress) / expectedfinalsize;
  }
  if (timespentmillis) {
    speed = sizeprogress / timespentmillis;
  }
  if (speed) {
    timeremaining = (expectedfinalsize - sizeprogress) / (speed * 1024);
  }
  filesprogress = existingtargets.size() + filescompleted.size();
  if (almostdone && !ongoingtransfers && filesprogress + (int)filesfailed.size() >= filestotal) {
    setDone();
  }
}

int TransferJob::getProgress() const {
  return progress;
}

int TransferJob::getMilliProgress() const {
  return milliprogress;
}

int TransferJob::timeSpent() const {
  return timespentsecs;
}

int TransferJob::timeRemaining() const {
  return timeremaining;
}

unsigned long long int TransferJob::sizeProgress() const {
  return sizeprogress;
}

unsigned long long int TransferJob::totalSize() const {
  return expectedfinalsize;
}

unsigned int TransferJob::getSpeed() const {
  return speed;
}

std::string TransferJob::timeQueued() const {
  return timequeued;
}

std::string TransferJob::timeStarted() const {
  return timestarted;
}

std::string TransferJob::typeString() const {
  std::string type;
  switch (this->type) {
    case TRANSFERJOB_DOWNLOAD:
      type = "Download";
      break;
    case TRANSFERJOB_UPLOAD:
      type = "Upload";
      break;
    case TRANSFERJOB_FXP:
      type = "FXP";
      break;
  }
  return type;
}

int TransferJob::filesProgress() const {
  return filesprogress;
}

int TransferJob::filesTotal() const {
  return filestotal;
}

void TransferJob::countTotalFiles() {
  switch (type) {
    case TRANSFERJOB_DOWNLOAD:
    {
      int files = 0;
      for (std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
        if (it->first == "") {
          File* f = it->second->getFile(srcfile);
          if (f != NULL && !f->isDirectory() && f->getSize() > 0) {
            ++files;
          }
          continue;
        }
        for (std::list<File*>::const_iterator it2 = it->second->begin(); it2 != it->second->end(); ++it2) {
          if ((*it2)->isDirectory() || !(*it2)->getSize()) {
            continue;
          }
          SkipListMatch filematch = global->getSkipList()->check((dstpath / it->first / (*it2)->getName()).toString(), false, false);
          if (filematch.action != SKIPLIST_DENY) {
            ++files;
          }
        }
      }
      filestotal = files;
      break;
    }
    case TRANSFERJOB_FXP:
    {
      int files = 0;
      const SkipList& dstskip = dst->getSite()->getSkipList();
      for (std::unordered_map<std::string, std::shared_ptr<FileList>>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
        std::shared_ptr<FileList> dstlist = dstfilelists.at(it->first);
        if (it->first == "") {
          File* f = it->second->getFile(srcfile);
          if (f != NULL && !f->isDirectory() && f->getSize() > 0) {
            ++files;
          }
          continue;
        }
        for (std::list<File*>::const_iterator it2 = it->second->begin(); it2 != it->second->end(); ++it2) {
          if ((*it2)->isDirectory() || !(*it2)->getSize()) {
            continue;
          }
          std::string filename = (*it2)->getName();
          SkipListMatch filematch = dstskip.check((dstlist->getPath() / filename).toString(), false, false, dstsectionskiplist);
          if (filematch.action == SKIPLIST_DENY ||
              (filematch.action == SKIPLIST_UNIQUE && dstlist->containsPattern(filematch.matchpattern, filematch.matchedpath, false)))
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
          ++files;
        }
      }
      filestotal = files;
      break;
    }
    case TRANSFERJOB_UPLOAD:
    {
      int files = 0;
      const SkipList& dstskip = dst->getSite()->getSkipList();
      std::unordered_map<std::string, std::shared_ptr<LocalFileList>>::const_iterator it;
      for (it = localfilelists.begin(); it != localfilelists.end(); it++) {
        std::shared_ptr<FileList> dstlist = dstfilelists.at(it->first);
        if (it->first == "") {
          std::unordered_map<std::string, LocalFile>::const_iterator it2 = it->second->find(srcfile);
          if (it2 != it->second->end() && !it2->second.isDirectory() && it2->second.getSize() > 0) {
            ++files;
          }
          continue;
        }
        for (std::unordered_map<std::string, LocalFile>::const_iterator it2 = it->second->begin(); it2 != it->second->end(); ++it2) {
          if (it2->second.isDirectory() || !it2->second.getSize()) {
            continue;
          }
          std::string filename = it2->first;
          SkipListMatch filematch = dstskip.check((dstlist->getPath() / filename).toString(), false, false, dstsectionskiplist);
          if (filematch.action == SKIPLIST_DENY ||
             (filematch.action == SKIPLIST_UNIQUE && dstlist->containsPattern(filematch.matchpattern, filematch.matchedpath, false)))
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
          ++files;
        }
      }
      filestotal = files;
      break;
    }
  }
}

void TransferJob::abort() {
  if (isDone()) {
    return;
  }
  setDone();
  status = TRANSFERJOB_ABORTED;
  if (!!srcsitetransferjob) {
    src->abortTransfers(srcsitetransferjob);
  }
  if (!!dstsitetransferjob) {
    dst->abortTransfers(srcsitetransferjob);
  }
}

void TransferJob::clearExisting() {
  existingtargets.clear();
  pendingtransfers.clear();
}

unsigned int TransferJob::getId() const {
  return id;
}

void TransferJob::setDone() {
  if (isDone()) {
    return;
  }
  global->getTickPoke()->stopPoke(this, 0);
  status = TRANSFERJOB_DONE;
  timeremaining = 0;
}

void TransferJob::updateLocalFileLists() {
  std::shared_ptr<LocalFileList> base;
  auto it = localfilelists.find("");
  if (it != localfilelists.end()) {
    base = it->second;
    global->getLocalStorage()->updateLocalFileList(base);
  }
  else {
    Path basepath = type == TRANSFERJOB_DOWNLOAD ? dstpath : srcpath;
    if (!FileSystem::directoryExists(basepath)) {
      if (type == TRANSFERJOB_UPLOAD) {
        return;
      }
      util::Result res = FileSystem::createDirectoryRecursive(basepath);
      if (!res.success) {
        global->getEventLog()->log("TransferJob", "Failed to create " + basepath.toString() + ": " + res.error);
        return;
      }
    }
    localfilelists[""] = base = global->getLocalStorage()->getLocalFileList(basepath);
  }
  if (!base) {
    return;
  }
  std::unordered_map<std::string, LocalFile>::const_iterator it2 = base->find(srcfile);
  if (it2 != base->end() && it2->second.isDirectory()) {
    updateLocalFileLists(base->getPath(), base->getPath() / srcfile);
  }
}

void TransferJob::updateLocalFileLists(const Path& basepath, const Path& path) {
  std::string subpath = (path - basepath).toString();
  auto it = localfilelists.find(subpath);
  std::shared_ptr<LocalFileList> filelist;
  if (it != localfilelists.end()) {
    filelist = it->second;
    global->getLocalStorage()->updateLocalFileList(filelist);

  }
  else {
    filelist = global->getLocalStorage()->getLocalFileList(path);
  }
  if (!!filelist) {
    localfilelists[subpath] = filelist;
    if (type == TRANSFERJOB_DOWNLOAD && srcfilelists.find(subpath) == srcfilelists.end()) {
      std::shared_ptr<FileList> fl = std::make_shared<FileList>(srcfilelists[""]->getUser(), srcpath / subpath);
      srcfilelists[subpath] = fl;
      filelistsrefreshed[fl] = REFRESH_NOW;
    }
    else if (type == TRANSFERJOB_UPLOAD && dstfilelists.find(subpath) == dstfilelists.end()) {
      std::shared_ptr<FileList> fl = std::make_shared<FileList>(dstfilelists[""]->getUser(), dstpath / subpath);
      dstfilelists[subpath] = fl;
      filelistsrefreshed[fl] = REFRESH_NOW;
    }
    std::unordered_map<std::string, LocalFile>::const_iterator it;
    for (it = filelist->begin(); it != filelist->end(); it++) {
      if (it->second.isDirectory()) {
        updateLocalFileLists(basepath, path / it->first);
      }
    }
  }
}

bool TransferJob::hasFailedTransfer(const std::string& dstpath) const {
  std::unordered_map<std::string, int>::const_iterator it = transferattempts.find(dstpath);
  return it != transferattempts.end() && it->second >= MAX_TRANSFER_ATTEMPTS_BEFORE_SKIP;
}

void TransferJob::transferSuccessful(const std::shared_ptr<TransferStatus>& ts) {
  idletime = 0;
  addTransferAttempt(ts, true);
}

void TransferJob::transferFailed(const std::shared_ptr<TransferStatus>& ts, int) {
  if (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) {
    if (!checkFileListExists(ts->getSourceFileList())) {
      return;
    }
  }
  if (type == TRANSFERJOB_UPLOAD || type == TRANSFERJOB_FXP) {
    if (!checkFileListExists(ts->getTargetFileList())) {
      return;
    }
  }
  idletime = 0;
  if (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) {
    filelistsrefreshed[ts->getSourceFileList()] = REFRESH_NOW;
    ts->getSourceFileList()->resetUpdateState();
  }
  if (type == TRANSFERJOB_UPLOAD || type == TRANSFERJOB_FXP) {
    if (!dst->getSite()->useXDUPE()) {
      filelistsrefreshed[ts->getTargetFileList()] = REFRESH_NOW;
      ts->getTargetFileList()->resetUpdateState();
    }
  }
  addTransferAttempt(ts, false);
}

void TransferJob::addTransferAttempt(const std::shared_ptr<TransferStatus>& ts, bool success) {
  std::string dstpath = (Path(ts->getTargetPath()) / ts->getFile()).toString();
  std::unordered_map<std::string, int>::iterator it = transferattempts.find(dstpath);
  if (it == transferattempts.end()) {
    transferattempts[dstpath] = 1;
  }
  else {
    it->second++;
  }
  if (success && this->dstpath == ts->getTargetPath() && this->dstfile == ts->getFile()) {
    transferattempts[dstpath] = MAX_TRANSFER_ATTEMPTS_BEFORE_SKIP;
  }
}

std::shared_ptr<SiteTransferJob>& TransferJob::getSrcTransferJob() {
  return srcsitetransferjob;
}

std::shared_ptr<SiteTransferJob>& TransferJob::getDstTransferJob() {
  return dstsitetransferjob;
}

SkipList* TransferJob::getDstSectionSkipList() const {
  return dstsectionskiplist;
}

void TransferJob::reset() {
  global->getTickPoke()->stopPoke(this, 0);
  srcfilelists.clear();
  dstfilelists.clear();
  localfilelists.clear();
  pendingtransfers.clear();
  existingtargets.clear();
  srclisttarget.reset();
  dstlisttarget.reset();
  filelistsrefreshed.clear();
  transferattempts.clear();
  resetValues();
  if (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) {
    std::shared_ptr<FileList> srcfilelist = std::make_shared<FileList>(src->getSite()->getUser(), srcpath);
    srcfilelists[""] = srcfilelist;
    filelistsrefreshed[srcfilelist] = REFRESH_NOW;
  }
  if (type == TRANSFERJOB_UPLOAD || type == TRANSFERJOB_FXP) {
    std::shared_ptr<FileList> dstfilelist = std::make_shared<FileList>(dst->getSite()->getUser(), dstpath);
    dstfilelists[""] = dstfilelist;
    filelistsrefreshed[dstfilelist] = REFRESH_NOW;
  }
  if (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_UPLOAD) {
    updateLocalFileLists();
  }
}

void TransferJob::resetValues() {
  status = TRANSFERJOB_QUEUED;
  timequeued = util::ctimeLog();
  almostdone = false;
  slots = !!src ? src->getSite()->getMaxDownTransferJob() : 1;
  expectedfinalsize = 0;
  sizeprogress = 0;
  timeremaining = -1;
  timespentmillis = 0;
  timespentsecs = 0;
  progress = 0;
  milliprogress = 0;
  speed = 0;
  filesprogress = 0;
  filestotal = 0;
  idletime = 0;
  timestarted = "-";
}
