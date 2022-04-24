#include "filesystem.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <list>

#include "core/util.h"
#include "path.h"

#define READBLOCKSIZE 256

namespace FileSystem {

bool directoryExistsAccessible(const Path& path, bool write) {
  int how = write ? R_OK | W_OK : R_OK;
  if (access(path.toString().c_str(), how) == 0) {
    struct stat status;
    stat(path.toString().c_str(), &status);
    if (status.st_mode & S_IFDIR) {
      return true;
    }
  }
  return false;
}

bool directoryExistsWritable(const Path& path) {
  return directoryExistsAccessible(path, true);
}

bool directoryExistsReadable(const Path& path) {
  return directoryExistsAccessible(path, false);
}

bool fileExists(const Path& path) {
  return !access(path.toString().c_str(), F_OK);
}

bool directoryExists(const Path& path) {
  return !access(path.toString().c_str(), F_OK);
}

bool fileExistsReadable(const Path& path) {
  return !access(path.toString().c_str(), R_OK);
}

bool fileExistsWritable(const Path& path) {
  return !access(path.toString().c_str(), R_OK | W_OK);
}

util::Result createDirectory(const Path& path, bool privatedir) {
  mode_t mode = privatedir ? 0700 : 0755;
  if (mkdir(path.toString().c_str(), mode) != 0) {
    return util::Result(false, Core::util::getStrError(errno));
  }
  return true;
}

util::Result createDirectoryRecursive(const Path& path) {
  std::list<std::string> pathdirs = path.split();
  Path partialpath = path.isAbsolute() ? "/" : "";
  while (pathdirs.size() > 0) {
    partialpath = partialpath / pathdirs.front();
    pathdirs.pop_front();
    if (!directoryExists(partialpath)) {
      util::Result res = createDirectory(partialpath);
      if (!res.success) {
        return res;
      }
    }
  }
  return true;
}

util::Result readFile(const Path& path, Core::BinaryData& rawdata) {
  std::fstream infile;
  infile.open(path.toString().c_str(), std::ios::in | std::ios::binary);
  if (!infile) {
    return util::Result(false, Core::util::getStrError(errno));
  }
  int gcount = 0;
  std::vector<unsigned char *> rawdatablocks;
  while (!infile.eof() && infile.good()) {
    unsigned char * rawdatablock = new unsigned char[READBLOCKSIZE];
    rawdatablocks.push_back(rawdatablock);
    infile.read((char *)rawdatablock, READBLOCKSIZE);
    gcount = infile.gcount();
  }
  infile.close();
  size_t rawdatalen = ((rawdatablocks.size() - 1) * READBLOCKSIZE) + gcount;
  size_t rawdatasize = rawdatablocks.size() * READBLOCKSIZE;
  rawdata.resize(rawdatasize);
  std::vector<unsigned char *>::iterator it;
  int count = 0;
  for (it = rawdatablocks.begin(); it != rawdatablocks.end(); it++) {
    memcpy(&rawdata[0] + (count++ * READBLOCKSIZE), *it, READBLOCKSIZE);
    delete[] *it;
  }
  rawdata.resize(rawdatalen);
  return true;
}

util::Result writeFile(const Path& path, const Core::BinaryData& data) {
  std::ofstream outfile;
  outfile.open(path.toString().c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  if (!outfile) {
    return util::Result(false, Core::util::getStrError(errno));
  }
  outfile.write((const char *)&data[0], data.size());
  if (!outfile) {
    std::string error = Core::util::getStrError(errno);
    outfile.close();
    return util::Result(false, error);
  }
  outfile.close();
  if (!outfile) {
    return util::Result(false, Core::util::getStrError(errno));
  }
  return true;
}

util::Result move(const Path& src, const Path& dst) {
  if (rename(src.toString().c_str(), dst.toString().c_str()) != 0) {
    return util::Result(false, Core::util::getStrError(errno));
  }
  return true;
}

SpaceInfo getSpaceInfo(const Path & path) {
  struct statvfs stats;
  statvfs(path.toString().c_str(), &stats);
  SpaceInfo info;
  info.size = stats.f_bsize * stats.f_blocks;
  info.used = stats.f_bsize * (stats.f_blocks - stats.f_bfree);
  info.avail = stats.f_bsize * stats.f_bavail;
  return info;
}

}
