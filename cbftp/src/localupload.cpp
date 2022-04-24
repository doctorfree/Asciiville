#include "localupload.h"

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include "core/iomanager.h"
#include "core/util.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"
#include "localstorage.h"
#include "path.h"

LocalUpload::LocalUpload() :
  filepos(0)
{
  buf = (char *) malloc(CHUNK);
  buflen = CHUNK;
}

void LocalUpload::engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, path, filename, ssl, true, port);
  sockid = interConnect(Address(addr, port, ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4), ftpconn->getDataProxy());
}

bool LocalUpload::engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, path, filename, ssl, false);
  sockid = interListen(ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4, ftpconn->getDataProxy());
  return sockid != -1;
}

void LocalUpload::init(TransferMonitor* tm, FTPConn* ftpconn, const Path& path, const std::string& filename, bool ssl, bool passivemode, int port) {
  this->tm = tm;
  this->ftpconn = ftpconn;
  this->path = path;
  this->filename = filename;
  this->ssl = ssl;
  this->port = port;
  this->passivemode = passivemode;
  filepos = 0;
  fileopened = false;
  activate();
}

void LocalUpload::FDInterConnected(int sockid) {
  if (sockid != this->sockid) {
    return;
  }
  tm->localInfo("Connection established");
  if (ssl) {
    tm->localInfo("Performing TLS handshake");
    global->getIOManager()->negotiateSSLConnect(sockid, ftpconn->getSockId());
  }
  if (passivemode) {
    tm->activeStarted();
  }
  if (!openFile(true)) {
    return;
  }
  if (!ssl) {
    sendChunk();
  }
}

void LocalUpload::FDInterDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  if (sockid != this->sockid) {
    return;
  }
  if (fileopened) {
    filestream.close();
  }
  deactivate();
  tm->localError(details);
  tm->sourceError(TM_ERR_OTHER);
  this->sockid = -1;
}

void LocalUpload::FDSSLSuccess(int sockid, const std::string& cipher) {
  if (sockid != this->sockid) {
    return;
  }
  tm->localInfo("TLS handshake successful");
  ftpconn->printCipher(cipher);
  bool sessionreused = global->getIOManager()->getSSLSessionReused(sockid);
  tm->sslDetails(cipher, sessionreused);
  sendChunk();
}

void LocalUpload::sendChunk() {
  assert(fileopened);
  filestream.read(buf, buflen);
  if (filestream.fail() && !filestream.eof()) {
    std::string error = Core::util::getStrError(errno);
    filestream.close();
    global->getIOManager()->closeSocket(sockid);
    FDFail(sockid, "Failed reading file " + (path / filename).toString() + ": " + error);
    return;
  }
  int gcount = filestream.gcount();
  if (gcount == 0) {
    filestream.close();
    tm->localInfo("Upload finished, closing connection");
    global->getIOManager()->closeSocket(sockid);
    tm->sourceComplete();
    deactivate();
    return;
  }
  filepos += gcount;
  global->getIOManager()->sendData(sockid, buf, gcount);
}

void LocalUpload::FDInterSendComplete(int sockid) {
  if (sockid != this->sockid) {
    return;
  }
  sendChunk();
}

void LocalUpload::FDFail(int sockid, const std::string& error) {
  if (sockid == -1 || sockid != this->sockid) {
    return;
  }
  deactivate();
  tm->localError(error);
  tm->sourceError(TM_ERR_OTHER);
  this->sockid = -1;
}

void LocalUpload::FDInterData(int sockid, char* data, unsigned int len) {
  if (sockid != this->sockid) {
    return;
  }
  if (fileopened) {
    filestream.close();
  }
  deactivate();
  global->getIOManager()->closeSocket(sockid);
  tm->sourceError(TM_ERR_OTHER);
}

void LocalUpload::FDInterListening(int sockid, const Address& addr) {
  if (sockid != this->sockid) {
    return;
  }
  tm->passiveReady(addr.host, addr.port);
}

unsigned long long int LocalUpload::size() const {
  return filepos;
}
