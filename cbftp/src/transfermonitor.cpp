#include "transfermonitor.h"

#include <algorithm>
#include <cassert>
#include <unordered_map>

#include "core/tickpoke.h"
#include "site.h"
#include "sitelogic.h"
#include "siterace.h"
#include "ftpconn.h"
#include "filelist.h"
#include "globalcontext.h"
#include "file.h"
#include "localstorage.h"
#include "localfilelist.h"
#include "localfile.h"
#include "transfermanager.h"
#include "transferstatus.h"
#include "localtransfer.h"
#include "localdownload.h"
#include "filesystem.h"
#include "statistics.h"
#include "transferprotocol.h"

// max time to wait for the other site to fail after a failure has happened
// before a hard disconnect occurs, in ms
#define MAX_WAIT_ERROR 5000

// max time to wait for the destination to complete the transfer after the
// source has reported the transfer to be complete, in ms
#define MAX_WAIT_SOURCE_COMPLETE 60000

// ticker heartbeat, in ms
#define TICK_INTERVAL 50

TransferMonitor::TransferMonitor(TransferManager * tm) :
  status(TM_STATUS_IDLE),
  clientactive(true),
  fxpdstactive(true),
  timestamp(0),
  tm(tm),
  ticker(0)
{
  global->getTickPoke()->startPoke(this, "TransferMonitor", TICK_INTERVAL, 0);
}

TransferMonitor::~TransferMonitor() {

}

bool TransferMonitor::idle() const {
  return status == TM_STATUS_IDLE;
}

void TransferMonitor::engageFXP(
  const std::string& sfile, const std::shared_ptr<SiteLogic>& sls, const std::shared_ptr<FileList>& fls,
  const std::string& dfile, const std::shared_ptr<SiteLogic>& sld, const std::shared_ptr<FileList>& fld,
  const std::shared_ptr<CommandOwner>& srcco, const std::shared_ptr<CommandOwner>& dstco)
{
  reset();
  type = TM_TYPE_FXP;
  this->sls = sls;
  this->sld = sld;
  this->fls = fls;
  this->spath = fls->getPath();
  this->fld = fld;
  this->dpath = fld->getPath();
  this->sfile = sfile;
  this->dfile = dfile;
  this->srcco = srcco;
  this->dstco = dstco;
  setStatus(TM_STATUS_AWAITING_PASSIVE);
  if (!sls->lockDownloadConn(fls, &src, srcco, this)) {
    tm->transferFailed(ts, TM_ERR_LOCK_DOWN);
    setStatus(TM_STATUS_IDLE);
    return;
  }
  if (!sld->lockUploadConn(fld, &dst, dstco, this)) {
    tm->transferFailed(ts, TM_ERR_LOCK_UP);
    sls->returnConn(src, true);
    setStatus(TM_STATUS_IDLE);
    return;
  }
  fxpdstactive = !sls->getSite()->hasBrokenPASV();
  int spol = sls->getSite()->getSSLTransferPolicy();
  int dpol = sld->getSite()->getSSLTransferPolicy();
  if (spol != SITE_SSL_ALWAYS_OFF && dpol != SITE_SSL_ALWAYS_OFF &&
      (spol == SITE_SSL_ALWAYS_ON || dpol == SITE_SSL_ALWAYS_ON ||
      (spol == SITE_SSL_PREFER_ON && dpol == SITE_SSL_PREFER_ON)))
  {
    ssl = true;
    sourcesslclient = fxpdstactive;
    if ((fxpdstactive && !sls->getSite()->supportsSSCN() && !sls->getSite()->supportsCPSV()) ||
        (!fxpdstactive && !sld->getSite()->supportsSSCN() && !sld->getSite()->supportsCPSV()))
    {
      sourcesslclient = !sourcesslclient;
    }
  }
  TransferProtocol sprot = sls->getSite()->getTransferProtocol();
  TransferProtocol dprot = sld->getSite()->getTransferProtocol();
  ipv6 = useIPv6(sprot, dprot);
  fld->touchFile(dfile, sld->getSite()->getUser(), true);
  latestsrctouch = fls->getFile(sfile)->getTouch();
  latestdsttouch = fld->getFile(dfile)->getTouch();
  fls->download(sfile);

  ts = std::make_shared<TransferStatus>(TRANSFERSTATUS_TYPE_FXP, sls->getSite()->getName(),
      sld->getSite()->getName(), srcco ? srcco->getName() : "", dfile, fls, fls->getPath(), fld, fld->getPath(),
      fls->getFile(sfile)->getSize(),
      sls->getSite()->getAverageSpeed(sld->getSite()->getName()), src, dst, ssl, fxpdstactive);
  tm->addNewTransferStatus(ts);
  if ((spol == SITE_SSL_ALWAYS_OFF && dpol == SITE_SSL_ALWAYS_ON) ||
      (spol == SITE_SSL_ALWAYS_ON && dpol == SITE_SSL_ALWAYS_OFF))
  {
    lateFXPFailure("FXP transfer not possible due to conflicting TLS settings");
    return;
  }
  if ((spol == SITE_SSL_ALWAYS_ON || dpol == SITE_SSL_ALWAYS_ON) &&
      !sls->getSite()->supportsSSCN() && !sld->getSite()->supportsSSCN())
  {
    lateFXPFailure("TLS FXP transfer not possible due to neither site supporting SSCN");
    return;
  }
  if (!transferProtocolCombinationPossible(sprot, dprot)) {
    lateFXPFailure("FXP transfer not possible due to conflicting IPv4/IPv6 settings");
    return;
  }
  if (fxpdstactive) {
    sls->preparePassiveTransfer(src, sfile, true, ipv6, ssl, sourcesslclient);
  }
  else {
    if (sld->getSite()->hasBrokenPASV()) {
      lateFXPFailure("FXP transfer not possible due to both sites having broken PASV");
      return;
    }
    sld->preparePassiveTransfer(dst, dfile, true, ipv6, ssl, !sourcesslclient);
  }
  int maxsls = sls->getSite()->getMaxTransferTimeSeconds();
  int maxsld = sld->getSite()->getMaxTransferTimeSeconds();
  int maxtime = tm->getMaxTransferTimeSeconds();
  if (maxsls >= 0) {
    maxtime = maxsls;
  }
  if (maxsld > 0 && (maxsls == 0 || maxsld < maxsls)) {
    maxtime = maxsld;
  }
  maxtransfertimeseconds = maxtime;
}

