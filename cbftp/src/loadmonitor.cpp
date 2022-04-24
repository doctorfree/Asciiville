#include "loadmonitor.h"

#include <ctime>
#include <thread>

#include "core/tickpoke.h"
#include "core/workmanager.h"

#include "globalcontext.h"
#include "loadmonitorcallback.h"
#include "statistics.h"

namespace {

const unsigned int LOAD_MONITOR_CHECK_INTERVAL_MS = 250;
const unsigned int HISTORY_LENGTH_SECONDS = 60;
const unsigned int PERFLEVEL_MIN = 1;
const unsigned int PERFLEVEL_MAX = 9;
const unsigned int PERFLEVEL_THROTTLE_THRESHOLD_PC = 75;
const unsigned int PERFLEVEL_THROTTLE_THRESHOLD_TOLERANCE = 10;
const unsigned int HISTORY_SLOTS = HISTORY_LENGTH_SECONDS * (1000 / LOAD_MONITOR_CHECK_INTERVAL_MS);

}

LoadMonitor::LoadMonitor() : lasttimeallms(0), lasttimeworkerms(0), perflevel(PERFLEVEL_MAX),
  allhistory(HISTORY_SLOTS, 0),
  workerhistory(HISTORY_SLOTS, 0),
  workqueuesizehistory(HISTORY_SLOTS, 0),
  perflevelhistory(HISTORY_SLOTS, PERFLEVEL_MAX),
  filelistrefreshratehistory(HISTORY_SLOTS, 0),
  numcores(std::thread::hardware_concurrency()),
  throttletoppc(PERFLEVEL_THROTTLE_THRESHOLD_PC + PERFLEVEL_THROTTLE_THRESHOLD_TOLERANCE),
  throttlebottompc(PERFLEVEL_THROTTLE_THRESHOLD_PC - PERFLEVEL_THROTTLE_THRESHOLD_TOLERANCE)
{
  global->getTickPoke()->startPoke(this, "LoadMonitor", LOAD_MONITOR_CHECK_INTERVAL_MS, 0);
}

LoadMonitor::~LoadMonitor() {
  global->getTickPoke()->stopPoke(this, 0);
}

void LoadMonitor::tick(int message) {
  struct timespec tpall;
  struct timespec tpworker;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tpall);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tpworker);
  unsigned long long int timeallms = tpall.tv_sec * 1000 + tpall.tv_nsec / 1000000;
  unsigned long long int timeworkerms = tpworker.tv_sec * 1000 + tpworker.tv_nsec / 1000000;
  unsigned long long int currtimeallms = timeallms - lasttimeallms;
  unsigned long long int currtimeworkerms = timeworkerms - lasttimeworkerms;
  unsigned int currallpc = currtimeallms * 100 / numcores / LOAD_MONITOR_CHECK_INTERVAL_MS;
  unsigned int currworkerpc = currtimeworkerms * 100 / LOAD_MONITOR_CHECK_INTERVAL_MS;
  unsigned int workqueuesize = global->getWorkManager()->getQueueSize();
  unsigned int filelistrefreshrate = global->getStatistics()->getFileListRefreshRate();

  allhistory.push_back(currallpc);
  allhistory.pop_front();
  allhistoryunseen.push_back(currallpc);
  if (allhistoryunseen.size() > HISTORY_SLOTS) {
    allhistoryunseen.pop_front();
  }
  workerhistory.push_back(currworkerpc);
  workerhistory.pop_front();
  workerhistoryunseen.push_back(currworkerpc);
  if (workerhistoryunseen.size() > HISTORY_SLOTS) {
    workerhistoryunseen.pop_front();
  }
  workqueuesizehistory.push_back(workqueuesize);
  workqueuesizehistory.pop_front();
  workqueuesizehistoryunseen.push_back(workqueuesize);
  if (workqueuesizehistoryunseen.size() > HISTORY_SLOTS) {
    workqueuesizehistoryunseen.pop_front();
  }
  filelistrefreshratehistory.push_back(filelistrefreshrate);
  filelistrefreshratehistory.pop_front();
  filelistrefreshratehistoryunseen.push_back(filelistrefreshrate);
  if (filelistrefreshratehistoryunseen.size() > HISTORY_SLOTS) {
    filelistrefreshratehistoryunseen.pop_front();
  }
  bool perflevelchanged = false;
  if ((currallpc > throttletoppc || currworkerpc > throttletoppc) && perflevel > PERFLEVEL_MIN) {
    --perflevel;
    perflevelchanged = true;
  }
  else if ((currallpc < throttlebottompc || currworkerpc < throttlebottompc) && perflevel < PERFLEVEL_MAX) {
    ++perflevel;
    perflevelchanged = true;
  }
  perflevelhistory.push_back(perflevel);
  perflevelhistory.pop_front();
  perflevelhistoryunseen.push_back(perflevel);
  if (perflevelhistoryunseen.size() > HISTORY_SLOTS) {
    perflevelhistoryunseen.pop_front();
  }
  lasttimeallms = timeallms;
  lasttimeworkerms = timeworkerms;
  if (perflevelchanged) {
    for (LoadMonitorCallback* cb : listeners) {
      cb->recommendedPerformanceLevelChanged(perflevel);
    }
  }
}

