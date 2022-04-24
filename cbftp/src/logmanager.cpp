#include "logmanager.h"

LogManager::LogManager() : maxrawbuflines(4096) {

}

int LogManager::getMaxRawbufLines() const {
  return maxrawbuflines;
}

void LogManager::setMaxRawbufLines(int max) {
  maxrawbuflines = max;
}
