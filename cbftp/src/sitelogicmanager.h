#pragma once

#include <memory>
#include <string>
#include <vector>

class SiteLogic;

class SiteLogicManager {
  private:
    std::vector<std::shared_ptr<SiteLogic> > sitelogics;
  public:
    SiteLogicManager();
    const std::shared_ptr<SiteLogic> getSiteLogic(const std::string &);
    const std::shared_ptr<SiteLogic> getSiteLogic(SiteLogic *);
    void deleteSiteLogic(const std::string &);
};