unsigned int LoadMonitor::getCurrentRecommendedPerformanceLevel() const {
  return perflevel;
}

unsigned int LoadMonitor::getCurrentCpuUsageAll() const {
  return allhistory.back();
}

unsigned int LoadMonitor::getCurrentCpuUsageWorker() const {
  return workerhistory.back();
}

unsigned int LoadMonitor::getCurrentWorkerQueueSize() const {
  return workqueuesizehistory.back();
}

unsigned int LoadMonitor::getCurrentFileListRefreshRate() const {
  return filelistrefreshratehistory.back();
}

void LoadMonitor::addListener(LoadMonitorCallback* cb) {
  listeners.push_back(cb);
}

void LoadMonitor::removeListener(LoadMonitorCallback* cb) {
  listeners.remove(cb);
}

const std::list<unsigned int>& LoadMonitor::getCpuUsageAllHistory() const {
  allhistoryunseen.clear();
  return allhistory;
}

const std::list<unsigned int>& LoadMonitor::getCpuUsageWorkerHistory() const {
  workerhistoryunseen.clear();
  return workerhistory;
}

const std::list<unsigned int>& LoadMonitor::getWorkQueueSizeHistory() const {
  workqueuesizehistoryunseen.clear();
  return workqueuesizehistory;
}

const std::list<unsigned int>& LoadMonitor::getPerformanceLevelHistory() const {
  perflevelhistoryunseen.clear();
  return perflevelhistory;
}

const std::list<unsigned int>& LoadMonitor::getFileListRefreshRateHistory() const {
  filelistrefreshratehistoryunseen.clear();
  return filelistrefreshratehistory;
}

std::list<unsigned int> LoadMonitor::getUnseenCpuUsageAllHistory() const {
  std::list<unsigned int> out = allhistoryunseen;
  allhistoryunseen.clear();
  return out;
}

std::list<unsigned int> LoadMonitor::getUnseenCpuUsageWorkerHistory() const {
  std::list<unsigned int> out = workerhistoryunseen;
  workerhistoryunseen.clear();
  return out;
}

std::list<unsigned int> LoadMonitor::getUnseenWorkQueueSizeHistory() const {
  std::list<unsigned int> out = workqueuesizehistoryunseen;
  workqueuesizehistoryunseen.clear();
  return out;
}

std::list<unsigned int> LoadMonitor::getUnseenPerformanceLevelHistory() const {
  std::list<unsigned int> out = perflevelhistoryunseen;
  perflevelhistoryunseen.clear();
  return out;
}

std::list<unsigned int> LoadMonitor::getUnseenFileListRefreshRateHistory() const {
  std::list<unsigned int> out = filelistrefreshratehistoryunseen;
  filelistrefreshratehistoryunseen.clear();
  return out;
}

int LoadMonitor::getHistoryLengthSeconds() const {
  return HISTORY_LENGTH_SECONDS;
}
