#pragma once

#include <list>
#include <mutex>

namespace Core {

class DataBlockPool {
public:
  DataBlockPool();
  ~DataBlockPool();
  char* getBlock();
  const int blockSize() const;
  void returnBlock(char* block);
private:
  void allocateNewBlocks();
  std::list<char*> blocks;
  std::list<char*> availableblocks;
  int totalblocks;
  std::mutex blocklock;
};

} // namespace Core
