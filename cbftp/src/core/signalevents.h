#pragma once

#include <mutex>
#include <vector>

namespace Core {

class EventReceiver;

struct SignalData {
  SignalData() : set(false), signal(0), value(0), er(nullptr) { }
  bool set;
  int signal;
  int value;
  EventReceiver* er;
};

class SignalEvents {
public:
  SignalEvents();
  bool set(EventReceiver* er, int signal, int value);
  bool hasEvent() const;
  SignalData getClearFirst();
  void flushEventReceiver(EventReceiver* er);
private:
  int hasevent;
  std::vector<SignalData> slots;
  mutable std::recursive_mutex signallock;
  /* the lock needs to be recursive since the set method is supposed to be
   * called from signal handlers, which may interrupt current execution at
   * any time */
};

} //namespace Core
