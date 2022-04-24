#include "sitelogicmanager.h"

#include "sitelogic.h"
#include "site.h"
#include "globalcontext.h"
#include "sitemanager.h"

SiteLogicManager::SiteLogicManager() {
}

const std::shared_ptr<SiteLogic> SiteLogicManager::getSiteLogic(const std::string & name) {
  std::vector<std::shared_ptr<SiteLogic> >::iterator it;
  for(it = sitelogics.begin(); it != sitelogics.end(); it++) {
    if ((*it)->getSite()->getName().compare(name) == 0) return *it;
  }
  if (!global->getSiteManager()->getSite(name)) {
    return std::shared_ptr<SiteLogic>();
  }
  std::shared_ptr<SiteLogic> x = std::make_shared<SiteLogic>(name);
  sitelogics.push_back(x);
  return x;
}

const std::shared_ptr<SiteLogic> SiteLogicManager::getSiteLogic(SiteLogic * sl) {
  std::vector<std::shared_ptr<SiteLogic> >::iterator it;
  for(it = sitelogics.begin(); it != sitelogics.end(); it++) {
    if ((*it).get() == sl) return *it;
  }
  return std::shared_ptr<SiteLogic>();
}

void SiteLogicManager::deleteSiteLogic(const std::string & name) {
  std::vector<std::shared_ptr<SiteLogic> >::iterator it;
  for(it = sitelogics.begin(); it != sitelogics.end(); it++) {
    if ((*it)->getSite()->getName().compare(name) == 0) {
      sitelogics.erase(it);
      return;
    }
  }
}
