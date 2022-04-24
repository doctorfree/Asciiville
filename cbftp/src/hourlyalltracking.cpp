#include "hourlyalltracking.h"

#include <ctime>

HourlyAllTracking::HourlyAllTracking() : hours(24), last24hours(0), all(0), minutessincelastrotation(0) {

}

void HourlyAllTracking::add(unsigned long long int value) {
  hours[0] += value;
  last24hours += value;
  all += value;
}

void HourlyAllTracking::tickMinute() {
  ++minutessincelastrotation;
  checkRotate();
}

void HourlyAllTracking::checkRotate() {
  if (minutessincelastrotation >= 24 * 60) {
    resetHours();
  }
  while (minutessincelastrotation >= 60) {
    minutessincelastrotation -= 60;
    last24hours -= hours[23];
    for (int i = 23; i > 0; --i) {
      hours[i] = hours[i - 1];
    }
    hours[0] = 0;
  }
}

unsigned long long int HourlyAllTracking::getLast24Hours() const {
  return last24hours;
}

unsigned long long int HourlyAllTracking::getAll() const {
  return all;
}

unsigned int HourlyAllTracking::getStartTimestamp() const {
  unsigned int currtime = static_cast<unsigned int>(time(nullptr));
  return currtime - (minutessincelastrotation * 60);
}

void HourlyAllTracking::setAll(unsigned long long int count) {
  all = count;
}

std::vector<unsigned long long int> HourlyAllTracking::getHours() const {
  return hours;
}

void HourlyAllTracking::setHours(const std::vector<unsigned long long int>& hours, unsigned int timestamp) {
  resetHours();
  this->hours = hours;
  for (unsigned long long int value : hours) {
    last24hours += value;
  }
  unsigned int currtime = static_cast<unsigned int>(time(nullptr));
  if (currtime <= timestamp) {
    return;
  }
  minutessincelastrotation = (currtime - timestamp) / 60;
  checkRotate();
}

void HourlyAllTracking::reset() {
  resetHours();
  all = 0;
}

void HourlyAllTracking::resetHours() {
  hours.clear();
  hours.resize(24);
  last24hours = 0;
  minutessincelastrotation = 0;
}
