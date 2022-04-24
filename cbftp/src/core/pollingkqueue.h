#pragma once

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>

#include "polling.h"

namespace Core {

class PollingImpl : public PollingBase {
public:
  PollingImpl() : kqueuefd(kqueue()), events(MAX_EVENTS)
  {
  }
  ~PollingImpl() {
    close(kqueuefd);
  }
  static std::string type() {
    return "kqueue";
  }
  void wait(std::list<PollEvent>& eventlist) override {
    eventlist.clear();
    int fds = kevent(kqueuefd, nullptr, 0, events.data(), MAX_EVENTS, nullptr);
    for (int i = 0; i < fds; i++) {
      PollEventType type = PollEventType::UNKNOWN;
      if (events[i].filter == EVFILT_READ) {
        type = PollEventType::IN;
      }
      else if (events[i].filter == EVFILT_WRITE) {
        type = PollEventType::OUT;
      }
      eventlist.emplace_back(static_cast<int>(events[i].ident), type,
          reinterpret_cast<unsigned long long int>(events[i].udata));
    }
  }
  void addFDIn(int addfd, unsigned int userdata) override {
    control(addfd, EVFILT_READ, EV_ADD, userdata);
  }
  void addFDOut(int addfd, unsigned int userdata) override {
    control(addfd, EVFILT_WRITE, EV_ADD, userdata);
  }
  void removeFD(int delfd) override {
    control(delfd, EVFILT_READ | EVFILT_WRITE, EV_DELETE);
  }
  void setFDIn(int modfd, unsigned int userdata) override {
    controlSet(modfd, EVFILT_READ, EVFILT_WRITE, userdata);
  }
  void setFDOut(int modfd, unsigned int userdata) override {
    controlSet(modfd, EVFILT_WRITE, EVFILT_READ, userdata);
  }
private:
  void control(int fd, int filter, int flag, unsigned int userdata = 0) {
    struct kevent ev;
    EV_SET(&ev, fd, filter, flag, 0, 0, reinterpret_cast<void*>(userdata));
    kevent(kqueuefd, &ev, 1, nullptr, 0, nullptr);
  }
  void controlSet(int fd, int addfilter, int removefilter, unsigned int userdata = 0) {
    struct kevent ev[2];
    EV_SET(&ev[0], fd, removefilter, EV_DELETE, 0, 0, reinterpret_cast<void*>(userdata));
    EV_SET(&ev[1], fd, addfilter, EV_ADD, 0, 0, reinterpret_cast<void*>(userdata));
    kevent(kqueuefd, ev, 2, nullptr, 0, nullptr);
  }
  int kqueuefd;
  std::vector<struct kevent> events;
};

} // namespace Core
