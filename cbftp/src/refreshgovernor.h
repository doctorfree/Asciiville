#pragma once

#include <map>
#include <memory>

#include "loadmonitorcallback.h"

class Site;
enum class SitePriority;

class RefreshGovernor : private LoadMonitorCallback {
public:
  RefreshGovernor(const std::shared_ptr<Site>& site);
  ~RefreshGovernor();
  bool refreshAllowed() const; // is it time to perform a refresh?
  bool immediateRefreshAllowed() const;
  void timePassed(int timepassed); // called continously by the sitelogic ticker
  void useRefresh(); // call when a refresh has been "taken" by a connection
  void update(); // call when site settings have changed
private:
  void recommendedPerformanceLevelChanged(int newlevel) override;
  void setDynamicInterval(int newlevel, SitePriority priority);
  std::shared_ptr<Site> site;
  int interval;
  int timepassed;
  bool immediaterefreshallowed;
};
