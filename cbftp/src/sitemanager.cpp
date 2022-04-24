#include "sitemanager.h"

#include <cassert>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <set>

#include "globalcontext.h"
#include "connstatetracker.h"
#include "eventlog.h"
#include "site.h"
#include "util.h"

#define DEFAULTUSERNAME "anonymous"
#define DEFAULTPASSWORD "anonymous"
#define DEFAULTMAXLOGINS 3
#define DEFAULTMAXUP -1
#define DEFAULTMAXDOWN 2
#define DEFAULTMAXIDLETIME 60
#define DEFAULTTLSMODE TLSMode::AUTH_TLS
#define DEFAULTSSLTRANSFER SITE_SSL_PREFER_OFF

bool siteNameComparator(const std::shared_ptr<Site> & a, const std::shared_ptr<Site> & b) {
  return util::naturalComparator()(a->getName(), b->getName());
}

SiteManager::SiteManager() :
  defaultusername(DEFAULTUSERNAME),
  defaultpassword(DEFAULTPASSWORD),
  defaultmaxlogins(DEFAULTMAXLOGINS),
  defaultmaxup(DEFAULTMAXUP),
  defaultmaxdown(DEFAULTMAXDOWN),
  defaultmaxidletime(DEFAULTMAXIDLETIME),
  defaultssltransfer(DEFAULTSSLTRANSFER),
  defaulttlsmode(DEFAULTTLSMODE)
{
}

int SiteManager::getNumSites() const {
  return sites.size();
}

std::shared_ptr<Site> SiteManager::createNewSite() const {
  std::shared_ptr<Site> site = std::make_shared<Site>("SUNET");
  site->setUser(getDefaultUserName());
  site->setPass(getDefaultPassword());
  site->setMaxLogins(getDefaultMaxLogins());
  site->setMaxUp(getDefaultMaxUp());
  site->setMaxDn(getDefaultMaxDown());
  site->setTLSMode(getDefaultTLSMode());
  site->setSSLTransferPolicy(getDefaultSSLTransferPolicy());
  site->setMaxIdleTime(getDefaultMaxIdleTime());
  return site;
}

std::shared_ptr<Site> SiteManager::getSite(const std::string & site) const {
  std::vector<std::shared_ptr<Site> >::const_iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getName().compare(site) == 0) {
      return *it;
    }
  }
  return std::shared_ptr<Site>();
}

std::shared_ptr<Site> SiteManager::getSite(unsigned int index) const {
  assert(index < sites.size());
  return sites[index];
}

void SiteManager::deleteSite(const std::string & site) {
  std::vector<std::shared_ptr<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getName().compare(site) == 0) {
      purgeSite(*it);
      sites.erase(it);
      global->getEventLog()->log("SiteManager", "Site " + site + " deleted.");
      return;
    }
  }
}

void SiteManager::addSite(const std::shared_ptr<Site> & site) {
  sites.push_back(site);
  std::set<std::shared_ptr<Site> >::const_iterator it;
  for (it = site->exceptSourceSitesBegin(); it != site->exceptSourceSitesEnd(); it++) {
    addExceptSourceForSite(site->getName(), (*it)->getName());
  }
  for (it = site->exceptTargetSitesBegin(); it != site->exceptTargetSitesEnd(); it++) {
    addExceptTargetForSite(site->getName(), (*it)->getName());
  }
  global->getEventLog()->log("SiteManager", "Site " + site->getName() + " added.");
  sortSites();
}

void SiteManager::addSiteLoad(const std::shared_ptr<Site> & site) {
  sites.push_back(site);
}

void SiteManager::sortSites() {
  std::sort(sites.begin(), sites.end(), siteNameComparator);
}

std::vector<std::shared_ptr<Site> >::const_iterator SiteManager::begin() const {
  return sites.begin();
}

std::vector<std::shared_ptr<Site> >::const_iterator SiteManager::end() const {
  return sites.end();
}

std::string SiteManager::getDefaultUserName() const {
  return defaultusername;
}

void SiteManager::setDefaultUserName(const std::string & username) {
  defaultusername = username;
}

std::string SiteManager::getDefaultPassword() const {
  return defaultpassword;
}

void SiteManager::setDefaultPassword(const std::string & password) {
  defaultpassword = password;
}

int SiteManager::getDefaultMaxLogins() const {
  return defaultmaxlogins;
}

