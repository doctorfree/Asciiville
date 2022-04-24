#pragma once

#include <list>
#include <string>

namespace Core {

#define MAX_EVENTS 256

enum class PollEventType {
  IN,
  OUT,
  UNKNOWN
};

struct PollEvent {
  PollEvent(int fd, PollEventType type, unsigned int userdata)
    : fd(fd), type(type), userdata(userdata)
  {}
  int fd;
  PollEventType type;
  unsigned int userdata;
};

class PollingBase {
public:
  virtual ~PollingBase() {
  }
  virtual void wait(std::list<PollEvent>& events) = 0;
  virtual void addFDIn(int addfd, unsigned int userdata) = 0;
  virtual void addFDOut(int addfd, unsigned int userdata) = 0;
  virtual void removeFD(int delfd) = 0;
  virtual void setFDIn(int modfd, unsigned int userdata) = 0;
  virtual void setFDOut(int modfd, unsigned int userdata) = 0;
};

} // namespace Core

#ifdef __linux
#include "pollingepoll.h"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||\
    defined(__APPLE__)
#include "pollingkqueue.h"
#else
#include "pollingpoll.h"
#endif

namespace Core {

class Polling {
public:
  static std::string type() {
    return PollingImpl::type();
  }
  void wait(std::list<PollEvent>& events) {
    impl.wait(events);
  }
  void addFDIn(int addfd, unsigned int userdata = 0) {
    impl.addFDIn(addfd, userdata);
  }
  void addFDOut(int addfd, unsigned int userdata = 0) {
    impl.addFDOut(addfd, userdata);
  }
  void removeFD(int delfd) {
    impl.removeFD(delfd);
  }
  void setFDIn(int modfd, unsigned int userdata = 0) {
    impl.setFDIn(modfd, userdata);
  }
  void setFDOut(int modfd, unsigned int userdata = 0) {
    impl.setFDOut(modfd, userdata);
  }
private:
  PollingImpl impl;
};

} // namespace Core
