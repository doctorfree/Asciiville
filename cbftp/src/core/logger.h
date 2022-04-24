#pragma once

#include <memory>
#include <string>

namespace Core {

enum class LogLevel {
  ERROR,
  WARNING,
  INFO,
  DEBUG
};

class Logger {
public:
  virtual ~Logger() { }
  virtual void log(const std::string& owner, const std::string& text, LogLevel level = LogLevel::INFO) = 0;
};

std::shared_ptr<Logger>& getLogger();
void setLogger(const std::shared_ptr<Logger>& logger);
std::string toString(LogLevel level);

} // namespace Core
