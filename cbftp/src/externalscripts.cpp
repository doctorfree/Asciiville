#include "externalscripts.h"

#include <string>

#include "datafilehandler.h"
#include "eventlog.h"
#include "filesystem.h"
#include "globalcontext.h"
#include "rawbuffer.h"
#include "restapi.h"
#include "subprocessmanager.h"
#include "util.h"

ExternalScript::ExternalScript(const std::string& name, const Path& path, int id) :
  name(name), path(path), id(id)
{

}

RunningScript::RunningScript(int pid, const std::string& name, const std::string& token) : pid(pid), name(name), tempauthtoken(token) {

}

ExternalScripts::ExternalScripts(const std::string& name) : name(name) {
  Path scriptspath = DataFileHandler::getDataDir() / "scripts";
  if (FileSystem::directoryExists(scriptspath)) {
    if (!FileSystem::directoryExistsReadable(scriptspath)) {
      std::string errorstring = "Error: no read access to " + scriptspath.toString();
      perror(errorstring.c_str());
      exit(1);
    }
    if (!FileSystem::directoryExistsWritable(scriptspath)) {
      std::string errorstring = "Error: no write access to " + scriptspath.toString();
      perror(errorstring.c_str());
      exit(1);
    }
  }
  else {
    util::Result res = FileSystem::createDirectory(scriptspath, true);
    if (!res.success) {
      std::string errorstring = "Error: could not create " + scriptspath.toString() + ":" + res.error;
      perror(errorstring.c_str());
      exit(1);
    }
  }
}

ExternalScripts::~ExternalScripts() {

}

std::list<ExternalScript>::const_iterator ExternalScripts::begin() const {
  return scripts.begin();
}

std::list<ExternalScript>::const_iterator ExternalScripts::end() const {
  return scripts.end();
}

void ExternalScripts::addScript(const std::string& name, const Path& path, int id)
{
  if (id == -1) {
    id = nextAvailableId();
  }
  scripts.emplace_back(name, path, id);
}

int ExternalScripts::size() const {
  return scripts.size();
}

void ExternalScripts::clear() {
  scripts.clear();
}

std::string ExternalScripts::getName() const {
  return name;
}

bool ExternalScripts::hasScript(const std::string& scriptname) const {
  for (std::list<ExternalScript>::const_iterator it = scripts.begin(); it != scripts.end(); ++it) {
    if (it->name == scriptname) {
      return true;
    }
  }
  return false;
}

bool ExternalScripts::hasScript(int id) const {
  for (std::list<ExternalScript>::const_iterator it = scripts.begin(); it != scripts.end(); ++it) {
    if (it->id == id) {
      return true;
    }
  }
  return false;
}

std::string ExternalScripts::getScriptName(int id) const {
  for (std::list<ExternalScript>::const_iterator it = scripts.begin(); it != scripts.end(); ++it) {
    if (it->id == id) {
      return it->name;
    }
  }
  return "";
}

std::shared_ptr<RawBuffer> ExternalScripts::execute(int id, const std::vector<std::string>& args) {
  for (std::list<ExternalScript>::const_iterator it = scripts.begin(); it != scripts.end(); ++it) {
    if (it->id == id) {
      Path path = it->path;
      if (path.isRelative()) {
        path = DataFileHandler::getDataDir() / path;
      }
      std::string token = global->getRestApi()->createTemporaryAuthToken();
      std::vector<std::string> pargs = args;
      pargs.insert(pargs.begin(), token);
      std::shared_ptr<SubProcess> subprocess = global->getSubProcessManager()->runProcess(this, path, pargs);
      if (subprocess) {
        runningscripts.emplace_back(subprocess->pid, it->name, token);
        pargs[0] = "********";
        global->getEventLog()->log("ExternalScripts", "Running: " + path.toString() + " " + util::join(pargs, " "));
      }
      break;
    }
  }
  return std::shared_ptr<RawBuffer>();
}

void ExternalScripts::processExited(int pid, int status) {
  for (std::list<RunningScript>::const_iterator it = runningscripts.begin(); it != runningscripts.end(); ++it) {
    if (it->pid == pid) {
      global->getRestApi()->removeTemporaryAuthToken(it->tempauthtoken);
      runningscripts.erase(it);
      return;
    }
  }
}

void ExternalScripts::processStdOut(int pid, const std::string& text) {
  for (std::list<RunningScript>::const_iterator it = runningscripts.begin(); it != runningscripts.end(); ++it) {
    if (it->pid == pid) {
      global->getEventLog()->log("ExternalScripts", it->name + ": " + text);
      return;
    }
  }
}

void ExternalScripts::processStdErr(int pid, const std::string& text) {
  for (std::list<RunningScript>::const_iterator it = runningscripts.begin(); it != runningscripts.end(); ++it) {
    if (it->pid == pid) {
      global->getEventLog()->log("ExternalScripts", it->name + ": " + text);
      return;
    }
  }
}

int ExternalScripts::nextAvailableId() const {
  for (int testid = 0; testid < 1000; ++testid) {
    bool found = false;
    for (std::list<ExternalScript>::const_iterator it = scripts.begin(); it != scripts.end(); ++it) {
      if (it->id == testid) {
        found = true;
        break;
      }
    }
    if (!found) {
      return testid;
    }
  }
  return -1;
}