void SiteManager::setDefaultMaxLogins(int maxlogins) {
  defaultmaxlogins = maxlogins;
}

int SiteManager::getDefaultMaxUp() const {
  return defaultmaxup;
}

void SiteManager::setDefaultMaxUp(int maxup) {
  defaultmaxup = maxup;
}

int SiteManager::getDefaultMaxDown() const {
  return defaultmaxdown;
}

void SiteManager::setDefaultMaxDown(int maxdown) {
  defaultmaxdown = maxdown;
}

unsigned int SiteManager::getDefaultMaxIdleTime() const {
  return defaultmaxidletime;
}

void SiteManager::setDefaultMaxIdleTime(unsigned int idletime) {
  defaultmaxidletime = idletime;
}

TLSMode SiteManager::getDefaultTLSMode() const {
  return defaulttlsmode;
}

void SiteManager::setDefaultTLSMode(TLSMode mode) {
  defaulttlsmode = mode;
}

int SiteManager::getDefaultSSLTransferPolicy() const {
  return defaultssltransfer;
}

void SiteManager::setDefaultSSLTransferPolicy(int policy) {
  defaultssltransfer = policy;
}

void SiteManager::proxyRemoved(const std::string & removedproxy) {
  std::vector<std::shared_ptr<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getProxyType() == SITE_PROXY_USE && (*it)->getProxy() == removedproxy) {
      (*it)->setProxyType(SITE_PROXY_GLOBAL);
      global->getEventLog()->log("SiteManager", "Used proxy (" + removedproxy + ") was removed, reset proxy type for " + (*it)->getName());
    }
  }
}

void SiteManager::purgeSite(const std::shared_ptr<Site>& site) {
  std::vector<std::shared_ptr<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    (*it)->purgeOtherSite(site);
  }
}

void SiteManager::resetSitePairsForSite(const std::string & site) {
  std::shared_ptr<Site> sitep = getSite(site);
  if (!sitep) {
    return;
  }
  sitep->clearExceptSites();
  std::vector<std::shared_ptr<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if (*it == sitep) {
      continue;
    }
    if (sitep->getTransferSourcePolicy() == SITE_TRANSFER_POLICY_ALLOW) {
      (*it)->addAllowedTargetSite(sitep);
    }
    else {
      (*it)->addBlockedTargetSite(sitep);
    }
    if (sitep->getTransferTargetPolicy() == SITE_TRANSFER_POLICY_ALLOW) {
      (*it)->addAllowedSourceSite(sitep);
    }
    else {
      (*it)->addBlockedSourceSite(sitep);
    }
  }
}

void SiteManager::addExceptSourceForSite(const std::string & site, const std::string & exceptsite) {
  std::shared_ptr<Site> sitep = getSite(site);
  std::shared_ptr<Site> exceptsitep = getSite(exceptsite);
  if (!sitep || !exceptsitep || sitep == exceptsitep) {
    return;
  }
  if (sitep->getTransferSourcePolicy() == SITE_TRANSFER_POLICY_ALLOW) {
    sitep->addBlockedSourceSite(exceptsitep);
    exceptsitep->addBlockedTargetSite(sitep);
  }
  else {
    sitep->addAllowedSourceSite(exceptsitep);
    exceptsitep->addAllowedTargetSite(sitep);
  }
}

void SiteManager::addExceptTargetForSite(const std::string & site, const std::string & exceptsite) {
  std::shared_ptr<Site> sitep = getSite(site);
  std::shared_ptr<Site> exceptsitep = getSite(exceptsite);
  if (!sitep || !exceptsitep || sitep == exceptsitep) {
    return;
  }
  if (sitep->getTransferTargetPolicy() == SITE_TRANSFER_POLICY_ALLOW) {
    sitep->addBlockedTargetSite(exceptsitep);
    exceptsitep->addBlockedSourceSite(sitep);
  }
  else {
    sitep->addAllowedTargetSite(exceptsitep);
    exceptsitep->addAllowedSourceSite(sitep);
  }
}

void SiteManager::resetHourlyStats() {
  std::vector<std::shared_ptr<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    (*it)->resetHourlyStats();
  }
}

void SiteManager::resetAllStats() {
  std::vector<std::shared_ptr<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    (*it)->resetAllStats();
  }
}
