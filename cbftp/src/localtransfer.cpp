#include "localtransfer.h"

#include <cstdio>
#include <cerrno>

#include "core/iomanager.h"
#include "core/tickpoke.h"
#include "core/util.h"
#include "globalcontext.h"
#include "transfermonitor.h"
#include "filesystem.h"

LocalTransfer::LocalTransfer() :
  buflen(0),
  timeoutticker(false),
  inuse(false)
{
}

bool LocalTransfer::active() const {
  return inuse;
}

void LocalTransfer::FDInterNew(int sockid, int newsockid) {
  if (sockid != newsockid) {
    global->getIOManager()->closeSocket(sockid);
    this->sockid = newsockid;
    global->getIOManager()->registerTCPServerClientSocket(this, newsockid);
  }
  if (timeoutticker) {
    global->getTickPoke()->stopPoke(this, 0);
    timeoutticker = false;
  }
  FDInterConnected(newsockid);
}

void LocalTransfer::FDInterInfo(int sockid, const std::string& info) {
  if (this->sockid != -1 && this->sockid != sockid) {
    return;
  }
  tm->localInfo(info);
}

void LocalTransfer::tick(int) {
  global->getIOManager()->closeSocket(sockid);
  global->getTickPoke()->stopPoke(this, 0);
  FDFail(sockid, "Connection timed out");
}

bool LocalTransfer::openFile(bool read) {
  if (read ? !FileSystem::fileExistsReadable(path) : !FileSystem::fileExistsWritable(path)) {
    global->getIOManager()->closeSocket(sockid);
    FDFail(sockid, std::string("Failed to access " + path.toString()).c_str());
    return false;
  }
  filestream.clear();
  filestream.open((path / filename).toString().c_str(), std::ios::binary | (read ? std::ios::in : (std::ios::ate | std::ios::out)));
  if (filestream.fail()) {
    std::string error = Core::util::getStrError(errno);
    filestream.close();
    global->getIOManager()->closeSocket(sockid);
    FDFail(sockid, "Failed to open file " + (path / filename).toString() + ": " + error);
    return false;
  }
  fileopened = true;
  return true;
}

int LocalTransfer::getPort() const {
  return port;
}

FTPConn * LocalTransfer::getConn() const {
  return ftpconn;
}

void LocalTransfer::activate() {
  inuse = true;
  timeoutticker = false;
  if (!passivemode) {
    global->getTickPoke()->startPoke(this, "LocalTransfer", 5000, 0);
    timeoutticker = true;
  }
}

void LocalTransfer::deactivate() {
  inuse = false;
  if (timeoutticker) {
    global->getTickPoke()->stopPoke(this, 0);
    timeoutticker = false;
  }
}
