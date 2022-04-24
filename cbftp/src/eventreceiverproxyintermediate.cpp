#include "eventreceiverproxyintermediate.h"

#include <cassert>

#include "globalcontext.h"
#include "core/iomanager.h"

#include "localstorage.h"
#include "proxy.h"
#include "proxysession.h"

EventReceiverProxyIntermediate::EventReceiverProxyIntermediate() : sockid(-1), proxy(nullptr) {

}

EventReceiverProxyIntermediate::~EventReceiverProxyIntermediate() {

}

int EventReceiverProxyIntermediate::interConnect(const Address& addr, Proxy* proxy, bool readimmediately) {
  this->proxy = proxy;
  this->connectaddr = addr;
  this->readimmediately = readimmediately;
  proxynegotiation = proxy;
  listenmode = false;
  bool resolving;
  if (proxy == nullptr) {
    sockid = global->getIOManager()->registerTCPClientSocket(this, addr.host, addr.port, resolving, addr.addrfam, readimmediately);
    if (resolving) {
      FDInterInfo(sockid, "Resolving");
    }
    else {
      FDInterInfo(sockid, "Connecting to " + addr.toString(false));
    }
  }
  else {
    proxyConnect();
  }
  return sockid;
}

int EventReceiverProxyIntermediate::interListen(Core::AddressFamily addrfam, Proxy* proxy, int parentsockid, bool readimmediately) {
  this->proxy = proxy;
  this->listenaddrfam = addrfam;
  this->readimmediately = readimmediately;
  proxynegotiation = proxy;
  listenmode = true;
  if (proxy == nullptr) {
    int startport = global->getLocalStorage()->getNextActivePort();
    int port = startport;
    bool entered = false;
    while (!entered || port != startport) {
      entered = true;
      sockid = global->getIOManager()->registerTCPServerSocket(this, port, addrfam);
      if (sockid != -1) {
        FDInterInfo(sockid, "Listening on port " + std::to_string(port));
        Core::StringResult res = addrfam == Core::AddressFamily::IPV4 ? global->getLocalStorage()->getAddress4(parentsockid) : global->getLocalStorage()->getAddress6(parentsockid);
        FDInterListening(sockid, Address(res.result, port, addrfam));
        return sockid;
      }
      port = global->getLocalStorage()->getNextActivePort();
    }
  }
  else {
    proxyConnect();
  }
  return sockid;
}

void EventReceiverProxyIntermediate::proxyConnect() {
  bool resolving;
  proxysession = std::unique_ptr<ProxySession>(new ProxySession());
  if (!listenmode) {
    proxysession->prepareConnect(proxy, connectaddr);
  }
  else {
    proxysession->prepareListen(proxy, listenaddrfam);
  }
  sockid = global->getIOManager()->registerTCPClientSocket(this, proxy->getAddr(), std::stoi(proxy->getPort()), resolving);
  if (resolving) {
    FDInterInfo(sockid, "Resolving proxy " + proxy->getAddr());
  }
  else {
    Address proxyaddr;
    proxyaddr.host = proxy->getAddr();
    proxyaddr.port = std::stoi(proxy->getPort());
    FDInterInfo(sockid, "Connecting to proxy " + proxyaddr.toString(false));
  }
}

void EventReceiverProxyIntermediate::FDInterConnecting(int sockid, const std::string& addr) {

}

void EventReceiverProxyIntermediate::FDInterInfo(int sockid, const std::string& info) {

}

void EventReceiverProxyIntermediate::FDInterNew(int sockid, int newsockid) {

}

void EventReceiverProxyIntermediate::FDInterSendComplete(int sockid) {

}

void EventReceiverProxyIntermediate::FDInterListening(int sockid, const Address& addr) {

}

void EventReceiverProxyIntermediate::FDConnecting(int sockid, const std::string& addr) {
  Address resolvedaddr;
  resolvedaddr.host = addr;
  resolvedaddr.brackets = addr.find(":") != std::string::npos && resolvedaddr.port != 21;
  FDInterInfo(sockid, std::string("Connecting to ") + (proxy ? "proxy " : "") + resolvedaddr.toString(false));
  if (!proxy) {
    FDInterConnecting(sockid, addr);
  }
}

void EventReceiverProxyIntermediate::FDConnected(int sockid) {
  if (proxynegotiation) {
    resolvedproxyaddr = global->getIOManager()->getSocketAddress(sockid);
    FDInterInfo(sockid, "Proxy connection established");
    proxySessionInit();
  }
  else {
    FDInterConnected(sockid);
  }
}

void EventReceiverProxyIntermediate::FDData(int sockid, char* data, unsigned int datalen) {
  if (proxynegotiation) {
    proxysession->received(data, datalen);
    proxySessionInit();
  }
  else {
    FDInterData(sockid, data, datalen);
  }
}

void EventReceiverProxyIntermediate::FDNew(int sockid, int newsockid) {
  assert(!proxy);
  FDInterNew(sockid, newsockid);
}

void EventReceiverProxyIntermediate::FDSendComplete(int sockid) {
  if (!proxynegotiation) {
    FDInterSendComplete(sockid);
  }
}

void EventReceiverProxyIntermediate::FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  if (proxynegotiation) {
    FDFail(sockid, (reason != Core::DisconnectType::ERROR ? "Proxy closed connection prematurely: " : "") + details);
  }
  else {
    FDInterDisconnected(sockid, reason, details);
  }
}

void EventReceiverProxyIntermediate::proxySessionInit() {
  switch (proxysession->instruction()) {
    case PROXYSESSION_SEND_CONNECT:
      FDInterInfo(sockid, "Requesting connection to " + connectaddr.toString(false) + " through proxy");
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SEND_LISTEN: {
      unsigned int port = proxysession->getTryListenPort();
      FDInterInfo(sockid, "Requesting listening" + (port ? " on port " + std::to_string(port) : "") + " through proxy");
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    }
    case PROXYSESSION_RECV_LISTEN: {
      Address addrlistening = proxysession->getListeningAddress();
      FDInterInfo(sockid, "Listening on port " + std::to_string(addrlistening.port) + " through proxy");
      std::string listeningaddr = proxy->getActiveAddressSource() == ActiveAddressSource::AUTO_BY_PROXY ? proxysession->getListeningAddress().host : resolvedproxyaddr;
      FDInterListening(sockid, Address(listeningaddr, addrlistening.port, addrlistening.addrfam));
      break;
    }
    case PROXYSESSION_SEND:
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SUCCESS:
      proxynegotiation = false;
      FDInterConnected(sockid);
      break;
    case PROXYSESSION_ERROR:
      global->getIOManager()->closeSocket(sockid);
      FDDisconnected(sockid, Core::DisconnectType::ERROR, "Proxy error: " + proxysession->getErrorMessage());
      break;
  }
}

void EventReceiverProxyIntermediate::negotiateSSLConnect(int parentsockid) {
  if (parentsockid == -1) {
    global->getIOManager()->negotiateSSLConnect(sockid);
  }
  else {
    global->getIOManager()->negotiateSSLConnectParent(sockid, parentsockid);
  }
}
