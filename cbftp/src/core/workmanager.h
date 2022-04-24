#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "asynctask.h"
#include "asyncworker.h"
#include "blockingqueue.h"
#include "datablockpool.h"
#include "event.h"
#include "prio.h"
#include "semaphore.h"
#include "signalevents.h"
#include "threading.h"

namespace Core {

class EventReceiver;
enum class DisconnectType;

/* The WorkManager handles the main workload of an application.
 * events from IOManager (sockets / file descriptors), TickPoke (time-based events),
 * AsyncWorkers (asynchronous tasks), signal handlers etc will be
 * dispatched here and run by the internal Worker thread.
 * An application built around the WorkManager normally does not have to deal with
 * any kind of threading, locks or synchronization by itself.
 */
class WorkManager {
public:
    WorkManager();
    ~WorkManager();

    /* The WorkManager needs to be initialized by calling this function right before
     * the application enters its main loop.
     * @param prefix: prefix name of the thread for debugging purposes
     * @param id: id number of the thread for debugging purposes
     */
    void init(const std::string& prefix, int id = 0);

    /* Begin stopping threads */
    void preStop();

    /* Stop the Worker and AsyncWorker threads and wait for join */
    void stop();

    /* Schedule an asynchronous task that runs on a secondary thread.
     * @param er: The calling EventReceiver where the callback will end up
     * @param type: The type of callback (for book keeping on the callee side)
     * @param taskFunction: the callback function with an int as argument
     * @param taskFunctionP: alternative callback function with a void* as argument
     * @param data: the data field (int or void* depending on callback type)
     *
     * When the task has finished, er->asyncTaskComplete() will be called.
     */
    void asyncTask(EventReceiver* er, int type, void (*taskFunction)(EventReceiver*, int), int data);
    void asyncTask(EventReceiver* er, int type, void (*taskFunctionP)(EventReceiver*, void*), void* data);

    /* Call this to ensure a delayed delete of an EventReceiver until all events
     * intended for that receiver are flushed from the event queue.
     */
    void deferDelete(const std::shared_ptr<EventReceiver>& er);

    /* Flush an EventReceiver from the work queues. */
    void flushEventReceiver(EventReceiver* er);

    DataBlockPool& getBlockPool();

    /*
     * Does the worker consider itself overloaded (i.e. is the work queue too large)?
     */
    bool overload();

    /*
     * Does the worker consider itself overloaded in the low-priority queue?
     */
    bool lowPrioOverload();

    /*
     * Total current size of the event queues
     */
    unsigned int getQueueSize() const;

    /* Call from signal handlers to schedule related work on the worker thread */
    void dispatchSignal(EventReceiver* er, int signal, int value);

    /* These are normally called from the IOManager only */
    void dispatchFDData(EventReceiver* er, int sockid);
    bool
    dispatchFDData(EventReceiver* er, int sockid, char* buf, int len, Prio prio = Prio::NORMAL);
    bool dispatchEventNew(EventReceiver* er, int sockid, int newsockid, Prio prio = Prio::NORMAL);
    void dispatchEventConnecting(EventReceiver* er,
                                 int sockid,
                                 const std::string& addr,
                                 Prio prio = Prio::NORMAL);
    void dispatchEventConnected(EventReceiver* er, int sockid, Prio prio = Prio::NORMAL);
    void dispatchEventDisconnected(EventReceiver* er,
                                   int sockid,
                                   const DisconnectType& reason,
                                   const std::string& details,
                                   Prio prio = Prio::NORMAL);
    void dispatchEventSSLSuccess(EventReceiver* er,
                                 int sockid,
                                 const std::string& cipher,
                                 Prio prio = Prio::NORMAL);
    void dispatchEventFail(EventReceiver* er,
                           int sockid,
                           const std::string& error,
                           Prio prio = Prio::NORMAL);
    void dispatchEventSendComplete(EventReceiver* er, int sockid, Prio prio = Prio::NORMAL);
    void addReadyNotify(EventReceiver* er);

    /* Called from TickPoke */
    void dispatchTick(EventReceiver* er, int interval);

    /* Called from AsyncWorker */
    void dispatchAsyncTaskComplete(AsyncTask& task);

    void dispatchApplicationMessage(EventReceiver* er,
                                    int messageId,
                                    void* messageData = nullptr,
                                    Prio prio = Prio::NORMAL);

    void run();
private:
  Thread<WorkManager> thread;
  std::list<AsyncWorker> asyncworkers;
  BlockingQueue<Event> dataqueue;
  BlockingQueue<Event> highprioqueue;
  BlockingQueue<Event> lowprioqueue;
  BlockingQueue<AsyncTask> asyncqueue;
  std::mutex readylock;
  std::mutex worklock;
  std::list<EventReceiver*> readynotify;
  SignalEvents signalevents;
  Semaphore event;
  Semaphore readdata;
  Semaphore flush;
  DataBlockPool blockpool;
  bool running;
  bool overloaded;
  bool lowpriooverloaded;
  std::vector<BlockingQueue<Event>*> eventqueues;
};

} // namespace Core