void TransferMonitor::engageDownload(
  const std::string& sfile, const std::shared_ptr<SiteLogic>& sls, const std::shared_ptr<FileList>& fls,
  const std::shared_ptr<LocalFileList>& localfl, const std::shared_ptr<CommandOwner>& co, int connid)
{
  reset();
  type = TM_TYPE_DOWNLOAD;
  this->sls = sls;
  this->sfile = sfile;
  this->dfile = sfile;
  this->spath = fls->getPath();
  if (localfl) {
    this->dpath = localfl->getPath();
  }
  this->fls = fls;
  this->localfl = localfl;
  this->srcco = co;
  this->src = connid;
  if (src == -1) {
    if (!sls->lockDownloadConn(fls, &src, srcco, this)) {
      return;
    }
  }
  else {
    sls->registerDownloadLock(src, fls, srcco, this);
  }
  setStatus(TM_STATUS_AWAITING_PASSIVE);
  int spol = sls->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  TransferProtocol sprot = sls->getSite()->getTransferProtocol();
  TransferProtocol dprot = global->getLocalStorage()->getTransferProtocol();
  ipv6 = useIPv6(sprot, dprot);
  clientactive = !sls->getSite()->hasBrokenPASV();
  ts = std::make_shared<TransferStatus>(TRANSFERSTATUS_TYPE_DOWNLOAD,
      sls->getSite()->getName(), "/\\", co ? co->getName() : "", dfile, fls, spath,
      nullptr, dpath, fls->getFile(sfile)->getSize(), 0, src, -1, ssl, clientactive);
  tm->addNewTransferStatus(ts);
  if (localfl) {
    localfl->touchFile(dfile, true);
    if (!FileSystem::directoryExists(dpath)) {
      util::Result res = FileSystem::createDirectoryRecursive(dpath);
      if (!res.success) {
        lateDownloadFailure("Failed to create directory " + dpath.toString() + ": " + res.error);
        return;
      }
    }
    if (FileSystem::fileExists(dpath / dfile) && global->getLocalStorage()->getLocalFile(dpath / dfile).getSize() > 0) {
      lateDownloadFailure((dpath / dfile).toString() + " already exists", true);
      return;
    }
  }
  if (!transferProtocolCombinationPossible(sprot, dprot)) {
    lateDownloadFailure("Transfer not possible due to conflicting IPv4/IPv6 settings");
    return;
  }
  if (clientactive) {
    sls->preparePassiveTransfer(src, sfile, false, ipv6, ssl);
  }
  else {
    FTPConn* conn = sls->getConn(src);
    if (!conn->getDataProxy()) {
      Core::StringResult passiveresult = ipv6 ? global->getLocalStorage()->getAddress6(conn->getSockId()) : global->getLocalStorage()->getAddress4(conn->getSockId());
      if (!passiveresult.success) {
        lateDownloadFailure(passiveresult.error);
        return;
      }
    }
    if (localfl) {
      lt = global->getLocalStorage()->activeModeDownload(this, dpath, dfile, ipv6, ssl, conn);
    }
    else {
      lt = global->getLocalStorage()->activeModeDownload(this, ipv6, ssl, conn);
      storeid = static_cast<LocalDownload *>(lt)->getStoreId();
    }
  }
  int maxsls = sls->getSite()->getMaxTransferTimeSeconds();
  int maxtime = tm->getMaxTransferTimeSeconds();
  if (maxsls >= 0) {
    maxtime = maxsls;
  }
  maxtransfertimeseconds = maxtime;
}

