#include "timereference.h"

#include <ctime>

#include "core/tickpoke.h"
#include "globalcontext.h"

#define INTERVAL 50

static int currentyear = 0;
static int currentmonth = 0;
static int currentday = 0;

TimeReference::TimeReference() {
  global->getTickPoke()->startPoke(this, "TimeReference", INTERVAL, 0);
}

void TimeReference::tick(int) {
  timeticker += INTERVAL;
}

unsigned long long TimeReference::timeReference() const {
  return timeticker;
}

unsigned long long TimeReference::timePassedSince(unsigned long long timestamp) const {
  if (timestamp > timeticker) {
    return 0 - timestamp + timeticker;
  }
  return timeticker - timestamp;
}

void TimeReference::updateTime() {
  time_t rawtime;
  time(&rawtime);
  struct tm timedata;
  localtime_r(&rawtime, &timedata);
  currentyear = timedata.tm_year + 1900;
  currentmonth = timedata.tm_mon + 1;
  currentday = timedata.tm_mday;
}

int TimeReference::currentYear() {
  return currentyear;
}

int TimeReference::currentMonth() {
  return currentmonth;
}

int TimeReference::currentDay() {
  return currentday;
}
