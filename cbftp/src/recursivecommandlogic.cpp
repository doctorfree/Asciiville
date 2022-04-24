#include "recursivecommandlogic.h"

#include <algorithm>
#include <unordered_map>

#include "filelist.h"
#include "file.h"
#include "util.h"

bool lengthSort(const std::pair<Path, bool>& a, const std::pair<Path, bool>& b) {
  if (a.first.toString().length() < b.first.toString().length()) {
    return true;
  }
  return false;
}

RecursiveCommandLogic::RecursiveCommandLogic() {
  active = false;
}

RecursiveCommandLogic::~RecursiveCommandLogic() {

}

void RecursiveCommandLogic::initialize(RecursiveCommandType mode, const Path& target, const std::string& user) {
  this->mode = mode;
  active = true;
  listtarget = false;
  this->basepath = target.dirName();
  this->target = target;
  this->user = user;
  wantedlists.clear();
  deletefiles.clear();
  wantedlists.push_back(target);
}

bool RecursiveCommandLogic::isActive() const {
  return active;
}

int RecursiveCommandLogic::getAction(const Path& currentpath, Path& actiontarget) {
  if (listtarget) {
    if (currentpath == targetpath) {
      listtarget = false;
      return RCL_ACTION_LIST;
    }
    else {
      actiontarget = targetpath;
      return RCL_ACTION_CWD;
    }
  }
  if (!wantedlists.size()) {
    if (!deletefiles.size()) {
      active = false;
      return RCL_ACTION_NOOP;
    }
    std::pair<Path, bool> delfile = deletefiles.back();
    if (currentpath != delfile.first.dirName()) {
      targetpath = actiontarget = delfile.first.dirName();
      return RCL_ACTION_CWD;
    }
    actiontarget = delfile.first.baseName();
    deletefiles.pop_back();
    if (!deletefiles.size()) {
      active = false;
    }
    if (delfile.second) {
      return RCL_ACTION_DELDIR;
    }
    return RCL_ACTION_DELETE;
  }
  else {
    listtarget = true;
    targetpath = actiontarget = wantedlists.front();
    wantedlists.pop_front();
    if (currentpath == targetpath) {
      listtarget = false;
      return RCL_ACTION_LIST;
    }
    return RCL_ACTION_CWD;
  }
}

void RecursiveCommandLogic::addFileList(const std::shared_ptr<FileList>& fl) {
  const Path & path = fl->getPath();
  if (!path.contains(target)) {
    return;
  }
  deletefiles.push_back(std::pair<Path, bool>(path, true));
  for (std::list<File*>::iterator it = fl->begin(); it != fl->end(); it++) {
    if ((*it)->isDirectory()) {
      wantedlists.push_back(path / (*it)->getName());
    }
    else {
      if (mode == RCL_DELETEOWN && !util::eightCharUserCompare((*it)->getOwner(), user)) {
        continue;
      }
      deletefiles.push_back(std::pair<Path, bool>(path / (*it)->getName(), false));
    }
  }
  if (!wantedlists.size()) {
    std::sort(deletefiles.begin(), deletefiles.end(), lengthSort);
  }
}

void RecursiveCommandLogic::failedCwd() {
  listtarget = false;
  if (targetpath == basepath) {
    active = false;
  }
  else {
    deletefiles.push_back(std::pair<Path, bool>(targetpath, true));
  }
  if (!wantedlists.size()) {
    std::sort(deletefiles.begin(), deletefiles.end(), lengthSort);
  }
}
