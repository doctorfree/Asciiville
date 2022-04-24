#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "path.h"
#include "subprocessmanager.h"

class RawBuffer;

struct ExternalScript {
  ExternalScript(const std::string& name, const Path& path, int id);
  std::string name;
  Path path;
  int id;
};

struct RunningScript {
  RunningScript(int pid, const std::string& name, const std::string& token);
  int pid;
  std::string name;
  std::string tempauthtoken;
};

class ExternalScripts : public SubProcessCallback {
public:
  ExternalScripts(const std::string& name = "");
  virtual ~ExternalScripts();
  std::list<ExternalScript>::const_iterator begin() const;
  std::list<ExternalScript>::const_iterator end() const;
  std::string getName() const;
  bool hasScript(const std::string& scriptname) const;
  bool hasScript(int id) const;
  std::string getScriptName(int id) const;
  void addScript(const std::string& name, const Path& path, int id = -1);
  std::shared_ptr<RawBuffer> execute(int id, const std::vector<std::string>& args);
  int size() const;
  void clear();
private:
  int nextAvailableId() const;
  void processExited(int pid, int status) override;
  void processStdOut(int pid, const std::string& text) override;
  void processStdErr(int pid, const std::string& text) override;
  std::list<ExternalScript> scripts;
  std::string name;
  std::list<RunningScript> runningscripts;
};
