#pragma once

#ifndef __APPLE__

#include <condition_variable>
#include <mutex>

namespace Core {

class Semaphore {
public:
  void post() {
    std::unique_lock<std::mutex> lock(mutex);
    ++count;
    condition.notify_one();
  }
  void wait() {
    std::unique_lock<std::mutex> lock(mutex);
    while (!count) { // Handle spurious wake-ups.
      condition.wait(lock);
    }
    --count;
  }
private:
  std::mutex mutex;
  std::condition_variable condition;
  unsigned long count = 0; // Initialized as locked.
};

#else

#include <dispatch/dispatch.h>

namespace Core {

class Semaphore {
public:
  Semaphore() :
    semaphore(dispatch_semaphore_create(0)) {
  }
  ~Semaphore() {
    dispatch_release(semaphore);
  }
  void post() {
    dispatch_semaphore_signal(semaphore);
  }
  void wait() {
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
  }
private:
  dispatch_semaphore_t semaphore;
};

#endif

} // namespace Core
