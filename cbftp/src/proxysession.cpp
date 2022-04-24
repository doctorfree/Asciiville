#include "proxysession.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>

#include "proxy.h"

#define PROXYSESSION_SOCKSVERSION 5
#define PROXYSESSION_AUTHTYPESSUPPORTED 1
#define PROXYSESSION_AUTH_NONE 0
#define PROXYSESSION_AUTH_USERPASS 2
#define PROXYSESSION_AUTH_VERSION 1
#define PROXYSESSION_STATUS_SUCCESS 0
#define PROXYSESSION_TCP_STREAM 1
#define PROXYSESSION_TCP_BIND 2
#define PROXYSESSION_UDP 3
#define PROXYSESSION_RESERVED 0
#define PROXYSESSION_ADDR_IPV4 1
#define PROXYSESSION_ADDR_DNS 3
#define PROXYSESSION_ADDR_IPV6 4

namespace {

std::string getConnectError(char c) {
  switch (c) {
    case 1:
      return "General failure";
    case 2:
      return "Connection not allowed by ruleset";
    case 3:
      return "Network unreachable";
    case 4:
      return "Host unreachable";
    case 5:
      return "Connection refused by destination host";
    case 6:
      return "TTL expired";
    case 7:
      return "Command not supported / protocol error";
    case 8:
      return "Address type not supported";
  }
  return "Unknown error code: " + std::to_string(c);
}

struct ParsedAddress {
  Address addr;
  size_t size;
};

ParsedAddress parseAddress(char* data) {
  ParsedAddress out;
  if (data[0] == PROXYSESSION_ADDR_IPV4) {
    out.size = 7;
    out.addr.port = ntohs(*reinterpret_cast<unsigned short*>(data + 5));
    char buf[INET_ADDRSTRLEN];
    out.addr.host = inet_ntop(AF_INET, data + 1, buf, INET_ADDRSTRLEN);
  }
  else if (data[0] == PROXYSESSION_ADDR_DNS) {
    size_t len = data[1];
    out.addr.host = std::string(data + 1, len);
    out.addr.port = ntohs(*reinterpret_cast<unsigned short*>(data + 1 + len));
    out.size = len + 3;
  }
  else if (data[0] == PROXYSESSION_ADDR_IPV6) {
    out.size = 19;
    out.addr.port = ntohs(*reinterpret_cast<unsigned short*>(data + 17));
    char buf[INET6_ADDRSTRLEN];
    out.addr.host = inet_ntop(AF_INET, data + 1, buf, INET6_ADDRSTRLEN);
  }
  else {
    out.size = 0;
  }
  return out;
}

}

ProxySession::ProxySession() :
  state(PROXYSESSION_INIT)
{
}

void ProxySession::prepareConnect(Proxy* proxy, const Address& addr) {
  this->proxy = proxy;
  this->addr = addr;
  listenmode = false;
  state = PROXYSESSION_INIT;
  setInitialData();
  struct addrinfo request, *result;
  memset(&request, 0, sizeof(request));
  if (addr.addrfam == Core::AddressFamily::IPV4) {
    request.ai_family = AF_INET;
  }
  else if (addr.addrfam == Core::AddressFamily::IPV6) {
    request.ai_family = AF_INET6;
  }
  else {
    request.ai_family = AF_UNSPEC;
  }
  request.ai_socktype = SOCK_STREAM;
  if (proxy->getResolveHosts()) {
    request.ai_flags |= AI_NUMERICHOST;
  }
  std::string port = std::to_string(addr.port);
  int returncode = getaddrinfo(addr.host.data(), port.data(), &request, &result);
  if (returncode == EAI_NONAME) {
    addressmode = AddressMode::DNS;
    return;
  }
  if (returncode != 0) {
    state = PROXYSESSION_ERROR;
    errormessage = std::string("Failed to retrieve address information: ") + gai_strerror(returncode);
    return;
  }
  if (result->ai_family == AF_INET) {
    saddr = (struct sockaddr_in*)result->ai_addr;
    addressmode = AddressMode::IPV4;
  }
  else {
    saddr6 = (struct sockaddr_in6*)result->ai_addr;
    addressmode = AddressMode::IPV6;
  }
}

