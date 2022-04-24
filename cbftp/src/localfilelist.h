#pragma once

#include <string>
#include <unordered_map>

#include "localfile.h"
#include "path.h"

class LocalFileList {
public:
  LocalFileList(const Path& path);
  void updateFile(const LocalFile& file, int touch = 0);
  void touchFile(const std::string& name, bool download = false);
  std::unordered_map<std::string, LocalFile>::const_iterator begin() const;
  std::unordered_map<std::string, LocalFile>::const_iterator end() const;
  std::unordered_map<std::string, LocalFile>::const_iterator find(const std::string& name) const;
  const Path & getPath() const;
  unsigned int size() const;
  unsigned int sizeFiles() const;
  void cleanSweep(int touch);
  void finishDownload(const std::string& file);
private:
  std::unordered_map<std::string, LocalFile>::iterator findIntern(const std::string& name);
  std::unordered_map<std::string, LocalFile> files;
  std::unordered_map<std::string, std::string> lowercasefilemap;
  Path path;
  int sizefiles;
};
