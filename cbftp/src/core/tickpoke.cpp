#include "tickpoke.h"

#include <chrono>
#include <cstdlib>
#include <unistd.h>

#include "workmanager.h"
#include "tickpoketarget.h"

namespace Core {

namespace {

#define DEFAULT_SLEEP_INTERVAL_MS 50
#define ADJUSTMENT_INTERVAL_MS 200
#define SLEEP_TIME_MIN_ADJUSTMENT_USECS 25

long long int epochMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

} // namespace

TickPoke::TickPoke(WorkManager& wm) : wm(wm), forever(true),
    targetsleeptimems(DEFAULT_SLEEP_INTERVAL_MS), actualsleeptimeusecs(targetsleeptimems * 1000),
    basetimems(0), lasttimediffms(0), timepassedms(0)
{
}

TickPoke::~TickPoke() {
  breakLoop();
}

void TickPoke::tickerLoop() {
  std::lock_guard<std::recursive_mutex> lock(looplock);
  basetimems = epochMs();
  while (forever) {
    tickIntern();
    usleep(actualsleeptimeusecs);
  }
}

void TickPoke::tickIntern(bool adjust)
{
  long long int lasttimepassedms = timepassedms;
  timepassedms += targetsleeptimems;
  wm.dispatchTick(this, targetsleeptimems);
  if (adjust && lasttimepassedms % ADJUSTMENT_INTERVAL_MS + targetsleeptimems >= ADJUSTMENT_INTERVAL_MS) {
    long long int actualtimepassedms = epochMs();
    long long int timediffms = actualtimepassedms - basetimems - timepassedms;
    if ((lasttimediffms > 0 && timediffms > 0 && lasttimediffms > timediffms) ||
        (lasttimediffms < 0 && timediffms < 0 && lasttimediffms < timediffms))
    {
      // the tick skew is already being corrected in the right direction, no need to adjust further
      lasttimediffms = timediffms;
      return;
    }
    lasttimediffms = timediffms;
    if (timediffms > 0) { // too much real time has passed
      if (actualsleeptimeusecs == SLEEP_TIME_MIN_ADJUSTMENT_USECS) {
        // at minimum sleep already, cannot lower further
        while (timepassedms < actualtimepassedms) {
          tickIntern(false);
        }
        return;
      }
      actualsleeptimeusecs -= SLEEP_TIME_MIN_ADJUSTMENT_USECS;
    }
    else if (timediffms < 0) { // too little real time has passed
      actualsleeptimeusecs += SLEEP_TIME_MIN_ADJUSTMENT_USECS;
    }
  }
}

void TickPoke::tickerThread(const std::string& prefix, int id) {
  thread.start((prefix + "-tp-" + std::to_string(id)).c_str(), this);
}

void TickPoke::stop() {
  breakLoop();
  thread.join();
}

void TickPoke::run() {
  tickerLoop();
}

void TickPoke::breakLoop() {
  forever = false;
  std::lock_guard<std::recursive_mutex> lock(looplock); // grab lock to make sure the loop has stopped running
}

void TickPoke::tick(int interval) {
  if (!forever)
  {
    return;
  }
  std::list<std::pair<EventReceiver*, int>> ticklist;
  for (auto& target : targets) {
    EventReceiver* er = target.getPokee();
    int ticks = target.tick(interval);
    for (int i = 0; i < ticks; ++i) {
      ticklist.emplace_back(er, target.getMessage());
    }
  }
  for (auto& tickitem : ticklist) {
    tickitem.first->tick(tickitem.second);
  }
}

void TickPoke::startPoke(EventReceiver* pokee, const std::string& desc, int interval, int message) {
  targets.emplace_back(pokee, interval, message, desc);
}

void TickPoke::stopPoke(const EventReceiver* pokee, int message) {
  std::list<TickPokeTarget>::iterator it;
  for (it = targets.begin(); it != targets.end(); ++it) {
    if (it->getPokee() == pokee && it->getMessage() == message) {
      targets.erase(it);
      return;
    }
  }
}

void TickPoke::setGranularity(unsigned int interval) {
    targetsleeptimems = interval;
    actualsleeptimeusecs = targetsleeptimems * 1000;
}

} // namespace Core
