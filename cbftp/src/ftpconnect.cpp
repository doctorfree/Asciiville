#include "ftpconnect.h"

#include <cassert>

#include "core/iomanager.h"
#include "site.h"
#include "ftpconnectowner.h"
#include "ftpconn.h"
#include "globalcontext.h"
#include "proxysession.h"
#include "proxy.h"

#define WELCOME_TIMEOUT_MSEC 7000

FTPConnect::FTPConnect(int id, FTPConnectOwner* owner, const Address& addr, Proxy* proxy, bool primary, bool implicittls) :
  id(id),
  owner(owner),
  databuflen(DATA_BUF_SIZE),
  databuf((char *) malloc(databuflen)),
  databufpos(0),
  addr(addr),
  resolvedaddr(addr),
  primary(primary),
  engaged(true),
  connected(false),
  welcomereceived(false),
  millisecs(0),
  implicittls(implicittls)
{
  interConnect(addr, proxy);
}

FTPConnect::~FTPConnect() {
  assert(!engaged); // must disengage before deleting; events may still be in the work queue
  free(databuf);
}

void FTPConnect::FDInterConnecting(int sockid, const std::string& addr) {
  resolvedaddr.host = addr;
  resolvedaddr.brackets = addr.find(":") != std::string::npos && resolvedaddr.port != 21;

}

void FTPConnect::FDInterConnected(int sockid) {
  if (!engaged) {
    return;
  }
  connected = true;
  millisecs = 0;
  owner->ftpConnectInfo(id, "[" + addr.toString() + "][Connection established]");
  if (implicittls) {
    negotiateSSLConnect();
  }
}

void FTPConnect::FDInterDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  FDFail(sockid, "Disconnected: " + details);
}

void FTPConnect::FDInterData(int sockid, char* data, unsigned int datalen) {
  if (!engaged) {
    return;
  }
  else {
    owner->ftpConnectInfo(id, std::string(data, datalen));
    if (FTPConn::parseData(data, datalen, &databuf, databuflen, databufpos, databufcode)) {
      if (databufcode == 220) {
        welcomereceived = true;
        owner->ftpConnectSuccess(id, resolvedaddr);
      }
      else {
        owner->ftpConnectInfo(id, "[" + addr.toString() + "][Unknown response]");
        disengage();
        owner->ftpConnectInfo(id, "[" + addr.toString() + "][Disconnected]");
        owner->ftpConnectFail(id);
      }
    }
  }
}

void FTPConnect::FDInterInfo(int sockid, const std::string& info) {
  owner->ftpConnectInfo(id, "[" + addr.toString() + "][" + info + "]");
}

void FTPConnect::FDFail(int sockid, const std::string& error) {
  if (engaged) {
    engaged = false;
    owner->ftpConnectInfo(id, "[" + addr.toString() + "][" + error + "]");
    owner->ftpConnectFail(id);
  }
}

void FTPConnect::FDSSLSuccess(int sockid, const std::string& cipher) {
  owner->ftpConnectInfo(id, "[Cipher: " + cipher + "]");
}

int FTPConnect::getId() const {
  return id;
}

int FTPConnect::handedOver() {
  engaged = false;
  return sockid;
}

Address FTPConnect::getAddress() const {
  return addr;
}

bool FTPConnect::isPrimary() const {
  return primary;
}

void FTPConnect::disengage() {
  if (engaged) {
    global->getIOManager()->closeSocket(sockid);
    engaged = false;
  }
}

void FTPConnect::tickIntern() {
  millisecs += 1000;
  if (millisecs >= WELCOME_TIMEOUT_MSEC) {
    if (engaged && connected && !welcomereceived) {
      owner->ftpConnectInfo(id, "[" + addr.toString() + "][Timeout while waiting for welcome message]");
      disengage();
      owner->ftpConnectFail(id);
    }
  }
}
