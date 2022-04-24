#pragma once

#include <string>

#include "core/logger.h"

class RawBuffer;

class EventLog : public Core::Logger {
public:
  EventLog();
  ~EventLog();
  RawBuffer* getRawBuffer() const;
  int getLatestId() const;
  std::string getLatest() const;
  void log(const std::string& owner, const std::string& text, Core::LogLevel level = Core::LogLevel::INFO) override;
private:
  RawBuffer* rawbuf;
  std::string latest;
  int latestid;
};
