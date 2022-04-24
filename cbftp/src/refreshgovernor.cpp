#include "refreshgovernor.h"

#include "site.h"
#include "globalcontext.h"
#include "loadmonitor.h"

namespace {

unsigned int freqToInterval(float timespersecond) {
  return 1000 / timespersecond;
}

struct RefreshItem {
  RefreshItem(float timespersecond, bool immediate) : interval(freqToInterval(timespersecond)), immediate(immediate) {

  }
  unsigned int interval;
  bool immediate;
};

std::map<unsigned int, std::map<SitePriority, RefreshItem>> populateRefreshRateMap() {
  std::map<unsigned int, std::map<SitePriority, RefreshItem>> rates;
  rates[1].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(0.1, false)));
  rates[2].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(0.2, false)));
  rates[3].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(0.3, false)));
  rates[4].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(0.5, false)));
  rates[5].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(0.5, false)));
  rates[6].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(1, false)));
  rates[7].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(1, false)));
  rates[8].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(2, false)));
  rates[9].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(2, true)));
  rates[1].insert(std::make_pair(SitePriority::LOW, RefreshItem(0.2, false)));
  rates[2].insert(std::make_pair(SitePriority::LOW, RefreshItem(0.3, false)));
  rates[3].insert(std::make_pair(SitePriority::LOW, RefreshItem(0.5, false)));
  rates[4].insert(std::make_pair(SitePriority::LOW, RefreshItem(0.7, false)));
  rates[5].insert(std::make_pair(SitePriority::LOW, RefreshItem(1, false)));
  rates[6].insert(std::make_pair(SitePriority::LOW, RefreshItem(2, false)));
  rates[7].insert(std::make_pair(SitePriority::LOW, RefreshItem(3, false)));
  rates[8].insert(std::make_pair(SitePriority::LOW, RefreshItem(4, true)));
  rates[9].insert(std::make_pair(SitePriority::LOW, RefreshItem(5, true)));
  rates[1].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(0.5, false)));
  rates[2].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(0.7, false)));
  rates[3].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(1, false)));
  rates[4].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(2, false)));
  rates[5].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(3, false)));
  rates[6].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(4, false)));
  rates[7].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(6, true)));
  rates[8].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(8, true)));
  rates[9].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(10, true)));
  rates[1].insert(std::make_pair(SitePriority::HIGH, RefreshItem(0.5, false)));
  rates[2].insert(std::make_pair(SitePriority::HIGH, RefreshItem(1, false)));
  rates[3].insert(std::make_pair(SitePriority::HIGH, RefreshItem(2, false)));
  rates[4].insert(std::make_pair(SitePriority::HIGH, RefreshItem(3, false)));
  rates[5].insert(std::make_pair(SitePriority::HIGH, RefreshItem(5, false)));
  rates[6].insert(std::make_pair(SitePriority::HIGH, RefreshItem(8, true)));
  rates[7].insert(std::make_pair(SitePriority::HIGH, RefreshItem(10, true)));
  rates[8].insert(std::make_pair(SitePriority::HIGH, RefreshItem(12, true)));
  rates[9].insert(std::make_pair(SitePriority::HIGH, RefreshItem(15, true)));
  rates[1].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(1, false)));
  rates[2].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(2, false)));
  rates[3].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(3, false)));
  rates[4].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(5, true)));
  rates[5].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(8, true)));
  rates[6].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(10, true)));
  rates[7].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(12, true)));
  rates[8].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(15, true)));
  rates[9].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(20, true)));
  return rates;
}

std::map<unsigned int, std::map<SitePriority, RefreshItem>> rates = populateRefreshRateMap();

}

RefreshGovernor::RefreshGovernor(const std::shared_ptr<Site>& site) : site(site),
                                 interval(0), timepassed(0), immediaterefreshallowed(true)
{
  update();
  global->getLoadMonitor()->addListener(this);
}

RefreshGovernor::~RefreshGovernor() {
  global->getLoadMonitor()->removeListener(this);
}

bool RefreshGovernor::refreshAllowed() const {
  return timepassed >= interval;
}

bool RefreshGovernor::immediateRefreshAllowed() const {
  return immediaterefreshallowed;
}

void RefreshGovernor::timePassed(int timepassed) {
  this->timepassed += timepassed;
}

void RefreshGovernor::useRefresh() {
  if (timepassed >= interval) {
    timepassed %= interval;
  }
  else {
    timepassed = 0;
  }
}

void RefreshGovernor::update() {
  recommendedPerformanceLevelChanged(global->getLoadMonitor()->getCurrentRecommendedPerformanceLevel());
}

void RefreshGovernor::recommendedPerformanceLevelChanged(int newlevel) {
  if (site->getListCommand() == SITE_LIST_LIST) {
    interval = freqToInterval(1);
    immediaterefreshallowed = false;
    return;
  }
  switch (site->getRefreshRate()) {
    case RefreshRate::VERY_LOW: {
      const RefreshItem& item = rates.at(5).at(SitePriority::VERY_LOW);
      interval = item.interval;
      immediaterefreshallowed = item.immediate;
      break;
    }
    case RefreshRate::FIXED_LOW: {
      const RefreshItem& item = rates.at(6).at(SitePriority::LOW);
      interval = item.interval;
      immediaterefreshallowed = item.immediate;
      break;
    }
    case RefreshRate::FIXED_AVERAGE: {
      const RefreshItem& item = rates.at(7).at(SitePriority::NORMAL);
      interval = item.interval;
      immediaterefreshallowed = item.immediate;
      break;
    }
    case RefreshRate::FIXED_HIGH: {
      const RefreshItem& item = rates.at(7).at(SitePriority::HIGH);
      interval = item.interval;
      immediaterefreshallowed = item.immediate;
      break;
    }
    case RefreshRate::FIXED_VERY_HIGH: {
      const RefreshItem& item = rates.at(9).at(SitePriority::VERY_HIGH);
      interval = item.interval;
      immediaterefreshallowed = item.immediate;
      break;
    }
    case RefreshRate::AUTO:
    {
      SitePriority prio = site->getPriority();
      if (prio > SitePriority::NORMAL) {
        prio = SitePriority::NORMAL;
      }
      setDynamicInterval(newlevel, prio);
      break;
    }
    case RefreshRate::DYNAMIC_LOW:
      setDynamicInterval(newlevel, SitePriority::LOW);
      break;
    case RefreshRate::DYNAMIC_AVERAGE:
      setDynamicInterval(newlevel, SitePriority::NORMAL);
      break;
    case RefreshRate::DYNAMIC_HIGH:
      setDynamicInterval(newlevel, SitePriority::HIGH);
      break;
    case RefreshRate::DYNAMIC_VERY_HIGH:
      setDynamicInterval(newlevel, SitePriority::VERY_HIGH);
      break;
  }
}

void RefreshGovernor::setDynamicInterval(int newlevel, SitePriority priority) {
  const RefreshItem& item = rates.at(newlevel).at(priority);
  interval = item.interval;
  immediaterefreshallowed = item.immediate;
}