void TransferMonitor::engageUpload(
  const std::string& sfile, const std::shared_ptr<LocalFileList>& localfl,
  const std::shared_ptr<SiteLogic>& sld, const std::shared_ptr<FileList>& fld, const std::shared_ptr<CommandOwner>& co)
{
  reset();
  if (!localfl) return;
  type = TM_TYPE_UPLOAD;
  this->sld = sld;
  this->sfile = sfile;
  this->dfile = sfile;
  this->spath = localfl->getPath();
  this->dpath = fld->getPath();
  this->fld = fld;
  this->localfl = localfl;
  this->dstco = co;
  std::unordered_map<std::string, LocalFile>::const_iterator it = localfl->find(sfile);
  if (it == localfl->end()) return;
  const LocalFile & lf = it->second;
  if (!sld->lockUploadConn(fld, &dst, dstco, this)) {
    return;
  }
  setStatus(TM_STATUS_AWAITING_PASSIVE);
  int spol = sld->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  TransferProtocol sprot = global->getLocalStorage()->getTransferProtocol();
  TransferProtocol dprot = sld->getSite()->getTransferProtocol();
  ipv6 = useIPv6(sprot, dprot);
  fld->touchFile(dfile, sld->getSite()->getUser(), true);
  clientactive = !sld->getSite()->hasBrokenPASV();
  ts = std::make_shared<TransferStatus>(TRANSFERSTATUS_TYPE_UPLOAD,
      "/\\", sld->getSite()->getName(), co ? co->getName() : "", dfile, nullptr, spath,
      fld, dpath, lf.getSize(), 0, 0, dst, ssl, clientactive);
  tm->addNewTransferStatus(ts);
  if (!FileSystem::fileExists(spath / sfile)) {
    lateUploadFailure((spath / sfile).toString() + " already exists", true);
    return;
  }
  if (!transferProtocolCombinationPossible(sprot, dprot)) {
    lateUploadFailure("Transfer not possible due to conflicting IPv4/IPv6 settings");
    return;
  }
  if (clientactive) {
    sld->preparePassiveTransfer(dst, dfile, false, ipv6, ssl);
  }
  else {
    FTPConn* conn = sld->getConn(dst);
    if (!conn->getDataProxy()) {
      Core::StringResult passiveresult = ipv6 ? global->getLocalStorage()->getAddress6(conn->getSockId()) : global->getLocalStorage()->getAddress4(conn->getSockId());
      if (!passiveresult.success) {
        localError(passiveresult.error);
        ts->setFailed();
        tm->transferFailed(ts, TM_ERR_OTHER);
        sld->returnConn(dst, true);
        setStatus(TM_STATUS_IDLE);
        return;
      }
    }
    lt = global->getLocalStorage()->activeModeUpload(this, spath, sfile, ipv6, ssl, conn);
  }
  int maxsld = sld->getSite()->getMaxTransferTimeSeconds();
  int maxtime = tm->getMaxTransferTimeSeconds();
  if (maxsld >= 0) {
    maxtime = maxsld;
  }
  maxtransfertimeseconds = maxtime;
}

