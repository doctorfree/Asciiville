#include "statistics.h"

#include "core/tickpoke.h"
#include "globalcontext.h"

enum {
  TICK_MINUTE,
  TICK_SECOND
};

Statistics::Statistics() : spreadjobs(0), transferjobs(0), currentsecondrefreshtrack(0), lastsecondrefreshtrack(0)
{
  global->getTickPoke()->startPoke(this, "Statistics", 60 * 1000, TICK_MINUTE);
  global->getTickPoke()->startPoke(this, "Statistics", 1000, TICK_SECOND);
}

Statistics::~Statistics() {
  global->getTickPoke()->stopPoke(this, TICK_MINUTE);
  global->getTickPoke()->stopPoke(this, TICK_SECOND);
}

void Statistics::tick(int message) {
  if (message == TICK_MINUTE) {
    sizedown.tickMinute();
    filesdown.tickMinute();
    sizeup.tickMinute();
    filesup.tickMinute();
    sizefxp.tickMinute();
    filesfxp.tickMinute();
  }
  else if (message == TICK_SECOND) {
    lastsecondrefreshtrack = currentsecondrefreshtrack;
    currentsecondrefreshtrack = 0;
  }
}

void Statistics::addTransferStatsFile(StatsDirection direction, unsigned long long int size) {
  switch (direction) {
    case STATS_UP:
      sizeup.add(size);
      filesup.add(1);
      break;
    case STATS_DOWN:
      sizedown.add(size);
      filesdown.add(1);
      break;
    case STATS_FXP:
      sizefxp.add(size);
      filesfxp.add(1);
      break;
  }
}

void Statistics::addSpreadJob() {
  ++spreadjobs;
}

void Statistics::addTransferJob() {
  ++transferjobs;
}

void Statistics::addFileListRefresh() {
  ++currentsecondrefreshtrack;
}

const HourlyAllTracking& Statistics::getSizeDown() const {
  return sizedown;
}

const HourlyAllTracking& Statistics::getSizeUp() const {
  return sizeup;
}

const HourlyAllTracking& Statistics::getSizeFXP() const {
  return sizefxp;
}

const HourlyAllTracking& Statistics::getFilesDown() const {
  return filesdown;
}

const HourlyAllTracking& Statistics::getFilesUp() const {
  return filesup;
}

const HourlyAllTracking& Statistics::getFilesFXP() const {
  return filesfxp;
}

HourlyAllTracking& Statistics::getSizeDown() {
  return sizedown;
}

HourlyAllTracking& Statistics::getSizeUp() {
  return sizeup;
}

HourlyAllTracking& Statistics::getSizeFXP() {
  return sizefxp;
}

HourlyAllTracking& Statistics::getFilesDown() {
  return filesdown;
}

HourlyAllTracking& Statistics::getFilesUp() {
  return filesup;
}

HourlyAllTracking& Statistics::getFilesFXP() {
  return filesfxp;
}

unsigned int Statistics::getSpreadJobs() const {
  return spreadjobs;
}

unsigned int Statistics::getTransferJobs() const {
  return transferjobs;
}

unsigned int Statistics::getFileListRefreshRate() const {
  return lastsecondrefreshtrack;
}

void Statistics::setSpreadJobs(unsigned int jobs) {
  spreadjobs = jobs;
}

void Statistics::setTransferJobs(unsigned int jobs) {
  transferjobs = jobs;
}

void Statistics::resetHourlyStats() {
  filesdown.resetHours();
  filesup.resetHours();
  sizedown.resetHours();
  sizeup.resetHours();
  filesfxp.resetHours();
  sizefxp.resetHours();
}

void Statistics::resetAllStats() {
  filesdown.reset();
  filesup.reset();
  sizedown.reset();
  sizeup.reset();
  filesfxp.reset();
  sizefxp.reset();
  spreadjobs = 0;
  transferjobs = 0;
}
