#include "workmanager.h"

#include "eventreceiver.h"

namespace Core {

#define READY_SIZE 8
#define OVERLOAD_SIZE 32
#define ASYNC_WORKERS 2

WorkManager::WorkManager() : running(false), overloaded(false), lowpriooverloaded(false) {
  for (unsigned int i = 0; i < ASYNC_WORKERS; ++i) {
    asyncworkers.emplace_back(*this, asyncqueue);
  }
  eventqueues.push_back(&lowprioqueue);
  eventqueues.push_back(&dataqueue);
  eventqueues.push_back(&highprioqueue);
}

WorkManager::~WorkManager() {
  stop();
}

void WorkManager::init(const std::string& prefix, int id) {
  thread.start((prefix + "-work-" + std::to_string(id)).c_str(), this);
  std::list<AsyncWorker>::iterator it;
  std::lock_guard<std::mutex> lock(worklock);
  running = true;
  for (AsyncWorker& aw : asyncworkers) {
    aw.init(prefix, id);
  }
}

void WorkManager::dispatchFDData(EventReceiver* er, int sockid) {
  er->bindWorkManager(this);
  highprioqueue.push(Event(er, EventType::DATA, sockid));
  event.post();
  readdata.wait();
}

bool WorkManager::dispatchFDData(EventReceiver* er, int sockid, char * buf, int len, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::DATABUF, sockid, buf, len));
  event.post();
  if (prio == Prio::LOW) {
    return !lowPrioOverload();
  }
  return !overload();
}

void WorkManager::dispatchTick(EventReceiver* er, int interval) {
  er->bindWorkManager(this);
  highprioqueue.push(Event(er, EventType::TICK, interval));
  event.post();
}

bool WorkManager::dispatchEventNew(EventReceiver* er, int sockid, int newsockid, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::NEW, sockid, newsockid));
  event.post();
  if (prio == Prio::LOW) {
    return !lowPrioOverload();
  }
  return !overload();
}

void WorkManager::dispatchEventConnecting(EventReceiver* er, int sockid, const std::string & addr, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::CONNECTING, sockid, addr));
  event.post();
}

void WorkManager::dispatchEventConnected(EventReceiver* er, int sockid, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::CONNECTED, sockid));
  event.post();
}

void WorkManager::dispatchEventDisconnected(EventReceiver* er, int sockid, const DisconnectType& reason, const std::string& details, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::DISCONNECTED, sockid, details, static_cast<int>(reason)));
  event.post();
}

void WorkManager::dispatchEventSSLSuccess(EventReceiver* er, int sockid, const std::string & cipher, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::SSL_SUCCESS, sockid, cipher));
  event.post();
}

void WorkManager::dispatchEventFail(EventReceiver* er, int sockid, const std::string & error, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::FAIL, sockid, error));
  event.post();
}

void WorkManager::dispatchEventSendComplete(EventReceiver* er, int sockid, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::SEND_COMPLETE, sockid));
  event.post();
}

void WorkManager::dispatchSignal(EventReceiver* er, int signal, int value) {
  er->bindWorkManager(this);
  if (signalevents.set(er, signal, value)) {
    event.post();
  }
}

void WorkManager::dispatchAsyncTaskComplete(AsyncTask & task) {
  if (task.dataIsPointer()) {
    dataqueue.push(Event(task.getReceiver(), EventType::ASYNC_COMPLETE_P, task.getType(), task.getData()));
  }
  else {
    dataqueue.push(Event(task.getReceiver(), EventType::ASYNC_COMPLETE, task.getType(), task.getNumData()));
  }
  event.post();
}

void WorkManager::dispatchApplicationMessage(EventReceiver* er, int messageid, void* messagedata, Prio prio) {
  er->bindWorkManager(this);
  eventqueues[static_cast<int>(prio)]->push(Event(er, EventType::APPLICATION_MESSAGE, messageid, messagedata));
  event.post();
}

void WorkManager::deferDelete(const std::shared_ptr<EventReceiver>& er) {
  lowprioqueue.push(Event(er, EventType::DELETE));
  event.post();
}

void WorkManager::asyncTask(EventReceiver* er, int type, void (*taskfunction)(EventReceiver *, int), int data) {
  asyncqueue.push(AsyncTask(er, type, taskfunction, data));
}

void WorkManager::asyncTask(EventReceiver* er, int type, void (*taskfunction)(EventReceiver *, void *), void * data) {
  asyncqueue.push(AsyncTask(er, type, taskfunction, data));
}

DataBlockPool& WorkManager::getBlockPool() {
  return blockpool;
}

bool WorkManager::overload() {
  bool currentlyoverloaded = highprioqueue.size() + dataqueue.size() >= OVERLOAD_SIZE;
  if (currentlyoverloaded) {
    overloaded = true;
    lowpriooverloaded = true;
  }
  return overloaded;
}

