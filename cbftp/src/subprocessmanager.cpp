#include "subprocessmanager.h"

#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <cassert>
#include <csignal>
#include <vector>
#include <cctype>
#include <cstdlib>

#include "core/signal.h"
#include "core/iomanager.h"
#include "core/workmanager.h"
#include "core/util.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "path.h"
#include "util.h"

void SubProcessCallback::processExited(int pid, int status) {

}

void SubProcessCallback::processStdOut(int pid, const std::string& text) {

}

void SubProcessCallback::processStdErr(int pid, const std::string& text) {

}

SubProcessManager::SubProcessManager() {
  Core::registerSignalDispatch(SIGCHLD, global->getWorkManager(), this);
}

std::shared_ptr<SubProcess> SubProcessManager::runProcess(SubProcessCallback* cb, const Path& path, const std::vector<std::string>& args) {
  int stdinpipe[2];
  int stdoutpipe[2];
  int stderrpipe[2];
  pipe(stdinpipe);
  pipe(stdoutpipe);
  pipe(stderrpipe);
  int pid = fork();
  if (!pid) {
    const char* cargs[256];
    int i = 0;
    cargs[i++] = path.toString().c_str();
    for (const std::string& arg : args) {
      cargs[i++] = arg.c_str();
    }
    cargs[i] = nullptr;
    char* ccargs[256];
    memcpy(ccargs, cargs, sizeof(cargs));
    dup2(stdinpipe[0], STDIN_FILENO);
    dup2(stdoutpipe[1], STDOUT_FILENO);
    dup2(stderrpipe[1], STDERR_FILENO);
    execvp(path.toString().c_str(), ccargs);
    std::string error = Core::util::getStrError(errno);
    std::cout << "Error executing " << path.toString() << ": " << error;
    exit(1);
  }
  else {

    std::shared_ptr<SubProcess> subprocess = std::make_shared<SubProcess>();
    subprocess->pid = pid;
    subprocess->cb = cb;
    subprocess->stdinfd = stdinpipe[1];
    subprocess->stdoutfd = stdoutpipe[0];
    subprocess->stderrfd = stderrpipe[0];
    subprocess->stdinhandle = global->getIOManager()->registerExternalFD(this, subprocess->stdinfd);
    subprocess->stdouthandle = global->getIOManager()->registerExternalFD(this, subprocess->stdoutfd);
    subprocess->stderrhandle = global->getIOManager()->registerExternalFD(this, subprocess->stderrfd);
    subprocesses.push_back(subprocess);
    return subprocess;
  }
  return std::shared_ptr<SubProcess>();
}

void SubProcessManager::FDData(int sockid) {
  for (std::list<std::shared_ptr<SubProcess>>::iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
    if ((*it)->stdouthandle == sockid) {
      char* buf = blockpool.getBlock();
      int brecv = read((*it)->stdoutfd, buf, blockpool.blockSize());
      if (brecv <= 0) {
        blockpool.returnBlock(buf);
        global->getIOManager()->closeSocket((*it)->stdouthandle);
      }
      else {
        if ((*it)->cb != nullptr) {
          (*it)->cb->processStdOut((*it)->pid, std::string(buf, brecv));
        }
        blockpool.returnBlock(buf);
      }
      return;
    }
    else if ((*it)->stderrhandle == sockid) {
      char* buf = blockpool.getBlock();
      int brecv = read((*it)->stderrfd, buf, blockpool.blockSize());
      if (brecv <= 0) {
        blockpool.returnBlock(buf);
        global->getIOManager()->closeSocket((*it)->stderrhandle);
      }
      else {
        if ((*it)->cb != nullptr) {
          (*it)->cb->processStdErr((*it)->pid, std::string(buf, brecv));
        }
        blockpool.returnBlock(buf);
      }
      return;
    }
  }
}

void SubProcessManager::killProcess(int pid) {
  for (std::list<std::shared_ptr<SubProcess>>::iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
    if ((*it)->pid == pid) {
      global->getIOManager()->closeSocket((*it)->stdinhandle);
      global->getIOManager()->closeSocket((*it)->stdouthandle);
      global->getIOManager()->closeSocket((*it)->stderrhandle);
      kill(pid, SIGHUP);
      if ((*it)->cb != nullptr) {
        (*it)->cb->processExited(pid, -1);
      }
      subprocesses.erase(it);
      break;
    }
  }
}

void SubProcessManager::killAll() {
  for (std::list<std::shared_ptr<SubProcess>>::iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
    global->getIOManager()->closeSocket((*it)->stdinhandle);
    global->getIOManager()->closeSocket((*it)->stdouthandle);
    global->getIOManager()->closeSocket((*it)->stderrhandle);
    kill((*it)->pid, SIGHUP);
    if ((*it)->cb != nullptr) {
      (*it)->cb->processExited((*it)->pid, -1);
    }
  }
  subprocesses.clear();
}

bool SubProcessManager::getIsRunning(int pid) const {
  for (std::list<std::shared_ptr<SubProcess>>::const_iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
    if ((*it)->pid == pid) {
      return true;
    }
  }
  return false;
}

void SubProcessManager::signal(int signal, int) {
  if (signal == SIGCHLD) {
    int pid = 0;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
      for (std::list<std::shared_ptr<SubProcess>>::iterator it = subprocesses.begin(); it != subprocesses.end(); it++) {
        if ((*it)->pid == pid) {
          global->getIOManager()->closeSocket((*it)->stdinhandle);
          global->getIOManager()->closeSocket((*it)->stdouthandle);
          global->getIOManager()->closeSocket((*it)->stderrhandle);
          if ((*it)->cb != nullptr) {
            (*it)->cb->processExited(pid, WEXITSTATUS(status));
          }
          subprocesses.erase(it);
          break;
        }
      }
    }
  }
  else {
    assert(false);
  }
}
