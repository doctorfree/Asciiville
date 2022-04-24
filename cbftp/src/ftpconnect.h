#pragma once

#include <memory>
#include <string>

#include "eventreceiverproxyintermediate.h"

#include "address.h"

class FTPConnectOwner;
class ProxySession;
class Proxy;
struct Address;

class FTPConnect : public EventReceiverProxyIntermediate {
public:
  FTPConnect(int id, FTPConnectOwner* owner, const Address& addr, Proxy* proxy, bool primary, bool implicittls);
  ~FTPConnect();
  int getId() const;
  int handedOver();
  Address getAddress() const;
  bool isPrimary() const;
  void disengage();
  void tickIntern();
private:
  void FDInterConnecting(int sockid, const std::string& addr) override;
  void FDInterConnected(int sockid) override;
  void FDInterData(int sockid, char* data, unsigned int datalen) override;
  void FDInterInfo(int sockid, const std::string& info) override;
  void FDInterDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
  void FDFail(int sockid, const std::string& error) override;
  void FDSSLSuccess(int sockid, const std::string& cipher) override;
  void proxySessionInit();
  int id;
  FTPConnectOwner * owner;
  unsigned int databuflen;
  char * databuf;
  unsigned int databufpos;
  int databufcode;
  Address addr;
  Address resolvedaddr;
  bool primary;
  bool engaged;
  bool connected;
  bool welcomereceived;
  int millisecs;
  bool implicittls;
};
