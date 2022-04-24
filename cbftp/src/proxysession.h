#pragma once

#include <string>

#include "address.h"

enum ProxySessionStatus {
  PROXYSESSION_INIT,
  PROXYSESSION_SEND,
  PROXYSESSION_ERROR,
  PROXYSESSION_SUCCESS,
  PROXYSESSION_SEND_CONNECT,
  PROXYSESSION_SEND_LISTEN,
  PROXYSESSION_RECV_LISTEN
};

enum class AddressMode {
  IPV4,
  IPV6,
  DNS
};

class Proxy;

class ProxySession {
public:
  ProxySession();
  void prepareConnect(Proxy* proxy, const Address& addr);
  void prepareListen(Proxy* proxy, Core::AddressFamily addrfam);
  int instruction() const;
  const char * getSendData() const;
  int getSendDataLen() const;
  void received(char* data, int datalen);
  std::string getErrorMessage() const;
  Address getListeningAddress() const;
  Address getReplyAddress() const;
  unsigned int getTryListenPort() const;
private:
  void setInitialData();
  void setConnectRequestData();
  void setListenRequestData();
  struct sockaddr_in* saddr;
  struct sockaddr_in6* saddr6;
  AddressMode addressmode;
  char senddata[256];
  int senddatalen;
  Proxy * proxy;
  int state;
  bool listenmode;
  std::string errormessage;
  Address addr;
  Address addrlistening;
  Address addrreply;
  unsigned int trylistenport;
};
