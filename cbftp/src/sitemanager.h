#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "site.h"

class SiteManager {
  private:
    void purgeSite(const std::shared_ptr<Site>& site);
    std::vector<std::shared_ptr<Site> > sites;
    std::string defaultusername;
    std::string defaultpassword;
    int defaultmaxlogins;
    int defaultmaxup;
    int defaultmaxdown;
    unsigned int defaultmaxidletime;
    int defaultssltransfer;
    TLSMode defaulttlsmode;
  public:
    SiteManager();
    int getNumSites() const;
    std::shared_ptr<Site> createNewSite() const;
    void addSite(const std::shared_ptr<Site> &);
    void addSiteLoad(const std::shared_ptr<Site> &);
    std::shared_ptr<Site> getSite(const std::string &) const;
    std::shared_ptr<Site> getSite(unsigned int) const;
    void deleteSite(const std::string &);
    std::vector<std::shared_ptr<Site> >::const_iterator begin() const;
    std::vector<std::shared_ptr<Site> >::const_iterator end() const;
    std::string getDefaultUserName() const;
    void setDefaultUserName(const std::string &);
    std::string getDefaultPassword() const;
    void setDefaultPassword(const std::string &);
    int getDefaultMaxLogins() const;
    void setDefaultMaxLogins(int maxlogins);
    int getDefaultMaxUp() const;
    void setDefaultMaxUp(int maxup);
    int getDefaultMaxDown() const;
    void setDefaultMaxDown(int maxdown);
    unsigned int getDefaultMaxIdleTime() const;
    void setDefaultMaxIdleTime(unsigned int);
    TLSMode getDefaultTLSMode() const;
    void setDefaultTLSMode(TLSMode mode);
    int getDefaultSSLTransferPolicy() const;
    void setDefaultSSLTransferPolicy(int);
    void sortSites();
    void proxyRemoved(const std::string &);
    void resetSitePairsForSite(const std::string &);
    void addExceptSourceForSite(const std::string &, const std::string &);
    void addExceptTargetForSite(const std::string &, const std::string &);
    void resetHourlyStats();
    void resetAllStats();
};
