#include "event.h"

#include "eventreceiver.h"

namespace Core {

Event::Event() {
}

Event::Event(EventReceiver* er, EventType type, int numdata, void* data, int datalen) :
  receiver(er),
  type(type),
  data(data),
  datalen(datalen),
  numdata(numdata),
  numdata2(0)
{
}

Event::Event(EventReceiver* er, EventType type, int numdata, int numdata2) :
  receiver(er),
  type(type),
  data(nullptr),
  datalen(0),
  numdata(numdata),
  numdata2(numdata2)
{
}

Event::Event(EventReceiver* er, EventType type, int numdata, const std::string& strdata, int numdata2) :
  receiver(er),
  type(type),
  data(nullptr),
  datalen(0),
  numdata(numdata),
  numdata2(numdata2),
  strdata(strdata)
{
}

Event::Event(const std::shared_ptr<EventReceiver>& er, EventType type) :
  receiver(nullptr),
  preceiver(er),
  type(type),
  data(nullptr),
  datalen(0),
  numdata(0),
  numdata2(0)
{
}

Event::~Event() {

}

EventReceiver* Event::getReceiver() const {
  return receiver;
}

EventType Event::getType() const {
  return type;
}

void* Event::getData() const {
  return data;
}

int Event::getDataLen() const {
  return datalen;
}

int Event::getNumericalData() const {
  return numdata;
}

int Event::getNumericalData2() const {
  return numdata2;
}

std::string Event::getStrData() const {
  return strdata;
}

} // namespace Core