void TransferMonitor::engageList(const std::shared_ptr<SiteLogic>& sls, int connid,
  bool hiddenfiles, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co, bool ipv6)
{
  reset();
  type = TM_TYPE_LIST;
  this->sls = sls;
  this->fls = fl;
  this->srcco = co;
  src = connid;
  this->hiddenfiles = hiddenfiles;
  ssl = sls->getSite()->getTLSMode() != TLSMode::NONE;
  this->ipv6 = ipv6;
  setStatus(TM_STATUS_AWAITING_PASSIVE);
  if (!sls->getSite()->hasBrokenPASV()) {
    sls->preparePassiveList(src, this, ipv6, ssl);
  }
  else {
    clientactive = false;
    FTPConn* conn = sls->getConn(src);
    if (!conn->getDataProxy()) {
      Core::StringResult passiveresult = ipv6 ? global->getLocalStorage()->getAddress6(conn->getSockId()) : global->getLocalStorage()->getAddress4(conn->getSockId());
      if (!passiveresult.success) { // should be impossible since the protocol is the same as the control connection
        sls->getConn(src)->printLocalError(passiveresult.error);
        setStatus(TM_STATUS_IDLE);
        return;
      }
    }
    lt = global->getLocalStorage()->activeModeDownload(this, ipv6, ssl, conn);
    storeid = static_cast<LocalDownload *>(lt)->getStoreId();
  }
  maxtransfertimeseconds = 30;
}

void TransferMonitor::tick(int msg) {
  if (status != TM_STATUS_IDLE) {
    timestamp += TICK_INTERVAL;
    ++ticker;
    if (type == TM_TYPE_FXP) {
      updateFXPSizeSpeed();
      if (ticker % 20 == 0) { // run once per second
        if (checkForDeadFXPTransfers()) {
          return;
        }
      }
    }
    if (type == TM_TYPE_DOWNLOAD || type == TM_TYPE_UPLOAD) {
      if (ticker % 4 == 0) { // run every 200 ms
        updateLocalTransferSizeSpeed();
      }
    }
    if (ticker % 20 == 0) {
      if (checkMaxTransferTime()) {
        return;
      }
    }
  }
}

