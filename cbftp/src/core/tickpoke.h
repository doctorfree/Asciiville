#pragma once

#include <list>
#include <mutex>

#include "eventreceiver.h"
#include "threading.h"

namespace Core {

class TickPokeTarget;
class WorkManager;

/* TickPoke is used to schedule repeating time-based events without relying on
 * system calls such as timers etc. The granularity is 50 milliseconds by default.
 */
class TickPoke : private EventReceiver {
public:
  explicit TickPoke(WorkManager& wm);
  ~TickPoke();

  /* Schedule a repeating call to er->tick().
   * @param er: The receiving instance
   * @param desc: description of the receiving instance, class name or similar
   * @param interval: The call interval in milliseconds.
   * @param message: the integer provided in the tick(), also used as cancellation identifier
   */
  void startPoke(EventReceiver* er, const std::string& desc, int interval, int message);

  /* Cancel a scheduled ticker by providing its message as cancellation identifier */
  void stopPoke(const EventReceiver* er, int message);

  /* Call for a graceful application exit. */
  void breakLoop();

  /* A thread, preferably the main thread of the application, should rest in this
   * function.
   */
  void tickerLoop();

  /* Create a thread internally and enter tickerLoop with it.
   * @param prefix: prefix name of the thread for debugging purposes
   * @param id: id number of the thread for debugging purposes
   */
  void tickerThread(const std::string& prefix, int id = 0);

  /* Wait for the internal TickPoke thread to die, if there is one.
   */
  void stop();

  /* Sets the sleep time between ticks in milliseconds. */
  void setGranularity(unsigned int);

  /* Used internally through the WorkManager */
  void tick(int interval);

  /* Used internally by tickerThread */
  void run();

private:
  void tickIntern(bool adjust = true);
  Thread<TickPoke> thread;
  WorkManager& wm;
  std::list<TickPokeTarget> targets;
  bool forever;
  std::recursive_mutex looplock;
  unsigned int targetsleeptimems;
  unsigned int actualsleeptimeusecs;
  long long int basetimems;
  long long int lasttimediffms;
  long long int timepassedms;

};

} // namespace Core
