#include "proxy.h"

Proxy::Proxy() {

}

Proxy::Proxy(const std::string& name) : name(name), addr("127.0.0.1"),
    port("8080"), authmethod(PROXY_AUTH_NONE), resolvehosts(true),
    activeaddresssource(ActiveAddressSource::AUTO_BY_PROXY),
    activeportsmethod(ActivePortsMethod::AUTO_BY_PROXY),
    activeportfirst(47500), activeportlast(47600), nextport(activeportfirst)
{

}

std::string Proxy::getName() const {
  return name;
}

std::string Proxy::getAddr() const {
  return addr;
}

std::string Proxy::getPort() const {
  return port;
}

int Proxy::getAuthMethod() const {
  return authmethod;
}

std::string Proxy::getAuthMethodText() const {
  switch (authmethod) {
    case PROXY_AUTH_NONE:
      return "None";
    case PROXY_AUTH_USERPASS:
      return "User/Pass";
  }
  return "Unknown";
}

std::string Proxy::getUser() const {
  return user;
}

std::string Proxy::getPass() const {
  return pass;
}

bool Proxy::getResolveHosts() const {
  return resolvehosts;
}

ActiveAddressSource Proxy::getActiveAddressSource() const {
  return activeaddresssource;
}

ActivePortsMethod Proxy::getActivePortsMethod() const {
  return activeportsmethod;
}

int Proxy::getActivePortFirst() const {
  return activeportfirst;
}

int Proxy::getActivePortLast() const {
  return activeportlast;
}

int Proxy::getNextListenPort() const {
  if (activeportsmethod == ActivePortsMethod::AUTO_BY_PROXY) {
    return 0;
  }
  int port = nextport++;
  if (nextport >= activeportlast) {
    nextport = activeportfirst;
  }
  return port;
}

void Proxy::setName(const std::string& name) {
  this->name = name;
}

void Proxy::setAddr(const std::string& addr) {
  this->addr = addr;
}

void Proxy::setPort(const std::string& port) {
  this->port = port;
}

void Proxy::setAuthMethod(int authmethod) {
  this->authmethod = authmethod;
}

void Proxy::setUser(const std::string& user) {
  this->user = user;
}

void Proxy::setPass(const std::string& pass) {
  this->pass = pass;
}

void Proxy::setResolveHosts(bool resolve) {
  resolvehosts = resolve;
}

void Proxy::setActiveAddressSource(ActiveAddressSource source) {
  activeaddresssource = source;
}

void Proxy::setActivePortsMethod(ActivePortsMethod method) {
  activeportsmethod = method;
}

void Proxy::setActivePortFirst(int port) {
  activeportfirst = port;
  nextport = activeportfirst;
}

void Proxy::setActivePortLast(int port) {
  activeportlast = port;
}
