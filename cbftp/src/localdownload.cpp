#include "localdownload.h"

#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include "address.h"
#include "core/iomanager.h"
#include "core/types.h"
#include "core/util.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"
#include "localstorage.h"
#include "path.h"

LocalDownload::LocalDownload(LocalStorage* ls) :
  ls(ls)
{
}

void LocalDownload::engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, path, filename, false, -1, ssl, true, port);
  sockid = -1;
  sockid = interConnect(Address(addr, port, ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4), ftpconn->getDataProxy());
}

bool LocalDownload::engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, path, filename, false, -1, ssl, false);
  sockid = -1;
  sockid = interListen(ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4, ftpconn->getDataProxy(), ftpconn->getSockId());
  return sockid != -1;
}

void LocalDownload::engage(TransferMonitor* tm, int storeid, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, "", "", true, storeid, ssl, true, port);
  sockid = -1;
  sockid = interConnect(Address(addr, port, ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4), ftpconn->getDataProxy());
}

bool LocalDownload::engage(TransferMonitor* tm, int storeid, bool ipv6, bool ssl, FTPConn* ftpconn) {
  init(tm, ftpconn, "", "", true, storeid, ssl, false);
  sockid = -1;
  sockid = interListen(ipv6 ? Core::AddressFamily::IPV6 : Core::AddressFamily::IPV4, ftpconn->getDataProxy(), ftpconn->getSockId());
  return sockid != -1;
}

void LocalDownload::init(TransferMonitor* tm, FTPConn* ftpconn, const Path& path, const std::string& filename, bool inmemory, int storeid, bool ssl, bool passivemode, int port) {
  this->tm = tm;
  this->ftpconn = ftpconn;
  this->path = path;
  this->filename = filename;
  this->inmemory = inmemory;
  this->storeid = storeid;
  this->ssl = ssl;
  this->port = port;
  this->passivemode = passivemode;
  bufpos = 0;
  filesize = 0;
  fileopened = false;
  activate();
}

void LocalDownload::FDInterConnected(int sockid) {
  if (this->sockid == -1 || sockid != this->sockid) {
    return;
  }
  tm->localInfo("Connection established");
  if (ssl) {
    tm->localInfo("Performing TLS handshake");
    negotiateSSLConnect(ftpconn->getSockId());
  }
  if (passivemode) {
    tm->activeStarted();
  }
}

void LocalDownload::FDInterDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  if (this->sockid == -1 || sockid != this->sockid) {
    return;
  }
  if (!inmemory) {
    if (bufpos > 0) {
      if (!fileopened) {
        if (!openFile(false)) {
          return;
        }
      }
      filestream.write(buf, bufpos);
    }
    if (fileopened) {
      filestream.close();
    }
  }
  deactivate();
  if (inmemory) {
    Core::BinaryData out(buf, buf + bufpos);
    ls->storeContent(storeid, out);
  }
  if (reason != Core::DisconnectType::ERROR) {
    tm->localInfo(details);
    tm->targetComplete();
  }
  else {
    tm->localError(details);
    tm->targetError(TM_ERR_RETRSTOR_COMPLETE);
  }
  this->sockid = -1;
}

void LocalDownload::FDSSLSuccess(int sockid, const std::string& cipher) {
  if (this->sockid == -1 || sockid != this->sockid) {
    return;
  }
  tm->localInfo("TLS handshake successful");
  ftpconn->printCipher(cipher);
  bool sessionreused = global->getIOManager()->getSSLSessionReused(sockid);
  tm->sslDetails(cipher, sessionreused);
}

void LocalDownload::FDInterData(int sockid, char* data, unsigned int len) {
  if (this->sockid == -1 || sockid != this->sockid) {
    return;
  }
  append(data, len);
}

void LocalDownload::FDInterListening(int sockid, const Address& addr) {
  if (this->sockid == -1 || sockid != this->sockid) {
    return;
  }
  tm->passiveReady(addr.host, addr.port);
}

void LocalDownload::FDFail(int sockid, const std::string& error) {
  if (this->sockid == -1 || sockid != this->sockid) {
    return;
  }
  deactivate();
  tm->localError(error);
  tm->targetError(TM_ERR_OTHER);
  this->sockid = -1;
}

unsigned long long int LocalDownload::size() const {
  return filesize + bufpos;
}

void LocalDownload::append(char* data, unsigned int datalen) {
  if (!buflen) {
    buf = (char*) malloc(CHUNK);
    buflen = CHUNK;
  }
  if (bufpos + datalen > buflen) {
    if (inmemory) {
      while (bufpos + datalen > buflen) {
        buflen *= 2;
      }
      char* newbuf = static_cast<char*>(malloc(buflen));
      global->getEventLog()->log("test", "Increasing to " + std::to_string(buflen));
      memcpy(newbuf, buf, bufpos);
      delete buf;
      buf = newbuf;
    }
    else {
      if (!fileopened) {
        if (!openFile(false)) {
          return;
        }
      }
      filestream.write(buf, bufpos);
      if (filestream.fail()) {
        std::string error = Core::util::getStrError(errno);
        filestream.close();
        global->getIOManager()->closeSocket(sockid);
        FDFail(sockid, "Failed writing file " + (path / filename).toString() + ": " + error);
        return;
      }
      filesize = filestream.tellg();
      bufpos = 0;
    }
  }
  memcpy(buf + bufpos, data, datalen);
  bufpos += datalen;
}

int LocalDownload::getStoreId() const {
  return storeid;
}
