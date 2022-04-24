#pragma once

#include <vector>

class HourlyAllTracking {
public:
  HourlyAllTracking();
  void add(unsigned long long int);
  void tickMinute();
  unsigned long long int getLast24Hours() const;
  unsigned long long int getAll() const;
  unsigned int getStartTimestamp() const;
  void setAll(unsigned long long int count);
  std::vector<unsigned long long int> getHours() const;
  void setHours(const std::vector<unsigned long long int>& hours, unsigned int timestamp);
  void reset();
  void resetHours();
private:
  void checkRotate();
  std::vector<unsigned long long int> hours;
  unsigned long long int last24hours;
  unsigned long long int all;
  unsigned int minutessincelastrotation;
};
