#include "localpathinfo.h"

LocalPathInfo::LocalPathInfo(int dirs, int files, int depth, unsigned long long int size) : dirs(dirs), files(files), depth(depth), size(size) {

}

int LocalPathInfo::getNumDirs() const {
  return dirs;
}

int LocalPathInfo::getNumFiles() const {
  return files;
}

int LocalPathInfo::getDepth() const {
  return depth;
}

unsigned long long int LocalPathInfo::getSize() const {
  return size;
}
