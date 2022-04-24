#pragma once

#include "core/eventreceiver.h"

class TimeReference : public Core::EventReceiver {
public:
  TimeReference();
  void tick(int);
  unsigned long long timeReference() const;
  unsigned long long timePassedSince(unsigned long long) const;
private:
  unsigned long long timeticker;

public:
  static void updateTime();
  static int currentYear();
  static int currentMonth();
  static int currentDay();
};