void TransferMonitor::passiveReady(const std::string & host, int port) {
  assert(status == TM_STATUS_AWAITING_PASSIVE ||
         status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
         status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET);
  if (status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE) {
    sourceError(TM_ERR_OTHER);
    sls->returnConn(src, type != TM_TYPE_LIST);
    return;
  }
  if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET) {
    targetError(TM_ERR_OTHER);
    sld->returnConn(dst, type != TM_TYPE_LIST);
    return;
  }
  setStatus(TM_STATUS_AWAITING_ACTIVE);
  if (!!ts) {
    ts->setPassiveAddress((ipv6 ? "[" + host + "]" : host) + ":" + std::to_string(port));
  }
  switch (type) {
    case TM_TYPE_FXP:
      if (fxpdstactive) {
        sld->prepareActiveTransfer(dst, dfile, true, ipv6, host, port, ssl, !sourcesslclient);
      }
      else {
        sls->prepareActiveTransfer(src, sfile, true, ipv6, host, port, ssl, sourcesslclient);
      }
      break;
    case TM_TYPE_DOWNLOAD:
      if (clientactive) {
        if (localfl) {
          lt = global->getLocalStorage()->passiveModeDownload(this, dpath, dfile, ipv6, host, port, ssl, sls->getConn(src));
        }
        else {
          lt = global->getLocalStorage()->passiveModeDownload(this, ipv6, host, port, ssl, sls->getConn(src));
          storeid = static_cast<LocalDownload *>(lt)->getStoreId();
        }
      }
      else {
        sls->prepareActiveTransfer(src, sfile, false, ipv6, host, port, ssl);
      }
      break;
    case TM_TYPE_UPLOAD:
      if (clientactive) {
        lt = global->getLocalStorage()->passiveModeUpload(this, spath, sfile, ipv6, host, port, ssl, sld->getConn(dst));
      }
      else {
        sld->prepareActiveTransfer(dst, dfile, false, ipv6, host, port, ssl);
      }
      break;
    case TM_TYPE_LIST:
      if (clientactive) {
        lt = global->getLocalStorage()->passiveModeDownload(this, ipv6, host, port, ssl, sls->getConn(src));
        storeid = static_cast<LocalDownload *>(lt)->getStoreId();
      }
      else {
        sls->prepareActiveList(src, this, ipv6, host, port, ssl);
      }
      break;
  }
}

void TransferMonitor::activeReady() {
  assert(status == TM_STATUS_AWAITING_ACTIVE ||
         status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
         status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET);
  if (status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE) {
    sourceError(TM_ERR_OTHER);
    sls->returnConn(src, type != TM_TYPE_LIST);
    return;
  }
  if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET) {
    targetError(TM_ERR_OTHER);
    sld->returnConn(dst, type != TM_TYPE_LIST);
    return;
  }
  if (type == TM_TYPE_FXP) {
    if (fxpdstactive) {
      sld->upload(dst);
    }
    else {
      sls->download(src);
    }
  }
  else {
    startClientTransfer();
  }
}

void TransferMonitor::activeStarted() {
  assert(status == TM_STATUS_AWAITING_ACTIVE ||
         status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
         status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
         status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE);
  if (status == TM_STATUS_AWAITING_ACTIVE) {
    setStatus(TM_STATUS_TRANSFERRING);
    if (type == TM_TYPE_FXP) {
      if (fxpdstactive) {
        sls->download(src);
      }
      else {
        sld->upload(dst);
      }
    }
    else if (clientactive) {
      startClientTransfer();
    }
    startstamp = timestamp;
  }
}

void TransferMonitor::sslDetails(const std::string & cipher, bool sessionreused) {
  if (!!ts) {
    ts->setCipher(cipher);
    ts->setSSLSessionReused(sessionreused);
  }
}

void TransferMonitor::startClientTransfer() {
  switch (type) {
    case TM_TYPE_UPLOAD:
      sld->upload(dst);
      break;
    case TM_TYPE_DOWNLOAD:
      sls->download(src);
      break;
    case TM_TYPE_LIST:
      if (hiddenfiles) {
        sls->listAll(src);
      }
      else {
        sls->list(src);
      }
      break;
    case TM_TYPE_FXP:
      assert(false);
      break;
  }
}

void TransferMonitor::sourceComplete() {
  assert(status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
         status == TM_STATUS_TRANSFERRING ||
         status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE);
  partialcompletestamp = timestamp;
  if (fls) {
    fls->finishDownload(sfile);
  }
  if (status == TM_STATUS_TRANSFERRING) {
    setStatus(TM_STATUS_TRANSFERRING_SOURCE_COMPLETE);
  }
  else if (status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE) {
    transferFailed(ts, error);
  }
  else {
    finish();
  }
}

