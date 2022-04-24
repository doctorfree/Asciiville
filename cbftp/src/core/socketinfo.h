#pragma once

#include <list>
#include <string>

#include <openssl/ssl.h>

#include "datablock.h"
#include "prio.h"
#include "types.h"

struct addrinfo;

namespace Core {

enum class SocketType {
  UNUSED,
  EXTERNAL,
  TCP_PLAIN,
  TCP_SSL,
  UDP,
  TCP_SERVER,
  TCP_SERVER_EXTERNAL,
  TCP_PLAIN_LISTEN,
  TCP_SSL_NEG_CONNECT,
  TCP_SSL_NEG_REDO_CONNECT,
  TCP_SSL_NEG_ACCEPT,
  TCP_SSL_NEG_REDO_ACCEPT,
  TCP_CONNECTING,
  TCP_RESOLVING,
  STOP
};

enum class Direction {
  IN,
  OUT
};

class EventReceiver;

class SocketInfo {
public:
  SocketInfo() : type(SocketType::UNUSED), addrfam(AddressFamily::NONE), fd(-1), id(-1), parentid(-1), port(0), localport(0), gairet(0),
                 gaires(nullptr), gaiasync(false), receiver(nullptr), ssl(nullptr), sslshutdown(true),
                 prio(Prio::NORMAL), paused(false), direction(Direction::IN),
                 listenimmediately(true), closing(false), sessionkey(-1)
  {
  }
  SocketType type;
  AddressFamily addrfam;
  int fd;
  int id;
  int parentid;
  std::string addr;
  int port;
  std::string localaddr;
  int localport;
  int gairet;
  struct addrinfo* gaires;
  std::string gaierr;
  bool gaiasync;
  EventReceiver* receiver;
  mutable std::list<DataBlock> sendqueue;
  SSL* ssl;
  bool sslshutdown;
  Prio prio;
  bool paused;
  Direction direction;
  bool listenimmediately;
  bool closing;
  int sessionkey;
};

} // namespace Core
