#include "datablock.h"

namespace Core {

DataBlock::DataBlock(char* datap, int datalen) :
  datap(datap),
  datalen(datalen),
  offset(0) {
}

char* DataBlock::rawData() const {
  return datap;
}

char* DataBlock::data() const {
  return datap + offset;
}

int DataBlock::rawDataLength() const {
  return datalen;
}

int DataBlock::dataLength() const {
  return datalen - offset;
}

void DataBlock::consume(int consumed) {
  offset += consumed;
}

} // namespace Core