void ProxySession::prepareListen(Proxy* proxy, Core::AddressFamily addrfam) {
  this->proxy = proxy;
  listenmode = true;
  state = PROXYSESSION_INIT;
  setInitialData();
  struct addrinfo request, *result;
  memset(&request, 0, sizeof(request));
  std::string host;
  if (addrfam == Core::AddressFamily::IPV4) {
    request.ai_family = AF_INET;
    host = "0.0.0.0";
  }
  else {
    request.ai_family = AF_INET6;
    host = "::";
  }
  request.ai_socktype = SOCK_STREAM;
  trylistenport = proxy->getNextListenPort();
  std::string port = std::to_string(trylistenport);
  int status = getaddrinfo(host.data(), port.data(), &request, &result);
  if (status != 0) {
    state = PROXYSESSION_ERROR;
    errormessage = "Failed to retrieve address information";
    return;
  }
  if (result->ai_family == AF_INET) {
    saddr = (struct sockaddr_in*)result->ai_addr;
    addressmode = AddressMode::IPV4;
  }
  else {
    saddr6 = (struct sockaddr_in6*)result->ai_addr;
    addressmode = AddressMode::IPV6;
  }
}

int ProxySession::instruction() const {
  if (state == PROXYSESSION_INIT) {
    return PROXYSESSION_SEND;
  }
  return state;
}

const char * ProxySession::getSendData() const {
  return senddata;
}

int ProxySession::getSendDataLen() const {
  return senddatalen;
}

Address ProxySession::getListeningAddress() const {
  return addrlistening;
}

Address ProxySession::getReplyAddress() const {
  return addrreply;
}

unsigned int ProxySession::getTryListenPort() const {
  return trylistenport;
}

void ProxySession::received(char* data, int datalen) {
  switch (state) {
    case PROXYSESSION_INIT:
      if (datalen < 2) {
        state = PROXYSESSION_ERROR;
        errormessage = "Malformed response on session request";
        break;
      }
      if (data[0] != PROXYSESSION_SOCKSVERSION) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid SOCKS version in response";
        break;
      }
      if (data[1] != senddata[2]) {
        state = PROXYSESSION_ERROR;
        errormessage = "Unsupported authentication type";
        break;
      }
      if (data[1] == PROXYSESSION_AUTH_NONE) {
        if (listenmode) {
          state = PROXYSESSION_SEND_LISTEN;
          setListenRequestData();
        }
        else {
          state = PROXYSESSION_SEND_CONNECT;
          setConnectRequestData();
        }
        break;
      }
      state = PROXYSESSION_SEND;
      senddata[0] = PROXYSESSION_AUTH_VERSION;
      if (proxy->getAuthMethod() == PROXY_AUTH_USERPASS) {
        int pos = proxy->getUser().length();
        senddata[1] = (char) pos;
        memcpy((void *) &senddata[2], proxy->getUser().c_str(), pos);
        pos = pos + 2;
        int passlen = proxy->getPass().length();
        senddata[pos] = (char) passlen;
        pos = pos + 1;
        memcpy((void *) &senddata[pos], proxy->getPass().c_str(), passlen);
        senddatalen = pos + passlen;
      }
      break;
    case PROXYSESSION_SEND:
      if (datalen < 2) {
        state = PROXYSESSION_ERROR;
        errormessage = "Malformed response on authentication request";
        break;
      }
      if (data[0] != PROXYSESSION_AUTH_VERSION) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid authentication version";
        break;
      }
      if (data[1] != PROXYSESSION_STATUS_SUCCESS) {
        state = PROXYSESSION_ERROR;
        errormessage = "Authentication failed";
        break;
      }
      if (listenmode) {
        state = PROXYSESSION_SEND_LISTEN;
        setListenRequestData();
      }
      else {
        state = PROXYSESSION_SEND_CONNECT;
        setConnectRequestData();
      }
      break;
    case PROXYSESSION_SEND_CONNECT:
      if (datalen < 10) {
        state = PROXYSESSION_ERROR;
        errormessage = "Malformed response on connect request";
        break;
      }
      if (data[0] != PROXYSESSION_SOCKSVERSION) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid SOCKS version in response";
        break;
      }
      if (data[1] != PROXYSESSION_STATUS_SUCCESS) {
        state = PROXYSESSION_ERROR;
        errormessage = getConnectError(data[1]);
        break;
      }
      state = PROXYSESSION_SUCCESS;
      break;
    case PROXYSESSION_SEND_LISTEN: {
      if ((addr.addrfam == Core::AddressFamily::IPV4 && datalen < 10) ||
          (addr.addrfam == Core::AddressFamily::IPV6 && datalen < 22))
      {
        state = PROXYSESSION_ERROR;
        errormessage = "Malformed response on connect request";
        break;
      }
      if (data[0] != PROXYSESSION_SOCKSVERSION) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid SOCKS version in response";
        break;
      }
      if (data[1] != PROXYSESSION_STATUS_SUCCESS) {
        state = PROXYSESSION_ERROR;
        errormessage = getConnectError(data[1]);
        break;
      }
      ParsedAddress paddr = parseAddress(data + 3);
      addrlistening = paddr.addr;
      state = PROXYSESSION_RECV_LISTEN;
      break;
    }
    case PROXYSESSION_RECV_LISTEN: {
      if ((addr.addrfam == Core::AddressFamily::IPV4 && datalen < 10) ||
          (addr.addrfam == Core::AddressFamily::IPV6 && datalen < 22))
      {
        state = PROXYSESSION_ERROR;
        errormessage = "Malformed response on connect request";
        break;
      }
      if (data[0] != PROXYSESSION_SOCKSVERSION) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid SOCKS version in response";
        break;
      }
      if (data[1] != PROXYSESSION_STATUS_SUCCESS) {
        state = PROXYSESSION_ERROR;
        errormessage = getConnectError(data[1]);
        break;
      }
      ParsedAddress paddr = parseAddress(data + 3);
      addrreply = paddr.addr;
      state = PROXYSESSION_SUCCESS;
      break;
    }
  }
}

