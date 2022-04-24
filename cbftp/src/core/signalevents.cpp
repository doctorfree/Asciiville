#include "signalevents.h"

#include <cassert>

namespace Core {

SignalEvents::SignalEvents() :
  hasevent(0)
{
  slots.resize(10);
}

bool SignalEvents::set(EventReceiver * er, int signal, int value) {
  std::lock_guard<std::recursive_mutex> lock(signallock);
  for (size_t i = 0; i < slots.size(); ++i) {
    if (!slots[i].set) {
      slots[i].set = true;
      slots[i].er = er;
      slots[i].signal = signal;
      slots[i].value = value;
      hasevent++;
      return true;
    }
    else if (slots[i].er == er && slots[i].signal == signal && slots[i].value == value) {
      return false;
    }
  }
  assert(false);
  return false;
}

bool SignalEvents::hasEvent() const {
  std::lock_guard<std::recursive_mutex> lock(signallock);
  return hasevent;
}

SignalData SignalEvents::getClearFirst() {
  std::lock_guard<std::recursive_mutex> lock(signallock);
  SignalData ret;
  for (size_t i = 0; i < slots.size(); ++i) {
    if (slots[i].set) {
      ret = slots[i];
      slots[i].set = false;
      hasevent--;
      return ret;
    }
  }
  assert(false);
  return ret;
}

void SignalEvents::flushEventReceiver(EventReceiver* er) {
  std::lock_guard<std::recursive_mutex> lock(signallock);
  for (size_t i = 0; i < slots.size(); ++i) {
    if (slots[i].set && slots[i].er == er) {
      slots[i].set = false;
      hasevent--;
    }
  }
}

}
