#pragma once

class LoadMonitorCallback {
public:
  virtual void recommendedPerformanceLevelChanged(int newlevel) = 0;
  virtual ~LoadMonitorCallback() {}
};