void TransferMonitor::targetComplete() {
  assert(status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
         status == TM_STATUS_TRANSFERRING ||
         status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE ||
         status == TM_STATUS_AWAITING_ACTIVE);
  partialcompletestamp = timestamp;
  if (fld) {
    fld->finishUpload(dfile);
  }
  if (type == TM_TYPE_DOWNLOAD) {
    if (localfl) {
      localfl->finishDownload(dfile);
    }
  }
  if (status == TM_STATUS_TRANSFERRING || status == TM_STATUS_AWAITING_ACTIVE) {
    setStatus(TM_STATUS_TRANSFERRING_TARGET_COMPLETE);
  }
  else if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET) {
    transferFailed(ts, error);
  }
  else {
    finish();
  }
}

void TransferMonitor::finish() {
  int span = timestamp - startstamp;
  if (span == 0) {
    span = 10;
  }
  if (type == TM_TYPE_FXP) {
    updateFXPSizeSpeed();
  }
  if (type == TM_TYPE_DOWNLOAD || type == TM_TYPE_UPLOAD) {
    updateLocalTransferSizeSpeed();
  }
  switch (type) {
    case TM_TYPE_FXP:
    case TM_TYPE_DOWNLOAD: {
      File * srcfile = fls->getFile(sfile);
      if (srcfile) {
        unsigned long long int size = std::max(srcfile->getSize(), ts->knownTargetSize());
        unsigned int speed = size / span;
        ts->setTargetSize(size);
        ts->setSpeed(speed);
        ts->setTimeSpent(span / 1000);
        if (type == TM_TYPE_FXP) {
          fld->setFileUpdateFlag(dfile, size, speed, sls->getSite(), sld->getSite(), srcco, dstco);
        }
        else {
          global->getStatistics()->addTransferStatsFile(STATS_DOWN, lt->size());
          sls->getSite()->addTransferStatsFile(STATS_DOWN, lt->size());
        }
      }
      if (type == TM_TYPE_DOWNLOAD) {
        sls->downloadCompleted(src, storeid, fls, srcco);
      }
      break;
    }
    case TM_TYPE_UPLOAD: {
      unsigned long long int size = lt->size();
      unsigned int speed = size / span;
      ts->setTargetSize(size);
      ts->setSpeed(speed);
      ts->setTimeSpent(span / 1000);
      sld->getSite()->addTransferStatsFile(STATS_UP, size);
      global->getStatistics()->addTransferStatsFile(STATS_UP, size);
      break;
    }
    case TM_TYPE_LIST:
      sls->listCompleted(src, storeid, fls, srcco);
      break;
  }
  if (!!ts) {
    ts->setFinished();
  }
  tm->transferSuccessful(ts);
  setStatus(TM_STATUS_IDLE);
}

void TransferMonitor::sourceError(TransferError err) {
  assert(status == TM_STATUS_AWAITING_ACTIVE ||
         status == TM_STATUS_AWAITING_PASSIVE ||
         status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
         status == TM_STATUS_TRANSFERRING ||
         status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE);
  if (fls) {
    fls->finishDownload(sfile);
  }
  partialcompletestamp = timestamp;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    if (!!sld && !sld->getConn(dst)->isProcessing()) {
      sld->returnConn(dst, type != TM_TYPE_LIST);
      fld->finishUpload(dfile);
      transferFailed(ts, err);
      return;
    }
  }
  if (status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE ||
      status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
      (type == TM_TYPE_DOWNLOAD && lt == nullptr))
  {
    transferFailed(ts, err);
    return;
  }
  error = err;
  setStatus(TM_STATUS_SOURCE_ERROR_AWAITING_TARGET);
}

void TransferMonitor::targetError(TransferError err) {
  assert(status == TM_STATUS_AWAITING_ACTIVE ||
         status == TM_STATUS_AWAITING_PASSIVE ||
         status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
         status == TM_STATUS_TRANSFERRING ||
         status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE);
  if (fld) {
    if  (err != TM_ERR_DUPE) {
      fld->removeFile(dfile);
    }
    else {
      fld->finishUpload(dfile);
    }
  }
  partialcompletestamp = timestamp;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    if (!!sls && !sls->getConn(src)->isProcessing()) {
      sls->returnConn(src, type != TM_TYPE_LIST);
      if (fls) { // unset in case of LIST
        fls->finishDownload(sfile);
      }
      transferFailed(ts, err);
      return;
    }
  }
  if (status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE ||
      status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
      (type == TM_TYPE_UPLOAD && lt == nullptr))
  {
    transferFailed(ts, err);
    return;
  }
  error = err;
  setStatus(TM_STATUS_TARGET_ERROR_AWAITING_SOURCE);
}

