#pragma once

#include <string>

#include "core/types.h"
#include "path.h"
#include "util.h"

namespace FileSystem {

struct SpaceInfo {
  unsigned long long int size;
  unsigned long long int used;
  unsigned long long int avail;
};

bool directoryExists(const Path& path);
bool directoryExistsReadable(const Path& path);
bool directoryExistsWritable(const Path& path);
bool fileExists(const Path& path);
bool fileExistsReadable(const Path& path);
bool fileExistsWritable(const Path& path);
util::Result createDirectory(const Path& path, bool privatedir = false);
util::Result createDirectoryRecursive(const Path& path);
util::Result readFile(const Path& path, Core::BinaryData& rawdata);
util::Result writeFile(const Path& path, const Core::BinaryData& data);
util::Result move(const Path& src, const Path& dst);
SpaceInfo getSpaceInfo(const Path & path);
}

