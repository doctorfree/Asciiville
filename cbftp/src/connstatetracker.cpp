#include "connstatetracker.h"

#include <cassert>

#include "delayedcommand.h"
#include "eventlog.h"
#include "filelist.h"
#include "recursivecommandlogic.h"
#include "sitelogicrequest.h"
#include "siterace.h"

ConnStateTracker::ConnStateTracker() :
  time(0),
  idletime(0),
  lastcheckedcount(0),
  lastchecked(NULL),
  transfer(false),
  initialized(false),
  tm(NULL),
  aborted(false),
  transferlocked(false),
  loggedin(false),
  fxp(false),
  listtransfer(false),
  listinitialized(false),
  quitting(false),
  handling(false),
  recursivelogic(std::make_shared<RecursiveCommandLogic>()),
  refreshtoken(false)
{
}

ConnStateTracker::~ConnStateTracker() {

}

void ConnStateTracker::delayedCommand(const std::string & command, int delay, bool persisting, const std::shared_ptr<CommandOwner> & co) {
  delayedcommand.set(command, time + delay, co, persisting);
}

void ConnStateTracker::timePassed(int time) {
  this->time += time;
  this->idletime += time;
  if (delayedcommand.isActive()) {
    delayedcommand.currentTime(this->time);
  }
}

int ConnStateTracker::getTimePassed() const {
  return idletime;
}

void ConnStateTracker::check() {
  lastcheckedcount++;
}

void ConnStateTracker::setLastChecked(const std::shared_ptr<SiteRace>& sr) {
  if (lastchecked != sr) {
    lastchecked = sr;
    lastcheckedcount = 0;
  }
}

void ConnStateTracker::resetLastChecked() {
  lastchecked = nullptr;
  lastcheckedcount = 0;
}

const std::shared_ptr<SiteRace> & ConnStateTracker::lastChecked() const {
  return lastchecked;
}

int ConnStateTracker::checkCount() const {
    return lastcheckedcount;
}

void ConnStateTracker::purgeSiteRace(const std::shared_ptr<SiteRace>& sr) {
  if (lastchecked == sr) {
    lastchecked = NULL;
    lastcheckedcount = 0;
  }
  if (delayedcommand.isActive() && delayedcommand.getCommandOwner() == sr) {
    delayedcommand.set("handle", 0);
  }
}

DelayedCommand & ConnStateTracker::getCommand() {
  return delayedcommand;
}

void ConnStateTracker::setDisconnected() {
  assert(!transferlocked);
  assert(!listtransfer);
  assert(!transfer);
  assert(!request);
  loggedin = false;
  quitting = false;
  delayedcommand.weakReset();
  idletime = 0;
}

void ConnStateTracker::use() {
  assert(!transferlocked);
  delayedcommand.reset();
  idletime = 0;
}

void ConnStateTracker::resetIdleTime() {
  delayedcommand.reset();
  idletime = 0;
}

void ConnStateTracker::setTransfer(const std::string & file, bool fxp, bool ipv6, bool passive, const std::string & host, int port, bool ssl, bool sslclient) {
  assert(transferlocked);
  assert(!transfer);
  assert(!fxp || !request);
  assert(tm);
  this->transfer = true;
  this->initialized = false;
  this->file = file;
  this->fxp = fxp;
  this->passive = passive;
  this->host = host;
  this->port = port;
  this->ssl = ssl;
  this->sslclient = sslclient;
  this->ipv6 = ipv6;
}

void ConnStateTracker::setTransfer(const std::string& file, bool fxp, bool ipv6, bool ssl, bool sslclient) {
  setTransfer(file, fxp, ipv6, true, "", 0, ssl, sslclient);
}

void ConnStateTracker::setTransfer(const std::string& file, bool fxp, bool ipv6, const std::string& host, int port, bool ssl, bool sslclient) {
  setTransfer(file, fxp, ipv6, false, host, port, ssl, sslclient);
}

void ConnStateTracker::setList(TransferMonitor* tm, bool ipv6, bool listpassive, const std::string& host, int port, bool ssl) {
  assert(!transferlocked);
  assert(!listtransfer);
  assert(!transfer);
  use();
  this->listtransfer = true;
  this->listinitialized = false;
  this->listtm = tm;
  this->listpassive = listpassive;
  this->listhost = host;
  this->listport = port;
  this->listssl = ssl;
  this->listipv6 = ipv6;
}

void ConnStateTracker::setList(TransferMonitor* tm, bool ipv6, bool ssl) {
  setList(tm, ipv6, true, "", 0, ssl);
}

void ConnStateTracker::setList(TransferMonitor* tm, bool ipv6, const std::string & host, int port, bool ssl) {
  setList(tm, ipv6, false, host, port, ssl);
}

bool ConnStateTracker::isLoggedIn() const {
  return loggedin;
}

