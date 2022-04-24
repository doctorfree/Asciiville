#pragma once

#include <list>
#include <mutex>

#include "semaphore.h"

namespace Core {

template <class T> class BlockingQueue {
public:
  BlockingQueue() : queuesize(0) {}
  void push(T t) {
    std::lock_guard<std::mutex> lock(queuelock);
    queue.push_back(t);
    ++queuesize;
    content.post();
  }
  T pop() {
    content.wait();
    std::lock_guard<std::mutex> lock(queuelock);
    T ret = queue.front();
    queue.pop_front();
    --queuesize;
    return ret;
  }
  unsigned int size() const {
    std::lock_guard<std::mutex> lock(queuelock);
    return queuesize;
  }
  void lock() {
    queuelock.lock();
  }
  void unlock() {
    queuelock.unlock();
  }
  typename std::list<T>::iterator begin() {
    return queue.begin();
  }
  typename std::list<T>::iterator end() {
    return queue.end();
  }
  void erase(typename std::list<T>::iterator it) {
    queue.erase(it);
    --queuesize;
    content.wait();
  }
private:
  std::list<T> queue;
  unsigned int queuesize;
  mutable std::mutex queuelock;
  Semaphore content;
};

} // namespace Core
