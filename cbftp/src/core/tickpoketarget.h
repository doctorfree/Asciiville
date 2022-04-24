#pragma once

#include <string>

namespace Core {

class EventReceiver;

class TickPokeTarget {
public:
  TickPokeTarget(EventReceiver* pokee, int interval, int message, const std::string& desc);
  int getMessage() const;
  EventReceiver* getPokee() const;
  bool tick(int msecs);
private:
  EventReceiver* pokee;
  int interval;
  int currentval;
  int message;
  std::string desc;
};

} // namespace Core
