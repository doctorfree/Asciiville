#pragma once

#include <vector>
#include <string>

class Proxy;

class ProxyManager {
public:
  ProxyManager();
  void addProxy(Proxy* proxy);
  Proxy* getProxy(const std::string& proxy) const;
  void removeProxy(const std::string& proxy);
  std::vector<Proxy *>::const_iterator begin() const;
  std::vector<Proxy *>::const_iterator end() const;
  bool hasDefaultProxy() const;
  bool hasDefaultDataProxy() const;
  Proxy* getDefaultProxy() const;
  Proxy* getDefaultDataProxy() const;
  void setDefaultProxy(const std::string& proxy);
  void setDefaultDataProxy(const std::string& proxy);
  void sortProxys();
  unsigned int size() const;
private:
  std::vector<Proxy*> proxies;
  Proxy* defaultproxy;
  Proxy* defaultdataproxy;
};
