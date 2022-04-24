#include "eventreceiver.h"

#include "workmanager.h"

namespace Core {

EventReceiver::EventReceiver() : workmanager(nullptr) {

}

EventReceiver::~EventReceiver() {
  if (workmanager) {
    workmanager->flushEventReceiver(this);
  }
}

void EventReceiver::tick(int message) {

}

void EventReceiver::signal(int sig, int value) {

}

void EventReceiver::FDNew(int sockid, int newsockid) {

}

void EventReceiver::FDConnecting(int sockid, const std::string& addr) {

}

void EventReceiver::FDConnected(int sockid) {

}

void EventReceiver::FDData(int sockid) {

}

void EventReceiver::FDData(int sockid, char* data, unsigned int len) {

}

void EventReceiver::FDDisconnected(int sockid, DisconnectType reason, const std::string& details) {

}

void EventReceiver::FDFail(int sockid, const std::string& error) {

}

void EventReceiver::FDSSLSuccess(int sockid, const std::string& cipher) {

}

void EventReceiver::FDSendComplete(int sockid) {

}

void EventReceiver::asyncTaskComplete(int type, void* data) {

}

void EventReceiver::asyncTaskComplete(int type, int) {

}

void EventReceiver::receivedApplicationMessage(int messagetype, void* messagedata) {

}

void EventReceiver::workerReady() {

}

void EventReceiver::bindWorkManager(WorkManager* wm) {
  if (!workmanager) {
    workmanager = wm;
  }
}

} // namespace Core
