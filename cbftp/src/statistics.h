#pragma once

#include <vector>

#include "hourlyalltracking.h"
#include "core/eventreceiver.h"

enum StatsDirection {
  STATS_UP,
  STATS_DOWN,
  STATS_FXP
};

class Statistics : public Core::EventReceiver {
public:
  Statistics();
  ~Statistics();
  void tick(int);
  void addTransferStatsFile(StatsDirection, unsigned long long int size);
  void addSpreadJob();
  void addTransferJob();
  void addFileListRefresh();
  const HourlyAllTracking& getSizeDown() const;
  const HourlyAllTracking& getSizeUp() const;
  const HourlyAllTracking& getSizeFXP() const;
  const HourlyAllTracking& getFilesDown() const;
  const HourlyAllTracking& getFilesUp() const;
  const HourlyAllTracking& getFilesFXP() const;
  HourlyAllTracking& getSizeDown();
  HourlyAllTracking& getSizeUp();
  HourlyAllTracking& getSizeFXP();
  HourlyAllTracking& getFilesDown();
  HourlyAllTracking& getFilesUp();
  HourlyAllTracking& getFilesFXP();
  unsigned int getSpreadJobs() const;
  unsigned int getTransferJobs() const;
  unsigned int getFileListRefreshRate() const;
  void setSpreadJobs(unsigned int);
  void setTransferJobs(unsigned int);
  void resetHourlyStats();
  void resetAllStats();
private:
  HourlyAllTracking sizedown;
  HourlyAllTracking filesdown;
  HourlyAllTracking sizeup;
  HourlyAllTracking filesup;
  HourlyAllTracking sizefxp;
  HourlyAllTracking filesfxp;
  unsigned int spreadjobs;
  unsigned int transferjobs;
  unsigned int currentsecondrefreshtrack;
  unsigned int lastsecondrefreshtrack;
};
