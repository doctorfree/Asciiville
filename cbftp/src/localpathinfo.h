#pragma once

class LocalPathInfo {
public:
  LocalPathInfo(int dirs, int files, int depth, unsigned long long int size);
  int getNumDirs() const;
  int getNumFiles() const;
  int getDepth() const;
  unsigned long long int getSize() const;
private:
  int dirs;
  int files;
  int depth;
  unsigned long long int size;
};
