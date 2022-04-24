#pragma once

#include <string>

#include "threading.h"
#include "blockingqueue.h"
#include "asynctask.h"

namespace Core {

class WorkManager;

class AsyncWorker {
public:
  AsyncWorker(WorkManager& wm, BlockingQueue<AsyncTask>& queue);
  void init(const std::string& prefix, int id);
  void run();
  void join();
private:
  Thread<AsyncWorker> thread;
  BlockingQueue<AsyncTask>& queue;
  WorkManager& wm;
};

} // namespace Core
