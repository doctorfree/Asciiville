#pragma once

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <cerrno>
#include <vector>

#include "polling.h"
#include "semaphore.h"

namespace Core {

class PollingImpl : public PollingBase {
public:
  PollingImpl() : fds(MAX_EVENTS), userdatas(MAX_EVENTS), fdmapupdated(false)
  {
  }
  static std::string type() {
    return "poll";
  }
  void wait(std::list<PollEvent>& eventlist) override {
    eventlist.clear();
    while (!eventlist.size()) {
      int evcount = 0;
      {
        std::lock_guard<std::mutex> lock(maplock);
        for (std::map<int, Event>::const_iterator it = fdmap.begin();
             it != fdmap.end() && evcount < MAX_EVENTS; it++, evcount++)
        {
          fds[evcount].fd = it->first;
          fds[evcount].events = it->second.events;
          userdatas[evcount] = it->second.userdata;
        }
        fdmapupdated = false;
      }
      if (poll(fds.data(), evcount, 10)) {
        {
          std::lock_guard<std::mutex> lock(maplock);
          if (fdmapupdated) {
            continue;
          }
        }
        for (int i = 0; i < evcount; i++) {
          int revents = fds[i].revents;
          if (!revents) {
            continue;
          }
          int fd = fds[i].fd;
          unsigned int userdata = userdatas[i];
          PollEventType type = PollEventType::UNKNOWN;
          if (revents & POLLIN) {
            char c;
            if (recv(fd, &c, 1, MSG_PEEK) <= 0 && errno != ENOTSOCK && errno != ENOTCONN) {
              removeFDIntern(fd);
            }
            type = PollEventType::IN;
          }
          else if (revents & POLLOUT) {
            type = PollEventType::OUT;
          }
          if (revents & (POLLHUP | POLLERR)) {
            removeFDIntern(fd);
          }
          if (type != PollEventType::UNKNOWN) {
            eventlist.emplace_back(fd, type, userdata);
          }
        }
      }
    }
  }
  void addFDIn(int addfd, unsigned int userdata) override {
    std::lock_guard<std::mutex> lock(maplock);
    std::map<int, Event>::iterator it = fdmap.find(addfd);
    if (it != fdmap.end()) {
      it->second.events |= POLLIN;
    }
    else {
      fdmap[addfd] = {POLLIN, userdata};
    }
    fdmapupdated = true;
  }
  void addFDOut(int addfd, unsigned int userdata) override {
    std::lock_guard<std::mutex> lock(maplock);
    std::map<int, Event>::iterator it = fdmap.find(addfd);
    if (it != fdmap.end()) {
      it->second.events |= POLLOUT;
    }
    else {
      fdmap[addfd] = {POLLOUT, userdata};
    }
    fdmapupdated = true;
  }
  void removeFD(int delfd) override {
    std::lock_guard<std::mutex> lock(maplock);
    std::map<int, Event>::iterator it = fdmap.find(delfd);
    if (it != fdmap.end()) {
      fdmap.erase(it);
    }
    fdmapupdated = true;
  }
  void setFDIn(int modfd, int unsigned userdata) override {
    std::lock_guard<std::mutex> lock(maplock);
    fdmap[modfd] = {POLLIN, userdata};
    fdmapupdated = true;
  }
  void setFDOut(int modfd, unsigned int userdata) override {
    std::lock_guard<std::mutex> lock(maplock);
    fdmap[modfd] = {POLLOUT, userdata};
    fdmapupdated = true;
  }
private:
  void removeFDIntern(int delfd) {
    fdmap.erase(delfd);
  }
  std::mutex maplock;
  std::vector<struct pollfd> fds;
  std::vector<unsigned int> userdatas;
  struct Event {
    int events;
    unsigned int userdata;
  };
  std::map<int, Event> fdmap;
  bool fdmapupdated;
};

} // namespace Core
