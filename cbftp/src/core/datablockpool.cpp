#include "datablockpool.h"

#include <cstdlib>

namespace Core {

#define BLOCKSIZE 16384

DataBlockPool::DataBlockPool() : totalblocks(0) {
  std::lock_guard<std::mutex> lock(blocklock);
  allocateNewBlocks();
}

DataBlockPool::~DataBlockPool()
{
  std::lock_guard<std::mutex> lock(blocklock);
  for (char* block : blocks) {
    free(block);
  }
}

char* DataBlockPool::getBlock() {
  char* block;
  std::lock_guard<std::mutex> lock(blocklock);
  if (availableblocks.empty()) {
    allocateNewBlocks();
  }
  block = availableblocks.back();
  availableblocks.pop_back();
  return block;
}

const int DataBlockPool::blockSize() const {
  return BLOCKSIZE;
}

void DataBlockPool::returnBlock(char* block) {
  std::lock_guard<std::mutex> lock(blocklock);
  availableblocks.push_back(block);
}

void DataBlockPool::allocateNewBlocks() {
  for (int i = 0; i < 10; i++) {
    char* block = (char*) malloc(BLOCKSIZE);
    blocks.push_back(block);
    availableblocks.push_back(block);
    totalblocks++;
  }
}

} // namespace Core
