#pragma once

namespace Core {

class DataBlock {
public:
  DataBlock(char* datap, int datalen);
  char* rawData() const;
  int rawDataLength() const;
  char* data() const;
  int dataLength() const;
  void consume(int consumed);
private:
  char* datap;
  int datalen;
  int offset;
};

} // namespace Core
