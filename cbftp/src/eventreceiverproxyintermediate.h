#pragma once

#include <memory>

#include "core/eventreceiver.h"
#include "core/types.h"

#include "address.h"

class Proxy;
class ProxySession;

class EventReceiverProxyIntermediate : public Core::EventReceiver {
public:
  EventReceiverProxyIntermediate();
  ~EventReceiverProxyIntermediate();
  virtual void FDInterConnecting(int sockid, const std::string& addr);
  virtual void FDInterInfo(int sockid, const std::string& info);
  virtual void FDInterNew(int sockid, int newsockid);
  virtual void FDInterSendComplete(int sockid);
  virtual void FDInterListening(int sockid, const Address& addr);
  virtual void FDInterConnected(int sockid) = 0;
  virtual void FDInterData(int sockid, char* data, unsigned int datalen) = 0;
  virtual void FDInterDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) = 0;
protected:
  int interConnect(const Address& addr, Proxy* proxy = nullptr, bool listenimmediately = true);
  int interListen(Core::AddressFamily addrfam, Proxy* proxy = nullptr, int parentsockid = -1, bool listenimmediately = true);
  void negotiateSSLConnect(int parentsockid = -1);
  int sockid;
private:
  void FDConnecting(int sockid, const std::string& addr) final override;
  void FDConnected(int sockid) final override;
  void FDData(int sockid, char* data, unsigned int datalen) final override;
  void FDNew(int sockid, int newsockid) final override;
  void FDSendComplete(int sockid) final override;
  void FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) final override;
  void proxySessionInit();
  void proxyConnect();
  Proxy* proxy;
  Address connectaddr;
  Core::AddressFamily listenaddrfam;
  std::unique_ptr<ProxySession> proxysession;
  bool proxynegotiation;
  bool listenmode;
  bool readimmediately;
  std::string resolvedproxyaddr;
};
