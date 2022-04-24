#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <vector>

#include "polling.h"

namespace Core {

class PollingImpl : public PollingBase {
public:
  PollingImpl() :
    epollfd(epoll_create(100)),
    events(MAX_EVENTS) {
  }
  ~PollingImpl() {
    close(epollfd);
  }
  static std::string type() {
    return "epoll";
  }
  void wait(std::list<PollEvent>& eventlist) override {
    eventlist.clear();
    int fds = epoll_wait(epollfd, events.data(), MAX_EVENTS, -1);
    for (int i = 0; i < fds; i++) {
      PollEventType type = PollEventType::UNKNOWN;
      if (events[i].events & EPOLLIN) {
        type = PollEventType::IN;
      }
      else if (events[i].events & EPOLLOUT) {
        type = PollEventType::OUT;
      }
      eventlist.emplace_back(static_cast<int>(events[i].data.u64 & 0xFFFFFFFF),
          type, events[i].data.u64 >> 32);
    }
  }
  void addFDIn(int addfd, unsigned int userdata) override {
    control(EPOLLIN, EPOLL_CTL_ADD, addfd, userdata);
  }
  void addFDOut(int addfd, unsigned int userdata) override {
    control(EPOLLOUT, EPOLL_CTL_ADD, addfd, userdata);
  }
  void removeFD(int delfd) override {
    control(EPOLLIN | EPOLLOUT, EPOLL_CTL_DEL, delfd);
  }
  void setFDIn(int modfd, unsigned int userdata) override {
    control(EPOLLIN, EPOLL_CTL_MOD, modfd, userdata);
  }
  void setFDOut(int modfd, unsigned int userdata) override {
    control(EPOLLOUT, EPOLL_CTL_MOD, modfd, userdata);
  }
private:
  void control(int ev, int op, int fd, unsigned int userdata = 0) {
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = ev;
    event.data.u64 = (static_cast<uint64_t>(userdata) << 32) + fd;
    epoll_ctl(epollfd, op, fd, &event);
  }
  int epollfd;
  std::vector<struct epoll_event> events;
};

} // namespace Core
