#include "tickpoketarget.h"

namespace Core {

TickPokeTarget::TickPokeTarget(EventReceiver* pokee, int interval, int message, const std::string& desc) :
  pokee(pokee), interval(interval), currentval(0), message(message), desc(desc)
{
}

int TickPokeTarget::getMessage() const {
  return message;
}

EventReceiver* TickPokeTarget::getPokee() const {
  return pokee;
}

bool TickPokeTarget::tick(int msecs) {
  currentval += msecs;
  if (currentval >= interval) {
    currentval = currentval - interval;
    return true;
  }
  return false;
}

} // namespace Core
