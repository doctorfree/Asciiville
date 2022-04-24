#include "threading.h"

#include <cassert>

#include "signal.h"

namespace Core {
namespace Threading {

namespace {

struct ThreadFunc {
  void (*func)(void*);
  void* arg;
};

void* runDetached(void* vtf) {
  blockAllRegisteredSignals();
  ThreadFunc* tf = reinterpret_cast<ThreadFunc*>(vtf);
  tf->func(tf->arg);
  delete tf;
  return nullptr;
}
} // namespace

void setThreadName(pthread_t thread, const std::string& name) {
  assert(name.length() <= 15);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, name.c_str());
#endif
}

void setCurrentThreadName(const std::string& name) {
  assert(name.length() <= 15);
  setThreadName(pthread_self(), name);
}

pthread_t createDetachedThread(void (*func)(void*), void* arg) {
  pthread_t thread;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  ThreadFunc* tf = new ThreadFunc();
  tf->func = func;
  tf->arg = arg;
  pthread_create(&thread, &attr, runDetached, static_cast<void*>(tf));
  return thread;
}

void blockSignalsInternal() {
  blockAllRegisteredSignals();
}

} // namespace Threading
} // namespace Core