void ProxySession::setInitialData() {
  senddata[0] = PROXYSESSION_SOCKSVERSION;
  senddata[1] = PROXYSESSION_AUTHTYPESSUPPORTED;
  if (proxy->getAuthMethod() == PROXY_AUTH_NONE) {
    senddata[2] = PROXYSESSION_AUTH_NONE;
  }
  else {
    senddata[2] = PROXYSESSION_AUTH_USERPASS;
  }
  senddatalen = 3;
}

void ProxySession::setConnectRequestData() {
  senddata[0] = PROXYSESSION_SOCKSVERSION;
  senddata[1] = PROXYSESSION_TCP_STREAM;
  senddata[2] = PROXYSESSION_RESERVED;
  if (addressmode == AddressMode::IPV4) {
    senddata[3] = PROXYSESSION_ADDR_IPV4;
    memcpy((void *) &senddata[4], &saddr->sin_addr.s_addr, 4);
    memcpy((void *) &senddata[8], &saddr->sin_port, 2);
    senddatalen = 10;
  }
  else if (addressmode == AddressMode::IPV6) {
    senddata[3] = PROXYSESSION_ADDR_IPV6;
    memcpy((void *) &senddata[4], &saddr6->sin6_addr.s6_addr, 16);
    memcpy((void *) &senddata[20], &saddr->sin_port, 2);
    senddatalen = 22;
  }
  else {
    senddata[3] = PROXYSESSION_ADDR_DNS;
    size_t len = addr.host.length();
    senddata[4] = len;
    unsigned short port = htons(addr.port);
    memcpy((void *) &senddata[5], addr.host.data(), len);
    memcpy((void *) &senddata[5 + len], &port, 2);
    senddatalen = 7 + len;
  }
}

void ProxySession::setListenRequestData() {
  senddata[0] = PROXYSESSION_SOCKSVERSION;
  senddata[1] = PROXYSESSION_TCP_BIND;
  senddata[2] = PROXYSESSION_RESERVED;
  if (addressmode == AddressMode::IPV4) {
    senddata[3] = PROXYSESSION_ADDR_IPV4;
    memcpy((void *) &senddata[4], &saddr->sin_addr.s_addr, 4);
    memcpy((void *) &senddata[8], &saddr->sin_port, 2);
    senddatalen = 10;
  }
  else if (addressmode == AddressMode::IPV6) {
    senddata[3] = PROXYSESSION_ADDR_IPV6;
    memcpy((void *) &senddata[4], &saddr6->sin6_addr.s6_addr, 16);
    memcpy((void *) &senddata[20], &saddr->sin_port, 2);
    senddatalen = 22;
  }
}

std::string ProxySession::getErrorMessage() const {
  return errormessage;
}