std::shared_ptr<TransferStatus> TransferMonitor::getTransferStatus() const {
  return ts;
}

void TransferMonitor::updateFXPSizeSpeed() {
  File * srcfile = fls->getFile(sfile);
  if (srcfile) {
    int srctouch = srcfile->getTouch();
    if (latestsrctouch != srctouch) {
      latestsrctouch = srctouch;
      unsigned long long int srcfilesize = srcfile->getSize();
      if (srcfilesize > ts->sourceSize()) {
        ts->setSourceSize(srcfilesize);
      }
    }
  }
  File * dstfile = fld->getFile(dfile);
  if (dstfile) {
    unsigned long long int dstfilesize = dstfile->getSize();
    int span = timestamp - startstamp;
    int dsttouch = dstfile->getTouch();
    if (!span) {
      span = 10;
    }

    // if the file list has been updated (as seen on the file's touch stamp
    // the speed shall be recalculated.
    if (latestdsttouch != dsttouch) {
      latestdsttouch = dsttouch;
      setTargetSizeSpeed(dstfilesize, span);
    }
    else {
      // since the actual file size has not changed since last tick,
      // interpolate an updated file size through the currently known speed
      unsigned long long int speedtemp = ts->getSpeed() * 1024;
      ts->interpolateAddSize((speedtemp * TICK_INTERVAL) / 1000);
    }
    ts->setTimeSpent(span / 1000);
  }
}

void TransferMonitor::updateLocalTransferSizeSpeed() {
  if (lt) {
    unsigned int filesize = lt->size();
    int span = timestamp - startstamp;
    if (!span) {
      span = 10;
    }
    setTargetSizeSpeed(filesize, span);
    ts->setTimeSpent(span / 1000);
  }
}

bool TransferMonitor::checkForDeadFXPTransfers() {
  if ((status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET &&
       timestamp - partialcompletestamp > MAX_WAIT_ERROR) ||
      (status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE &&
       timestamp - partialcompletestamp > MAX_WAIT_SOURCE_COMPLETE))
  {
    ts->addLogLine("[" + util::ctimeLog() + "] [Partial completion timeout reached. Disconnecting target]");
    sld->disconnectConn(dst);
    sld->connectConn(dst);
    return true;
  }
  else if (status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE &&
           timestamp - partialcompletestamp > MAX_WAIT_ERROR)
  {
    ts->addLogLine("[" + util::ctimeLog() + "] [Partial completion timeout reached. Disconnecting source]");
    sls->disconnectConn(src);
    sls->connectConn(src);
    return true;
  }
  else if (status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE &&
           timestamp - partialcompletestamp > MAX_WAIT_ERROR)
  {
    ts->addLogLine("[" + util::ctimeLog() + "] [Partial completion timeout reached. Disconnecting source]");
    sls->finishTransferGracefully(src);
    sls->disconnectConn(src);
    sls->connectConn(src);
    return true;
  }
  return false;
}

bool TransferMonitor::checkMaxTransferTime() {
  if (maxtransfertimeseconds > 0 && timestamp / 1000 > maxtransfertimeseconds) {
    if (status == TM_STATUS_TRANSFERRING || status == TM_STATUS_AWAITING_ACTIVE || status == TM_STATUS_AWAITING_PASSIVE) {
      ts->addLogLine("[" + util::ctimeLog() + "] [Timeout reached after " + std::to_string(maxtransfertimeseconds) + " seconds. Disconnecting]");
      if (sld) {
        sld->disconnectConn(dst);
        sld->connectConn(dst);
      }
      if (sls) {
        sls->disconnectConn(src);
        sls->connectConn(src);
      }
      timeout = true;
      return true;
    }
  }
  return false;
}