void ConnStateTracker::setLoggedIn() {
  loggedin = true;
}

bool ConnStateTracker::hasTransfer() const {
  return isListLocked() || hasFileTransfer();
}

bool ConnStateTracker::hasFileTransfer() const {
  return transfer;
}

void ConnStateTracker::finishTransfer() {
  if (listtransfer) {
    listtransfer = false;
    listtm = NULL;
    return;
  }
  finishFileTransfer();
}

void ConnStateTracker::finishFileTransfer() {
  transfer = false;
  transferlocked = false;
  tm = NULL;
  co.reset();
}

void ConnStateTracker::abortTransfer() {
  aborted = true;
}

bool ConnStateTracker::getTransferAborted() const {
  return aborted;
}

TransferMonitor * ConnStateTracker::getTransferMonitor() const {
  if (listtransfer) {
    return listtm;
  }
  if (transferlocked) {
    return tm;
  }
  return NULL;
}

std::shared_ptr<FileList> ConnStateTracker::getTransferFileList() const {
  return fl;
}

std::string ConnStateTracker::getTransferFile() const {
  return file;
}

int ConnStateTracker::getTransferType() const {
  if (listtransfer) {
    return CST_LIST;
  }
  return type;
}

bool ConnStateTracker::getTransferPassive() const {
  if (listtransfer) {
    return listpassive;
  }
  return passive;
}

bool ConnStateTracker::getTransferFXP() const {
  if (listtransfer) {
    return false;
  }
  return fxp;
}

std::string ConnStateTracker::getTransferHost() const {
  if (listtransfer) {
    return listhost;
  }
  return host;
}

int ConnStateTracker::getTransferPort() const {
  if (listtransfer) {
    return listport;
  }
  return port;
}

bool ConnStateTracker::getTransferSSL() const {
  if (listtransfer) {
    return listssl;
  }
  return ssl;
}

bool ConnStateTracker::getTransferIPv6() const {
  if (listtransfer) {
    return listipv6;
  }
  return ipv6;
}

bool ConnStateTracker::getTransferSSLClient() const {
  if (listtransfer) {
    return false;
  }
  return sslclient;
}

const std::shared_ptr<CommandOwner> & ConnStateTracker::getCommandOwner() const {
  return co;
}

void ConnStateTracker::lockForTransfer(TransferMonitor * tm, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner> & co, bool download) {
  assert(!transferlocked);
  assert(!transfer);
  assert(download || !request);
  use();
  this->tm = tm;
  this->fl = fl;
  this->co = co;
  aborted = false;
  transferlocked = true;
  type = download ? CST_DOWNLOAD : CST_UPLOAD;
}

bool ConnStateTracker::isListLocked() const {
  return listtransfer;
}

bool ConnStateTracker::isTransferLocked() const {
  return transferlocked;
}

bool ConnStateTracker::hasRequest() const {
  return !!request;
}

bool ConnStateTracker::isLocked() const {
  return isListOrTransferLocked() || hasRequest() || isQuitting();
}

bool ConnStateTracker::isListOrTransferLocked() const {
  return isListLocked() || isTransferLocked();
}

bool ConnStateTracker::isHardLocked() const {
  return isTransferLocked() || hasRequest() || isQuitting();
}

const std::shared_ptr<SiteLogicRequest> & ConnStateTracker::getRequest() const {
  return request;
}

void ConnStateTracker::setRequest(SiteLogicRequest request) {
  assert(!this->request);
  this->request = std::make_shared<SiteLogicRequest>(request);
}

void ConnStateTracker::finishRequest() {
  request.reset();
}

const std::shared_ptr<RecursiveCommandLogic> & ConnStateTracker::getRecursiveLogic() const {
  return recursivelogic;
}

bool ConnStateTracker::transferInitialized() const {
  if (listtransfer) {
    return listinitialized;
  }
  return transfer && initialized;
}

void ConnStateTracker::initializeTransfer() {
  if (listtransfer) {
    listinitialized = true;
  }
  else {
    initialized = true;
  }
}

bool ConnStateTracker::isQuitting() const {
  return quitting;
}

void ConnStateTracker::setQuitting() {
  quitting = true;
}

bool ConnStateTracker::hasRefreshToken() const {
  return refreshtoken;
}

void ConnStateTracker::setRefreshToken() {
  refreshtoken = true;
}

void ConnStateTracker::useRefreshTokenFor(const std::string& refreshpath) {
  refreshtoken = false;
  lastrefreshpath = refreshpath;
}

std::string ConnStateTracker::getLastRefreshPath() const {
  return lastrefreshpath;
}

bool ConnStateTracker::isHandlingConnection() const {
  return handling;
}

void ConnStateTracker::setHandlingConnection(bool handling) {
  this->handling = handling;
}
