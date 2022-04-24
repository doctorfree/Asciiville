#pragma once

#include <string>
#include <utility>

class RawBufferCallback {
public:
  virtual ~RawBufferCallback() {}
  virtual void newRawBufferLine(const std::pair<std::string, std::string> &) = 0;
};
