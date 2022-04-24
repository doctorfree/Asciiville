#pragma once

#include <pthread.h>
#include <string>
#include <unistd.h>

namespace Core {
namespace Threading {
void setThreadName(pthread_t thread, const std::string& name);
void setCurrentThreadName(const std::string& name);
pthread_t createDetachedThread(void (*func)(void*), void* arg = nullptr);
void blockSignalsInternal();
} // namespace Threading

template <class T> class Thread {
public:
  void start(const std::string& name, T* instance) {
    this->instance = instance;
    pthread_create(&thread, nullptr, run, (void *) this);
    Threading::setThreadName(thread, name);
  }
  void join() {
    pthread_join(thread, nullptr);
  }
  bool isCurrentThread() {
    return pthread_self() == thread;
  }
private:
  static void* run(void* target) {
    Threading::blockSignalsInternal();
    ((Thread*) target)->instance->run();
    return nullptr;
  }
  T* instance;
  pthread_t thread;
};

} // namespace Core
