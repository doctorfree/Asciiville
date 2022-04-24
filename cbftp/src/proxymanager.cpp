#include "proxymanager.h"

#include <algorithm>

#include "proxy.h"
#include "eventlog.h"

namespace {

bool proxyNameComparator(Proxy* a, Proxy* b) {
  return a->getName().compare(b->getName()) < 0;
}

}

ProxyManager::ProxyManager() : defaultproxy(nullptr), defaultdataproxy(nullptr) {

}

void ProxyManager::addProxy(Proxy* proxy) {
  proxies.push_back(proxy);
  sortProxys();
}

Proxy* ProxyManager::getProxy(const std::string& name) const {
  for (std::vector<Proxy *>::const_iterator it = proxies.begin(); it != proxies.end(); it++) {
    if ((*it)->getName() == name) {
      return *it;
    }
  }
  return nullptr;
}

void ProxyManager::removeProxy(const std::string& name) {
  for (std::vector<Proxy *>::iterator it = proxies.begin(); it != proxies.end(); it++) {
    if ((*it)->getName() == name) {
      delete *it;
      proxies.erase(it);
      return;
    }
  }
}

std::vector<Proxy*>::const_iterator ProxyManager::begin() const {
  return proxies.begin();
}

std::vector<Proxy*>::const_iterator ProxyManager::end() const {
  return proxies.end();
}

bool ProxyManager::hasDefaultProxy() const {
  return defaultproxy != nullptr;
}

bool ProxyManager::hasDefaultDataProxy() const {
  return defaultdataproxy != nullptr;
}

Proxy* ProxyManager::getDefaultProxy() const {
  return defaultproxy;
}

Proxy* ProxyManager::getDefaultDataProxy() const {
  return defaultdataproxy;
}

void ProxyManager::setDefaultProxy(const std::string& proxy) {
  defaultproxy = getProxy(proxy);
}

void ProxyManager::setDefaultDataProxy(const std::string& proxy) {
  defaultdataproxy = getProxy(proxy);
}

void ProxyManager::sortProxys() {
  std::sort(proxies.begin(), proxies.end(), proxyNameComparator);
}

unsigned int ProxyManager::size() const {
  return proxies.size();
}
