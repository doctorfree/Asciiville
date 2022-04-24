#include "eventlog.h"

#include "rawbuffer.h"

EventLog::EventLog() {
  rawbuf = new RawBuffer();
  latestid = 0;
}

EventLog::~EventLog() {
  delete rawbuf;
}

RawBuffer* EventLog::getRawBuffer() const {
  return rawbuf;
}

void EventLog::log(const std::string& owner, const std::string& text, Core::LogLevel level) {
  std::string line = "<" + owner + "> " + text;
  latest = line;
  latestid++;
  rawbuf->writeLine("<" + owner + "> " + text);
}

std::string EventLog::getLatest() const {
  return latest;
}

int EventLog::getLatestId() const {
  return latestid;
}
