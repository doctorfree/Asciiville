#pragma once

#include <list>

#include "core/eventreceiver.h"

class LoadMonitorCallback;

class LoadMonitor : private Core::EventReceiver {
public:
  LoadMonitor();
  ~LoadMonitor();
  unsigned int getCurrentRecommendedPerformanceLevel() const;
  unsigned int getCurrentCpuUsageAll() const;
  unsigned int getCurrentCpuUsageWorker() const;
  unsigned int getCurrentWorkerQueueSize() const;
  unsigned int getCurrentFileListRefreshRate() const;
  void addListener(LoadMonitorCallback* cb);
  void removeListener(LoadMonitorCallback* cb);
  const std::list<unsigned int>& getCpuUsageAllHistory() const;
  const std::list<unsigned int>& getCpuUsageWorkerHistory() const;
  const std::list<unsigned int>& getWorkQueueSizeHistory() const;
  const std::list<unsigned int>& getPerformanceLevelHistory() const;
  const std::list<unsigned int>& getFileListRefreshRateHistory() const;
  std::list<unsigned int> getUnseenCpuUsageAllHistory() const;
  std::list<unsigned int> getUnseenCpuUsageWorkerHistory() const;
  std::list<unsigned int> getUnseenWorkQueueSizeHistory() const;
  std::list<unsigned int> getUnseenPerformanceLevelHistory() const;
  std::list<unsigned int> getUnseenFileListRefreshRateHistory() const;
  int getHistoryLengthSeconds() const;
private:
  void tick(int message) override;
  unsigned long long int lasttimeallms;
  unsigned long long int lasttimeworkerms;
  unsigned int perflevel;
  std::list<unsigned int> allhistory;
  std::list<unsigned int> workerhistory;
  std::list<unsigned int> workqueuesizehistory;
  std::list<unsigned int> perflevelhistory;
  std::list<unsigned int> filelistrefreshratehistory;
  mutable std::list<unsigned int> allhistoryunseen;
  mutable std::list<unsigned int> workerhistoryunseen;
  mutable std::list<unsigned int> workqueuesizehistoryunseen;
  mutable std::list<unsigned int> perflevelhistoryunseen;
  mutable std::list<unsigned int> filelistrefreshratehistoryunseen;
  unsigned int numcores;
  unsigned int throttletoppc;
  unsigned int throttlebottompc;
  std::list<LoadMonitorCallback*> listeners;
};