void TransferMonitor::setTargetSizeSpeed(unsigned long long int filesize, int span) {
  ts->setTargetSize(filesize);
  unsigned int currentspeed = ts->targetSize() / span;
  unsigned int prevspeed = ts->getSpeed();
  if (currentspeed < prevspeed) {
    ts->setSpeed(prevspeed * 0.9 + currentspeed * 0.1);
  }
  else {
    ts->setSpeed(prevspeed * 0.7 + currentspeed * 0.3);
  }
}

Status TransferMonitor::getStatus() const {
  return status;
}

void TransferMonitor::reset() {
  sls.reset();
  sld.reset();
  fls.reset();
  fld.reset();
  lt = nullptr;
  ts.reset();
  srcco.reset();
  dstco.reset();
  clientactive = true;
  fxpdstactive = true;
  ssl = false;
  timestamp = 0;
  startstamp = 0;
  partialcompletestamp = 0;
  rawbufqueue.clear();
  storeid = -1;
  maxtransfertimeseconds = -1;
  timeout = false;
}

bool TransferMonitor::willFail() const {
  return status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
         status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE;
}

void TransferMonitor::transferFailed(const std::shared_ptr<TransferStatus> & ts, TransferError err) {
  if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
      status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE)
  {
    if (error == TM_ERR_DUPE) {
      err = TM_ERR_DUPE;
    }
  }
  if (!!ts) {
    if (err == TM_ERR_DUPE) {
      ts->setDupe();
    }
    else {
      if (timeout) {
        ts->setTimeout();
      }
      else {
        ts->setFailed();
      }
    }
  }
  if (type == TM_TYPE_FXP && err != TM_ERR_DUPE) {
    std::string extension = File::getExtension(dfile);
    if (extension != "sfv" && extension != "nfo") {
      sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0, 0);
    }
  }
  tm->transferFailed(ts, err);
  setStatus(TM_STATUS_IDLE);
}

void TransferMonitor::newRawBufferLine(const std::pair<std::string, std::string> & line) {
  if (!!ts) {
    if (!rawbufqueue.empty()) {
      for (std::list<std::pair<std::string, std::string> >::const_iterator it = rawbufqueue.begin(); it != rawbufqueue.end(); it++) {
        ts->addLogLine(it->first + " " + it->second);
      }
      rawbufqueue.clear();
    }
    ts->addLogLine(line.first + " " + line.second);
  }
  else {
    rawbufqueue.push_back(line);
  }
}

void TransferMonitor::localInfo(const std::string& info) {
  std::string localtag = "[" + util::ctimeLog() + "]";
  newRawBufferLine(std::make_pair(localtag, "[" + info + "]"));
}

void TransferMonitor::localError(const std::string& info) {
  localInfo("Error: " + info);
}

void TransferMonitor::lateFXPFailure(const std::string& reason) {
  localError(reason);
  ts->setFailed();
  tm->transferFailed(ts, TM_ERR_OTHER);
  sls->returnConn(src, true);
  sld->returnConn(dst, true);
  setStatus(TM_STATUS_IDLE);
}

void TransferMonitor::lateDownloadFailure(const std::string& reason, bool dupe) {
  localError(reason);
  if (dupe) {
    ts->setDupe();
  }
  else {
    ts->setFailed();
  }
  tm->transferFailed(ts, dupe ? TM_ERR_DUPE : TM_ERR_OTHER);
  sls->returnConn(src, true);
  setStatus(TM_STATUS_IDLE);
  return;
}

void TransferMonitor::lateUploadFailure(const std::string& reason, bool dupe) {
  localError(reason);
  if (dupe) {
    ts->setDupe();
  }
  else {
    ts->setFailed();
  }
  tm->transferFailed(ts, dupe ? TM_ERR_DUPE : TM_ERR_OTHER);
  sld->returnConn(dst, true);
  setStatus(TM_STATUS_IDLE);
  return;
}

void TransferMonitor::setStatus(Status status) {
  this->status = status;
}
