#include "asyncworker.h"

#include "workmanager.h"

namespace Core {

AsyncWorker::AsyncWorker(WorkManager& wm, BlockingQueue<AsyncTask>& queue) :
  queue(queue), wm(wm)
{
}

void AsyncWorker::init(const std::string& prefix, int id) {
  thread.start((prefix + "-aswk-" + std::to_string(id)).c_str(), this);
}

void AsyncWorker::run() {
  while (true) {
    AsyncTask task = queue.pop();
    if (!task.execute()) {
      return;
    }
    wm.dispatchAsyncTaskComplete(task);
  }
}

void AsyncWorker::join() {
  thread.join();
}

} // namespace Core
