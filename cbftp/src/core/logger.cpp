#include "logger.h"

#include <cassert>
#include <iostream>

namespace Core {

namespace {

std::shared_ptr<Logger> g_logger;

class CoutLogger : public Logger {
  void log(const std::string& owner, const std::string& text, LogLevel level) override {
    std::cout << toString(level) << "<" << owner << "> " << text << std::endl;
  }
};

} // namespace

std::shared_ptr<Logger>& getLogger() {
  if (!g_logger) {
    g_logger = std::make_shared<CoutLogger>();
    g_logger->log("Logger", "No custom logger set, using stdout.", LogLevel::INFO);
  }
  return g_logger;
}

void setLogger(const std::shared_ptr<Logger>& logger) {
  assert(!g_logger);
  g_logger = logger;
}

std::string toString(LogLevel level) {
  switch (level) {
    case LogLevel::ERROR:
      return "ERROR";
    case LogLevel::WARNING:
      return "WARNING";
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::DEBUG:
      return "DEBUG";
  }
  return "UNKNOWN";
}

} // namespace Core
