#include "localfilelist.h"

#include <utility>

#include "util.h"

LocalFileList::LocalFileList(const Path& path) :
  path(path), sizefiles(0)
{
}

void LocalFileList::updateFile(const LocalFile& file, int touch) {
  std::string name = file.getName();
  auto it = files.insert(std::pair<std::string, LocalFile>(name, file));
  if (!it.second) {
    if (it.first->second.getSize() < file.getSize()) {
      it.first->second.setSize(file.getSize());
    }
  }
  else {
    lowercasefilemap[util::toLower(name)] = name;
    if (!file.isDirectory()) {
      sizefiles++;
    }
  }
  it.first->second.setTouch(touch);
}

void LocalFileList::touchFile(const std::string& name, bool download) {
  LocalFile file(name, 512, false, "", "", 0, 0, 0, 0, 0, download);
  updateFile(file);
}

std::unordered_map<std::string, LocalFile>::const_iterator LocalFileList::begin() const {
  return files.begin();
}

std::unordered_map<std::string, LocalFile>::const_iterator LocalFileList::end() const {
  return files.end();
}

std::unordered_map<std::string, LocalFile>::const_iterator LocalFileList::find(const std::string& file) const {
  std::unordered_map<std::string, LocalFile>::const_iterator it = files.find(file);
  if (it == files.end()) {
    std::unordered_map<std::string, std::string>::const_iterator it2 = lowercasefilemap.find(util::toLower(file));
    if (it2 != lowercasefilemap.end()) {
      it = files.find(it2->second);
    }
  }
  return it;
}

const Path & LocalFileList::getPath() const {
  return path;
}

unsigned int LocalFileList::size() const {
  return files.size();
}

unsigned int LocalFileList::sizeFiles() const {
  return sizefiles;
}

void LocalFileList::cleanSweep(int touch) {
  std::list<std::string> eraselist;
  for (const std::pair<const std::string, LocalFile>& file : files) {
    if (file.second.getTouch() != touch && !file.second.isDownloading()) {
      eraselist.push_back(file.first);
    }
  }
  if (!eraselist.empty()) {
    for (const std::string& file : eraselist) {
      files.erase(file);
      lowercasefilemap.erase(util::toLower(file));
    }
    sizefiles = 0;
    for (const std::pair<const std::string, LocalFile>& file : files) {
      if (!file.second.isDirectory()) {
        ++sizefiles;
      }
    }
  }
}

std::unordered_map<std::string, LocalFile>::iterator LocalFileList::findIntern(const std::string& file) {
  std::unordered_map<std::string, LocalFile>::iterator it = files.find(file);
  if (it == files.end()) {
    std::unordered_map<std::string, std::string>::const_iterator it2 = lowercasefilemap.find(util::toLower(file));
    if (it2 != lowercasefilemap.end()) {
      it = files.find(it2->second);
    }
  }
  return it;
}

void LocalFileList::finishDownload(const std::string& file) {
  std::unordered_map<std::string, LocalFile>::iterator it = findIntern(file);
  if (it != files.end()) {
    it->second.finishDownload();
  }
}