bool WorkManager::lowPrioOverload() {
  bool currentlyoverloaded = highprioqueue.size() + dataqueue.size() + lowprioqueue.size() >= OVERLOAD_SIZE;
  if (currentlyoverloaded) {
    lowpriooverloaded = true;
  }
  return lowpriooverloaded;
}

unsigned int WorkManager::getQueueSize() const {
  return highprioqueue.size() + dataqueue.size() + lowprioqueue.size();
}

void WorkManager::addReadyNotify(EventReceiver* er) {
  std::lock_guard<std::mutex> lock(readylock);
  readynotify.push_back(er);
}

void WorkManager::flushEventReceiver(EventReceiver* er)
{
  worklock.lock();
  if (running) {
    worklock.unlock();
    if (thread.isCurrentThread()) {
      signalevents.flushEventReceiver(er);
      for (size_t i = 0; i < eventqueues.size(); ++i) {
        eventqueues[i]->lock();
        for (auto it = eventqueues[i]->begin(); it != eventqueues[i]->end();) {
          if (it->getReceiver() == er) {
            eventqueues[i]->erase(it);
          }
          else {
            ++it;
          }
        }
        eventqueues[i]->unlock();
      }
    }
    else {
      eventqueues[static_cast<int>(Prio::LOW)]->push(Event(nullptr, EventType::FLUSH, 0));
      event.post();
      flush.wait();
    }
    return;
  }
  worklock.unlock();
}

void WorkManager::run() {
  while (true) {
    event.wait();
    if (signalevents.hasEvent()) {
      SignalData signal = signalevents.getClearFirst();
      if (!signal.er) {
        std::lock_guard<std::mutex> lock(worklock);
        running = false;
        flush.post();
        return;
      }
      signal.er->signal(signal.signal, signal.value);
    }
    else {
      Event event;
      if (highprioqueue.size()) {
        event = highprioqueue.pop();
      }
      else if (dataqueue.size()) {
        event = dataqueue.pop();
      }
      else {
        event = lowprioqueue.pop();
      }
      EventReceiver* er = event.getReceiver();
      int numdata = event.getNumericalData();
      switch (event.getType()) {
        case EventType::DATA:
          er->FDData(numdata);
          readdata.post();
          break;
        case EventType::DATABUF: {
          char* data = static_cast<char*>(event.getData());
          er->FDData(numdata, data, event.getDataLen());
          blockpool.returnBlock(data);
          break;
        }
        case EventType::TICK:
          er->tick(numdata);
          break;
        case EventType::CONNECTING:
          er->FDConnecting(numdata, event.getStrData());
          break;
        case EventType::CONNECTED:
          er->FDConnected(numdata);
          break;
        case EventType::DISCONNECTED:
          er->FDDisconnected(numdata, static_cast<DisconnectType>(event.getNumericalData2()), event.getStrData());
          break;
        case EventType::SSL_SUCCESS:
          er->FDSSLSuccess(numdata, event.getStrData());
          break;
        case EventType::NEW:
          er->FDNew(numdata, event.getNumericalData2());
          break;
        case EventType::FAIL:
          er->FDFail(numdata, event.getStrData());
          break;
        case EventType::SEND_COMPLETE:
          er->FDSendComplete(numdata);
          break;
        case EventType::DELETE: // will be deleted when going out of scope
          break;
        case EventType::ASYNC_COMPLETE:
          er->asyncTaskComplete(numdata, event.getNumericalData2());
          break;
        case EventType::ASYNC_COMPLETE_P:
          er->asyncTaskComplete(numdata, event.getData());
          break;
        case EventType::APPLICATION_MESSAGE:
          er->receivedApplicationMessage(numdata, event.getData());
          break;
        case EventType::FLUSH:
          flush.post();
          break;
      }
      if ((overloaded || lowpriooverloaded) &&
          dataqueue.size() + highprioqueue.size() + lowprioqueue.size() <= READY_SIZE)
      {
        overloaded = false;
        lowpriooverloaded = false;
        std::lock_guard<std::mutex> lock(readylock);
        for (EventReceiver* ernotify : readynotify) {
          ernotify->workerReady();
        }
      }
    }
  }
}

void WorkManager::preStop() {
  std::lock_guard<std::mutex> lock(worklock);
  if (!running) {
    return;
  }
  signalevents.set(nullptr, 0, 0);
  event.post();
  for (unsigned int i = 0; i < asyncworkers.size(); ++i) {
    asyncqueue.push(AsyncTask(nullptr, 0, nullptr, 0));
  }
}

void WorkManager::stop() {
  preStop();
  thread.join();
  for (auto& aswk : asyncworkers) {
    aswk.join();
  }
  flush.post();
}

} // namespace Core
