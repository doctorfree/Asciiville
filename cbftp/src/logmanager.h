#pragma once

class LogManager {
public:
  LogManager();
  int getMaxRawbufLines() const;
  void setMaxRawbufLines(int max);
private:
  int maxrawbuflines;
};
