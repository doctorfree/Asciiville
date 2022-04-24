#include "transferstatus.h"

#include <cassert>

#include "filelist.h"
#include "util.h"

TransferStatus::TransferStatus(int type, const std::string& source, const std::string& target,
    const std::string& jobname, const std::string& file, const std::shared_ptr<FileList>& fls,
    const Path& sourcepath, const std::shared_ptr<FileList>& fld, const Path& targetpath,
    unsigned long long int sourcesize, unsigned int assumedspeed, int srcslot,
    int dstslot, bool ssl, bool defaultactive) :
    type(type), source(source), target(target), jobname(jobname), file(file),
    timestamp(util::ctimeLog()), sourcepath(sourcepath),
    targetpath(targetpath), sourcesize(sourcesize), knowntargetsize(0),
    interpolatedtargetsize(0), interpolationfilltargetsize(0), speed(assumedspeed),
    state(TRANSFERSTATUS_STATE_IN_PROGRESS), timespent(0), progress(0),
    awaited(false), callback(NULL), fls(fls), fld(fld), srcslot(srcslot), dstslot(dstslot),
    ssl(ssl), defaultactive(defaultactive), passiveaddr("-"), cipher("-"), sslsessionreused(false)
{
  if (!this->speed) {
    this->speed = 1024;
  }
  if (!this->sourcesize) {
    this->sourcesize = 1;
  }
  this->timeremaining = this->sourcesize / (this->speed * 1024);
}

TransferStatus::~TransferStatus() {
}

std::string TransferStatus::getSource() const {
  return source;
}

std::string TransferStatus::getTarget() const {
  return target;
}

std::string TransferStatus::getJobName() const {
  return jobname;
}

std::string TransferStatus::getFile() const {
  return file;
}

const Path & TransferStatus::getSourcePath() const {
  return sourcepath;
}

const Path & TransferStatus::getTargetPath() const {
  return targetpath;
}

std::shared_ptr<FileList> TransferStatus::getSourceFileList() const {
  return fls;
}

std::shared_ptr<FileList> TransferStatus::getTargetFileList() const {
  return fld;
}

unsigned long long int TransferStatus::sourceSize() const {
  return sourcesize;
}

unsigned long long int TransferStatus::targetSize() const {
  return interpolatedtargetsize;
}

unsigned long long int TransferStatus::knownTargetSize() const {
  return knowntargetsize;
}

unsigned int TransferStatus::getSpeed() const {
  return speed;
}

unsigned int TransferStatus::getTimeSpent() const {
  return timespent;
}

int TransferStatus::getTimeRemaining() const {
  return timeremaining;
}

unsigned int TransferStatus::getProgress() const {
  return progress;
}

std::string TransferStatus::getTimestamp() const {
  return timestamp;
}

TransferStatusCallback * TransferStatus::getCallback() const {
  return callback;
}

int TransferStatus::getState() const {
  return state;
}

int TransferStatus::getType() const {
  return type;
}

bool TransferStatus::isAwaited() const {
  return awaited;
}

int TransferStatus::getSourceSlot() const {
  return srcslot;
}

int TransferStatus::getTargetSlot() const {
  return dstslot;
}

bool TransferStatus::getSSL() const {
  return ssl;
}

bool TransferStatus::getDefaultActive() const {
  return defaultactive;
}

std::string TransferStatus::getPassiveAddress() const {
  return passiveaddr;
}

std::string TransferStatus::getCipher() const {
  return cipher;
}

bool TransferStatus::getSSLSessionReused() const {
  return sslsessionreused;
}

const std::list<std::string> & TransferStatus::getLogLines() const {
  return loglines;
}

void TransferStatus::setFinished() {
  state = TRANSFERSTATUS_STATE_SUCCESSFUL;
  progress = 100;
  interpolatedtargetsize = knowntargetsize;
  timeremaining = 0;
}

void TransferStatus::setFailed() {
  if (state != TRANSFERSTATUS_STATE_ABORTED) {
    state = TRANSFERSTATUS_STATE_FAILED;
    knowntargetsize = 0;
    interpolatedtargetsize = 0;
    progress = 0;
    speed = 0;
  }
  timeremaining = 0;
}

void TransferStatus::setTimeout() {
  if (state != TRANSFERSTATUS_STATE_ABORTED) {
    state = TRANSFERSTATUS_STATE_TIMEOUT;
  }
  timeremaining = 0;
}

void TransferStatus::setDupe() {
  if (state != TRANSFERSTATUS_STATE_ABORTED) {
    state = TRANSFERSTATUS_STATE_DUPE;
    knowntargetsize = 0;
    interpolatedtargetsize = 0;
    progress = 0;
    speed = 0;
  }
  timeremaining = 0;
}

void TransferStatus::setAborted() {
  state = TRANSFERSTATUS_STATE_ABORTED;
}

void TransferStatus::setAwaited(bool awaited) {
  this->awaited = awaited;
}

void TransferStatus::setCallback(TransferStatusCallback * callback) {
  this->callback = callback;
}

void TransferStatus::setTargetSize(unsigned long long int targetsize) {
  // the appearing size (interpolatedtargetsize) cannot shrink. knownsize is
  // only used for determining when speed recalculation is necessary
  knowntargetsize = targetsize;
  if (knowntargetsize > sourcesize) {
    sourcesize = knowntargetsize;
  }
  if (interpolatedtargetsize < knowntargetsize) {
    interpolatedtargetsize = knowntargetsize;
    updateProgress();
  }
  interpolationfilltargetsize = knowntargetsize;
}

void TransferStatus::interpolateAddSize(unsigned long long int interpolateaddsize) {
  if (interpolationfilltargetsize < interpolatedtargetsize) {
    interpolationfilltargetsize += interpolateaddsize;
    if (interpolationfilltargetsize > interpolatedtargetsize) {
      interpolatedtargetsize = interpolationfilltargetsize;
      updateProgress();
    }
  }
  else if (interpolatedtargetsize + interpolateaddsize <= sourcesize) {
    interpolatedtargetsize += interpolateaddsize;
    interpolationfilltargetsize = interpolatedtargetsize;
    updateProgress();
  }
}

void TransferStatus::updateProgress() {
  unsigned long long int temptargetsize = interpolatedtargetsize;
  progress = (100 * temptargetsize) / sourcesize;
  if (progress > 100) {
    progress = 100;
  }
}
void TransferStatus::setSpeed(unsigned int speed) {
  this->speed = speed;
  if (!this->speed) {
    this->speed = 10;
  }
}

void TransferStatus::setTimeSpent(unsigned int timespent) {
  this->timespent = timespent;
  if (sourcesize > interpolatedtargetsize) {
    timeremaining = (sourcesize - interpolatedtargetsize) / (speed * 1024);
  }
  else {
    timeremaining = -1;
  }
}

void TransferStatus::setPassiveAddress(const std::string & passiveaddr) {
  this->passiveaddr = passiveaddr;
}

void TransferStatus::setCipher(const std::string & cipher) {
  this->cipher = cipher;
}

void TransferStatus::setSSLSessionReused(bool reused) {
  this->sslsessionreused = reused;
}

void TransferStatus::setSourceSize(unsigned long long int size) {
  sourcesize = size;
}

void TransferStatus::addLogLine(const std::string & line) {
  assert(state == TRANSFERSTATUS_STATE_IN_PROGRESS ||
               state == TRANSFERSTATUS_STATE_ABORTED);
  loglines.push_back(line);
}
