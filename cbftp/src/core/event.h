#pragma once

#include <memory>
#include <string>

namespace Core {

class EventReceiver;

enum class EventType {
  DATA,
  DATABUF,
  TICK,
  CONNECTING,
  CONNECTED,
  DISCONNECTED,
  SSL_SUCCESS,
  NEW,
  FAIL,
  SEND_COMPLETE,
  DELETE,
  ASYNC_COMPLETE,
  ASYNC_COMPLETE_P,
  APPLICATION_MESSAGE,
  FLUSH
};

class Event {
public:
  Event();
  Event(EventReceiver* er, EventType type, int numdata = 0, void* data = nullptr, int datalen = 0);
  Event(EventReceiver* er, EventType type, int numdata, int numdata2);
  Event(EventReceiver* er, EventType type, int numdata, const std::string& strdata, int numdata2 = 0);
  Event(const std::shared_ptr<EventReceiver>& er, EventType type);
  ~Event();
  EventReceiver* getReceiver() const;
  EventType getType() const;
  void* getData() const;
  int getDataLen() const;
  int getNumericalData() const;
  int getNumericalData2() const;
  std::string getStrData() const;
private:
  EventReceiver* receiver;
  std::shared_ptr<EventReceiver> preceiver;
  EventType type;
  void* data;
  int datalen;
  int numdata;
  int numdata2;
  std::string strdata;
};

} // namespace Core
