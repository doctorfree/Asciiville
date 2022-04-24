#include "signal.h"

#include <cassert>
#include <map>
#include <pthread.h>

#include "threading.h"
#include "workmanager.h"

namespace Core {

namespace {

struct Sig {
  int sig;
  WorkManager* wm;
  EventReceiver* er;

};

std::map<int, Sig> registeredsignals;

} // namespace

void dispatchSignal(void* arg) {
  // this function runs in a new detached thread to avoid calling the
  // workmanager directly from the signal handler as mutexes may cause problems
  Sig* s = reinterpret_cast<Sig*>(arg);
  s->wm->dispatchSignal(s->er, s->sig, 0);
}



void signalHandlerDispatch(int sig) {
  std::map<int, Sig>::const_iterator it = registeredsignals.find(sig);
  assert(it != registeredsignals.end());
  Core::Threading::createDetachedThread(dispatchSignal, const_cast<void*>(static_cast<const void*>(&it->second)));
}

void registerSignalDispatch(int sig, WorkManager* wm, EventReceiver* er) {
  assert(registeredsignals.find(sig) == registeredsignals.end());
  Sig s;
  s.sig = sig;
  s.wm = wm;
  s.er = er;
  registeredsignals[sig] = s;
  registerSignalHandler(sig, signalHandlerDispatch);
}

void registerSignalHandler(int sig, void (*sighandler)(int)) {
  struct sigaction sa;
  sa.sa_handler = sighandler;
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);
  sigaction(sig, &sa, nullptr);
}

void blockAllRegisteredSignals() {
  sigset_t set;
  sigemptyset(&set);
  for (const std::pair<const int, Sig>& sig : registeredsignals) {
    sigaddset(&set, sig.first);
  }
  pthread_sigmask(SIG_BLOCK, &set, nullptr);
}

} // namespace Core
