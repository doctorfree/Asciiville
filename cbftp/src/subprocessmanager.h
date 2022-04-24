#pragma once

#include <string>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include "core/eventreceiver.h"
#include "core/datablockpool.h"

class Path;
class RawBuffer;

class SubProcessCallback {
public:
  virtual ~SubProcessCallback() {
  }
  virtual void processExited(int pid, int status);
  virtual void processStdOut(int pid, const std::string& text);
  virtual void processStdErr(int pid, const std::string& text);
};

struct SubProcess {
  int pid;
  std::shared_ptr<RawBuffer> rawbuf;
  SubProcessCallback* cb;
  int stdinfd;
  int stdoutfd;
  int stderrfd;
  int stdinhandle;
  int stdouthandle;
  int stderrhandle;

};

class SubProcessManager : public Core::EventReceiver {
public:
  SubProcessManager();
  std::shared_ptr<SubProcess> runProcess(SubProcessCallback* cb, const Path& path, const std::vector<std::string>& args);
  void killProcess(int pid);
  void killAll();
  bool getIsRunning(int pid) const;
  void signal(int signal, int);
private:
  void FDData(int sockid) override;
  std::list<std::shared_ptr<SubProcess>> subprocesses;
  Core::DataBlockPool blockpool;
};
