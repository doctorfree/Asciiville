#include "iomanager.h"

#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

#include "datablock.h"
#include "datablockpool.h"
#include "logger.h"
#include "sslmanager.h"
#include "tickpoke.h"
#include "util.h"
#include "workmanager.h"

namespace Core {

enum class ResolverResult {
  SUCCESS,
  WOULD_BLOCK,
  SOCKID_NON_EXISTING
};

namespace {

const unsigned int TICKPERIOD = 100;
const unsigned int MAX_SEND_BUFFER = 1048576;
const int CONNECT_TIMEOUT_MS = 5000;

enum class AsyncTaskType {
  HOST_RESOLUTION
};

void resolveHostAsync(EventReceiver* er, int sockid) {
  static_cast<IOManager*>(er)->resolveHost(sockid, true);
}

DisconnectType investigateSSLError(SocketInfo& socketinfo, const char* function, int brecv, std::string& errortext) {
  int errorErrno = errno;
  int sslError = SSL_get_error(socketinfo.ssl, brecv);
  switch (sslError) {
    case SSL_ERROR_WANT_READ:
      return DisconnectType::NONE;
    case SSL_ERROR_WANT_WRITE:
      return DisconnectType::NONE;
    case SSL_ERROR_ZERO_RETURN:
      errortext = "Connection closed gracefully by peer";
      return DisconnectType::GRACEFUL;
    case SSL_ERROR_SYSCALL:
      socketinfo.sslshutdown = false;
      if (!errorErrno) {
        errortext = "Connection closed abruptly by peer";
        return DisconnectType::ABRUPT;
      }
      break;
    case SSL_ERROR_SSL:
      socketinfo.sslshutdown = false;
      break;
  }
  unsigned long e = ERR_get_error();
  if (SSLManager::isAbruptDisconnectError(e)) {
    errortext = "Connection closed abruptly by peer";
    return DisconnectType::ABRUPT;
  }
  char buf[util::ERR_BUF_SIZE];
  ERR_error_string_n(e, buf, sizeof(buf));
  errortext = std::string(function) + ": " + SSLManager::sslErrorToString(sslError) +
                   (e ? ": " + std::string(buf) : ", return code: " +
                   std::to_string(brecv) + ", errno: " +
                   std::to_string(errorErrno) + " = " + util::getStrError(errorErrno));
  return DisconnectType::ERROR;
}

} // namespace

IOManager::IOManager(WorkManager& wm, TickPoke& tp)
  : workmanager(wm)
  , tickpoke(tp)
  , blockpool(workmanager.getBlockPool())
  , sendblockpool(std::make_shared<DataBlockPool>())
  , blocksize(blockpool.blockSize())
  , sockidcounter(0)
  , hasbindinterface(false)
  , sessionkeycounter(0)
{
  workmanager.addReadyNotify(this);
}

IOManager::~IOManager() {
  stop();
}

void IOManager::init(const std::string& prefix, int id) {
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGPIPE, &sa, nullptr);
  thread.start((prefix + "-io-" + std::to_string(id)).c_str(), this);
  tickpoke.startPoke(this, "IOManager", TICKPERIOD, 0);
  initialized.wait();
}

void IOManager::preStop() {
  tickpoke.stopPoke(this, 0);
  int fd[2];
  bool success = false;
  for (int i = 0; i < 20; ++i) {
    if (pipe(fd) == 0) {
      success = true;
      break;
    }
    usleep(100000);
  }
  if (!success) {
    getLogger()->log("IOManager", std::string("Cannot create pipe: ") + util::getStrError(errno), LogLevel::ERROR);
    assert(false);
  }
  close(fd[0]);
  socketinfomaplock.lock();
  int sockid = sockidcounter++;
  SocketInfo& socketinfo = socketinfomap[sockid];
  socketinfo.fd = fd[1];
  socketinfo.addrfam = AddressFamily::NONE;
  socketinfo.id = sockid;
  socketinfo.type = SocketType::STOP;
  socketinfo.receiver = nullptr;
  socketinfomaplock.unlock();
  pollWrite(socketinfo);
}

void IOManager::stop() {
  preStop();
  thread.join();
  while (!socketinfomap.empty()) {
    closeSocketIntern(socketinfomap.begin()->first);
  }
  while (!sessions.empty()) {
    clearSession(sessions.begin()->first);
  }
}

void IOManager::tick(int message) {
  bool closeFd = false;
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  for (auto& it : connecttimemap) {
    int& timeelapsed = it.second;
    timeelapsed += TICKPERIOD;
    if (timeelapsed >= CONNECT_TIMEOUT_MS) {
      timeelapsed = -1;
      closeFd = true;
    }
  }
  if (closeFd) {
    bool changes = true;
    while (changes) {
      changes = false;
      for (auto& it : connecttimemap) {
        if (it.second == -1) {
          changes = true;
          auto it2 = socketinfomap.find(it.first);
          if (it2 != socketinfomap.end()) {
            EventReceiver* er = it2->second.receiver;
            if (!it2->second.closing) {
              workmanager.dispatchEventFail(er, it2->second.id, "Connection timeout");
            }
          }
          closeSocketIntern(it.first);
          break;
        }
      }
    }
  }
}

void IOManager::pollRead(SocketInfo& socketinfo) {
  socketinfo.direction = Direction::IN;
  if (!socketinfo.paused) {
    polling.addFDIn(socketinfo.fd, socketinfo.id);
  }
}

void IOManager::pollWrite(SocketInfo& socketinfo) {
  socketinfo.direction = Direction::OUT;
  if (!socketinfo.paused) {
    polling.addFDOut(socketinfo.fd, socketinfo.id);
  }
}

void IOManager::setPollRead(SocketInfo& socketinfo) {
  socketinfo.direction = Direction::IN;
  if (!socketinfo.paused) {
    polling.setFDIn(socketinfo.fd, socketinfo.id);
  }
}

void IOManager::setPollWrite(SocketInfo& socketinfo) {
  socketinfo.direction = Direction::OUT;
  if (!socketinfo.paused) {
    polling.setFDOut(socketinfo.fd, socketinfo.id);
  }
}

void IOManager::unsetPoll(SocketInfo& socketinfo) {
  polling.removeFD(socketinfo.fd);
}

int IOManager::registerTCPClientSocket(EventReceiver* er, const std::string& addr, int port) {
  bool resolving;
  return registerTCPClientSocket(er, addr, port, resolving);
}

int IOManager::registerTCPClientSocket(EventReceiver* er,
                                       const std::string& addr,
                                       int port,
                                       bool& resolving,
                                       AddressFamily addrfam,
                                       bool listenimmediately)
{
  socketinfomaplock.lock();
  int sockid = sockidcounter++;
  SocketInfo& socketinfo = socketinfomap[sockid];
  socketinfo.id = sockid;
  socketinfo.addrfam = addrfam;
  socketinfo.fd = -1;
  socketinfo.addr = addr;
  socketinfo.port = port;
  socketinfo.type = SocketType::TCP_RESOLVING;
  socketinfo.receiver = er;
  socketinfo.listenimmediately = listenimmediately;
  connecttimemap[sockid] = 0;
  socketinfomaplock.unlock();
  ResolverResult result = resolveHost(sockid, false);
  resolving = result == ResolverResult::WOULD_BLOCK;
  if (result == ResolverResult::SUCCESS) {
    std::lock_guard<std::mutex> lock(socketinfomaplock);
    handleTCPNameResolution(socketinfo);
  }
  else if (result == ResolverResult::WOULD_BLOCK) {
    workmanager.asyncTask(this, static_cast<int>(AsyncTaskType::HOST_RESOLUTION), &resolveHostAsync, sockid);
  }
  return sockid;
}

void IOManager::handleTCPNameResolution(SocketInfo& socketinfo) {
  if (socketinfo.gairet) {
    if (!socketinfo.closing) {
      workmanager.dispatchEventFail(socketinfo.receiver, -1, socketinfo.gaierr);
    }
    closeSocketIntern(socketinfo.id);
    return;
  }
  struct addrinfo* result = static_cast<struct addrinfo*>(socketinfo.gaires);
  int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (sockfd == -1) {
    if (!handleError(socketinfo.receiver, socketinfo.id)) {
      closeSocketIntern(socketinfo.id);
      return;
    }
  }
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  char buf[INET6_ADDRSTRLEN];
  if (socketinfo.addrfam == AddressFamily::IPV4) {
    struct sockaddr_in* saddr = (struct sockaddr_in*)result->ai_addr;
    inet_ntop(AF_INET, &(saddr->sin_addr), buf, INET_ADDRSTRLEN);
  }
  else {
    struct sockaddr_in6* saddr = (struct sockaddr_in6*)result->ai_addr;
    inet_ntop(AF_INET6, &(saddr->sin6_addr), buf, INET6_ADDRSTRLEN);
  }
  if (socketinfo.addrfam == AddressFamily::IPV6) {
    int yes = 1;
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(int));
  }
  socketinfo.fd = sockfd;
  socketinfo.type = SocketType::TCP_CONNECTING;
  socketinfo.addr = buf;
  connecttimemap[socketinfo.id] = 0;
  StringResult bindto = getAddressToBind(socketinfo.addrfam, socketinfo.type);
  if (!bindto.success) {
    workmanager.dispatchEventFail(socketinfo.receiver, socketinfo.id, bindto.error);
    closeSocketIntern(socketinfo.id);
    return;
  }
  if (!bindto.result.empty()) {
    struct addrinfo request, *res;
    memset(&request, 0, sizeof(request));
    request.ai_family = (socketinfo.addrfam == AddressFamily::IPV4 ? AF_INET : AF_INET6);
    request.ai_socktype = SOCK_STREAM;
    int returncode = getaddrinfo(bindto.result.c_str(), "0", &request, &res);
    if (returncode) {
      workmanager.dispatchEventFail(socketinfo.receiver, socketinfo.id, gai_strerror(returncode));
      closeSocketIntern(socketinfo.id);
      return;
    }
    returncode = bind(sockfd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (returncode) {
      if (!handleError(socketinfo.receiver, socketinfo.id)) {
        closeSocketIntern(socketinfo.id);
        return;
      }
    }
  }
  int returnCode = connect(sockfd, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);
  socketinfo.gaires = nullptr;
  if (returnCode) {
    if (!handleError(socketinfo.receiver, socketinfo.id)) {
      closeSocketIntern(socketinfo.id);
      return;
    }
  }
  if (socketinfo.gaiasync && !socketinfo.closing) {
    workmanager.dispatchEventConnecting(socketinfo.receiver, socketinfo.id, socketinfo.addr);
  }
  pollWrite(socketinfo);
  return;
}

int IOManager::registerTCPServerSocket(EventReceiver* er, int port, AddressFamily addrfam, bool local) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  StringResult bindto = getAddressToBind(addrfam, SocketType::TCP_SERVER);
  if (!bindto.success) {
    workmanager.dispatchEventFail(er, -1, bindto.error);
    return -1;
  }
  std::string addr;
  if (addrfam == AddressFamily::IPV4) {
    sock.ai_family = AF_INET;
    addr = local ? "127.0.0.1" : "0.0.0.0";
  }
  else {
    sock.ai_family = AF_INET6;
    addr = local ? "::1" : "::";
  }
  if (!bindto.result.empty() && !local) {
    addr = bindto.result;
  }
  sock.ai_socktype = SOCK_STREAM;
  int returncode = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &sock, &res);
  if (returncode) {
    workmanager.dispatchEventFail(er, -1, gai_strerror(returncode));
    return -1;
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (addrfam == AddressFamily::IPV6) {
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(int));
  }
  returncode = bind(sockfd, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);
  if (returncode) {
    if (!handleError(er)) {
      close(sockfd);
      return -1;
    }
  }
  returncode = listen(sockfd, 100);
  if (returncode) {
    if (!handleError(er)) {
      close(sockfd);
      return -1;
    }
  }
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  int sockid = sockidcounter++;
  SocketInfo& socketinfo = socketinfomap[sockid];
  socketinfo.fd = sockfd;
  socketinfo.addrfam = addrfam;
  socketinfo.id = sockid;
  socketinfo.type = SocketType::TCP_SERVER;
  socketinfo.receiver = er;
  pollRead(socketinfo);
  return sockid;
}

int IOManager::registerTCPServerSocketExternalFD(EventReceiver* er, int sockfd, AddressFamily addrfam) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  int sockid = sockidcounter++;
  SocketInfo& socketinfo = socketinfomap[sockid];
  socketinfo.fd = sockfd;
  socketinfo.addrfam = addrfam;
  socketinfo.id = sockid;
  socketinfo.type = SocketType::TCP_SERVER_EXTERNAL;
  socketinfo.receiver = er;
  pollRead(socketinfo);
  return sockid;
}

void IOManager::registerTCPServerClientSocket(EventReceiver* er, int sockid, bool listenimmediately) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  socketinfomap[sockid].receiver = er;
  socketinfomap[sockid].listenimmediately = listenimmediately;
  if (listenimmediately) {
    pollRead(socketinfomap[sockid]);
  }
}

int IOManager::registerExternalFD(EventReceiver* er, int fd) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  int sockid = sockidcounter++;
  SocketInfo& socketinfo = socketinfomap[sockid];
  socketinfo.fd = fd;
  socketinfo.addrfam = AddressFamily::NONE;
  socketinfo.id = sockid;
  socketinfo.type = SocketType::EXTERNAL;
  socketinfo.receiver = er;
  pollRead(socketinfo);
  return sockid;
}

int IOManager::registerUDPServerSocket(EventReceiver* er, int port, AddressFamily addrfam) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  std::string addr;
  if (addrfam == AddressFamily::IPV4) {
      sock.ai_family = AF_INET;
      addr = "0.0.0.0";
  }
  else {
      sock.ai_family = AF_INET6;
      addr = "::";
  }
  sock.ai_socktype = SOCK_DGRAM;
  sock.ai_protocol = IPPROTO_UDP;
  int returncode = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &sock, &res);
  if (returncode) {
    workmanager.dispatchEventFail(er, -1, gai_strerror(returncode));
    return -1;
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (addrfam == AddressFamily::IPV6) {
    setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(int));
  }
  returncode = bind(sockfd, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);
  if (returncode) {
    if (!handleError(er)) {
      close(sockfd);
      return -1;
    }
  }
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  int sockid = sockidcounter++;
  SocketInfo& socketinfo = socketinfomap[sockid];
  socketinfo.fd = sockfd;
  socketinfo.addrfam = addrfam;
  socketinfo.id = sockid;
  socketinfo.type = SocketType::UDP;
  socketinfo.receiver = er;
  pollRead(socketinfo);
  return sockid;
}

void IOManager::adopt(EventReceiver* er, int sockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  socketinfomap[sockid].receiver = er;
}

void IOManager::negotiateSSLConnect(int sockid) {
  negotiateSSLConnect(sockid, -1);
}

void IOManager::negotiateSSLConnect(int sockid, int sessionkey) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end() &&
      (it->second.type == SocketType::TCP_PLAIN || it->second.type == SocketType::TCP_PLAIN_LISTEN))
  {
    it->second.type = SocketType::TCP_SSL_NEG_CONNECT;
    it->second.sessionkey = sessionkey;
    if (it->second.listenimmediately) {
      setPollWrite(it->second);
    }
    else {
      pollWrite(it->second);
    }
  }
}

void IOManager::negotiateSSLConnectParent(int sockid, int parentsockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end() &&
      (it->second.type == SocketType::TCP_PLAIN || it->second.type == SocketType::TCP_PLAIN_LISTEN))
  {
    it->second.type = SocketType::TCP_SSL_NEG_CONNECT;
    it->second.parentid = parentsockid;
    if (it->second.listenimmediately) {
      setPollWrite(it->second);
    }
    else {
      pollWrite(it->second);
    }
  }
}

void IOManager::negotiateSSLAccept(int sockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end() &&
      (it->second.type == SocketType::TCP_PLAIN || it->second.type == SocketType::TCP_PLAIN_LISTEN))
  {
    it->second.type = SocketType::TCP_SSL_NEG_ACCEPT;
    if (it->second.listenimmediately) {
      setPollWrite(it->second);
    }
    else {
      pollWrite(it->second);
    }
  }
}

bool IOManager::handleError(EventReceiver* er, int sockid) {
  if (errno == EINPROGRESS) {
    return true;
  }
  workmanager.dispatchEventFail(er, sockid, util::getStrError(errno));
  return false;
}

bool IOManager::sendData(int sockid, const std::string& data) {
  return sendData(sockid, data.c_str(), data.length());
}

bool IOManager::sendData(int sockid, const std::vector<char>& data) {
  if (data.empty()) {
    return true;
  }
  return sendData(sockid, &data[0], data.size());
}

bool IOManager::sendData(int sockid, const char* data, unsigned int datalen) {
  assert(datalen <= MAX_SEND_BUFFER);
  unsigned int sendblocksize = sendblockpool->blockSize();
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    return true;
  }
  SocketInfo& socketinfo = it->second;
  while (datalen > 0) {
    char* dataBlock = sendblockpool->getBlock();
    int copysize = datalen < sendblocksize ? datalen : sendblocksize;
    memcpy(dataBlock, data, copysize);
    socketinfo.sendqueue.push_back(DataBlock(dataBlock, copysize));
    data += copysize;
    datalen -= copysize;
  }
  if (socketinfo.type == SocketType::TCP_PLAIN || socketinfo.type == SocketType::TCP_PLAIN_LISTEN ||
      socketinfo.type == SocketType::TCP_SSL)
  {
    setPollWrite(socketinfo);
  }
  return socketinfo.sendqueue.size() * sendblockpool->blockSize() <= MAX_SEND_BUFFER;
}

void IOManager::closeSocket(int sockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    return;
  }
  if (!it->second.sendqueue.empty()) {
    it->second.closing = true;
  }
  else {
    closeSocketIntern(sockid);
  }
}

void IOManager::closeSocketNow(int sockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  closeSocketIntern(sockid);
}

void IOManager::closeSocketIntern(int sockid) {
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    return;
  }
  SocketInfo& socketinfo = it->second;
  polling.removeFD(socketinfo.fd);
  if (socketinfo.ssl && socketinfo.sslshutdown && !SSL_in_init(socketinfo.ssl)) {
    SSL_shutdown(socketinfo.ssl);
  }
  if (socketinfo.type != SocketType::EXTERNAL && socketinfo.type != SocketType::TCP_SERVER_EXTERNAL) {
    close(socketinfo.fd);
  }
  if (socketinfo.ssl) {
    SSL_free(socketinfo.ssl);
  }
  if (socketinfo.gaires) {
    freeaddrinfo(static_cast<struct addrinfo*>(socketinfo.gaires));
  }
  for (DataBlock& block : socketinfo.sendqueue) {
    sendblockpool->returnBlock(block.rawData());
  }
  socketinfo.sendqueue.clear();
  connecttimemap.erase(sockid);
  autopaused.erase(sockid);
  manuallypaused.erase(sockid);
  socketinfomap.erase(it);
}

ResolverResult IOManager::resolveHost(int sockid, bool mayblock) {
  socketinfomaplock.lock();
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    socketinfomaplock.unlock();
    return ResolverResult::SOCKID_NON_EXISTING;
  }
  std::string addr = it->second.addr;
  int port = it->second.port;
  struct addrinfo request;
  memset(&request, 0, sizeof(request));
  if (it->second.addrfam == AddressFamily::IPV4) {
    request.ai_family = AF_INET;
  }
  else if (it->second.addrfam == AddressFamily::IPV6) {
    request.ai_family = AF_INET6;
  }
  else {
    request.ai_family = AF_UNSPEC;
  }
  request.ai_socktype = SOCK_STREAM;
  request.ai_flags = AI_ADDRCONFIG;
  if (!mayblock) {
    request.ai_flags |= AI_NUMERICHOST;
  }
  socketinfomaplock.unlock();
  struct addrinfo* result;
  int returncode = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &request, &result);
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    if (!returncode) {
      freeaddrinfo(result);
    }
    return ResolverResult::SOCKID_NON_EXISTING;
  }
  it->second.gairet = returncode;
  if (returncode == EAI_NONAME && !mayblock) {
    it->second.gaiasync = true;
    return ResolverResult::WOULD_BLOCK;
  }
  if (returncode) {
    it->second.gaierr = gai_strerror(returncode);
  }
  else {
    it->second.gaires = result;
    if (request.ai_family == AF_UNSPEC) {
      it->second.addrfam = (result->ai_family == AF_INET ? AddressFamily::IPV4 : AddressFamily::IPV6);
    }
  }
  return ResolverResult::SUCCESS;
}

void IOManager::asyncTaskComplete(int type, int sockid) {
  assert(type == static_cast<int>(AsyncTaskType::HOST_RESOLUTION));
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
      return;
  }
  assert(it->second.type == SocketType::TCP_RESOLVING);
  handleTCPNameResolution(it->second);
}

std::string IOManager::getCipher(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end() || it->second.ssl == nullptr || it->second.type != SocketType::TCP_SSL) {
    return "?";
  }
  const char* cipher = SSLManager::getCipher(it->second.ssl);
  return std::string(cipher);
}

bool IOManager::getSSLSessionReuse(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    return it->second.parentid != -1 || it->second.sessionkey != -1;
  }
  return false;
}

bool IOManager::getSSLSessionReused(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end() || it->second.ssl == nullptr || it->second.type != SocketType::TCP_SSL) {
    return false;
  }
  return SSL_session_reused(it->second.ssl);
}

int IOManager::getReusedSessionKey(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    return it->second.sessionkey;
  }
  return -1;
}

std::string IOManager::getSocketAddress(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    return it->second.addr;
  }
  return "?";
}

int IOManager::getSocketPort(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    return it->second.port;
  }
  return -1;
}

int IOManager::getSocketFileDescriptor(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    return it->second.fd;
  }
  return -1;
}

std::string IOManager::getInterfaceAddress(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    return it->second.localaddr;
  }
  return "?";
}

StringResult IOManager::getInterfaceAddress4(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    if (it->second.addrfam == AddressFamily::IPV4) {
      return it->second.localaddr;
    }
    StringResult interfaceresult = getInterfaceName(it->second.localaddr);
    if (!interfaceresult.success) {
      return interfaceresult;
    }
    return getInterfaceAddress(interfaceresult.result);
  }
  return StringResult();
}

StringResult IOManager::getInterfaceAddress6(int sockid) const {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    if (it->second.addrfam == AddressFamily::IPV6) {
      return it->second.localaddr;
    }
    StringResult interfaceresult = getInterfaceName(it->second.localaddr);
    if (!interfaceresult.success) {
      return interfaceresult;
    }
    return getInterfaceAddress6(interfaceresult.result);
  }
  return StringResult();
}

AddressFamily IOManager::getAddressFamily(int sockid) const {
  std::string addr = "?";
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it != socketinfomap.end()) {
    return it->second.addrfam;
  }
  return AddressFamily::IPV4_IPV6;
}

void IOManager::handleExternalIn(SocketInfo& socketinfo) {
  socketinfomaplock.unlock();
  workmanager.dispatchFDData(socketinfo.receiver, socketinfo.id);
  socketinfomaplock.lock();
}

void IOManager::handleTCPConnectingOut(SocketInfo& socketinfo) {
  unsigned int error;
  socklen_t errorLen = sizeof(error);
  getsockopt(socketinfo.fd, SOL_SOCKET, SO_ERROR, &error, &errorLen);
  if (error != 0) {
    if (!socketinfo.closing) {
      workmanager.dispatchEventFail(socketinfo.receiver, socketinfo.id, util::getStrError(error));
    }
    closeSocketIntern(socketinfo.id);
    return;
  }
  char buf[INET6_ADDRSTRLEN];
  if (socketinfo.addrfam == AddressFamily::IPV4) {
    struct sockaddr_in localaddr;
    socklen_t localaddrLen = sizeof(localaddr);
    getsockname(socketinfo.fd, (struct sockaddr*)&localaddr, &localaddrLen);
    inet_ntop(AF_INET, &localaddr.sin_addr, buf, INET_ADDRSTRLEN);
    socketinfo.localport = ntohs(localaddr.sin_port);
  }
  else {
    struct sockaddr_in6 localaddr;
    socklen_t localaddrLen = sizeof(localaddr);
    getsockname(socketinfo.fd, (struct sockaddr*)&localaddr, &localaddrLen);
    inet_ntop(AF_INET6, &localaddr.sin6_addr, buf, INET6_ADDRSTRLEN);
    socketinfo.localport = ntohs(localaddr.sin6_port);
  }
  socketinfo.localaddr = buf;
  socketinfo.type = SocketType::TCP_PLAIN;
  connecttimemap.erase(socketinfo.id);
  if (!socketinfo.closing) {
    workmanager.dispatchEventConnected(socketinfo.receiver, socketinfo.id);
  }
  if (socketinfo.closing && (socketinfo.sendqueue.empty() || !socketinfo.listenimmediately)) {
    closeSocketIntern(socketinfo.id);
    return;
  }
  if (socketinfo.listenimmediately) {
    if (socketinfo.sendqueue.empty()) {
      setPollRead(socketinfo);
    }
  }
  else {
    unsetPoll(socketinfo);
  }
}

void IOManager::handleTCPPlainIn(SocketInfo& socketinfo) {
  char* buf = blockpool.getBlock();
  int brecv = read(socketinfo.fd, buf, blocksize);
  if (brecv == 0) {
    blockpool.returnBlock(buf);
    workmanager.dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, DisconnectType::GRACEFUL, "Connection closed by peer", socketinfo.prio);
    closeSocketIntern(socketinfo.id);
    return;
  }
  else if (brecv < 0) {
    int error = errno;
    blockpool.returnBlock(buf);
    if (error == EAGAIN || error == EWOULDBLOCK) {
      return;
    }
    std::string errortext = "Socket read error: " + std::to_string(error) + " = " + util::getStrError(error);
    workmanager.dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, DisconnectType::ERROR, errortext, socketinfo.prio);
    closeSocketIntern(socketinfo.id);
    return;
  }
  if (!workmanager.dispatchFDData(socketinfo.receiver, socketinfo.id, buf, brecv, socketinfo.prio)) {
    autoPause(socketinfo);
  }
}

void IOManager::handleTCPPlainOut(SocketInfo& socketinfo) {
  while (!socketinfo.sendqueue.empty()) {
    DataBlock& block = socketinfo.sendqueue.front();
    int bsent = write(socketinfo.fd, block.data(), block.dataLength());
    if (bsent < 0) {
      int error = errno;
      if (error == EAGAIN || error == EWOULDBLOCK) {
          return;
      }
      if (!socketinfo.closing) {
        std::string errortext = "Socket write error: " + std::to_string(error) + " = " + util::getStrError(error);
        workmanager.dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, DisconnectType::ERROR, errortext, socketinfo.prio);
      }
      closeSocketIntern(socketinfo.id);
      return;
    }
    if (bsent < block.dataLength()) {
      block.consume(bsent);
      return;
    }
    else {
      sendblockpool->returnBlock(block.rawData());
      socketinfo.sendqueue.pop_front();
    }
  }
  if (socketinfo.closing) {
    closeSocketIntern(socketinfo.id);
    return;
  }
  setPollRead(socketinfo);
  workmanager.dispatchEventSendComplete(socketinfo.receiver, socketinfo.id, socketinfo.prio);
}

void IOManager::handleTCPSSLNegotiationIn(SocketInfo& socketinfo) {
  SSL* ssl = socketinfo.ssl;
  int brecv;
  const char* function = nullptr;
  ERR_clear_error();
  if (socketinfo.type == SocketType::TCP_SSL_NEG_REDO_CONNECT) {
    function = "SSL_connect";
    brecv = SSL_connect(ssl);
  }
  else if (socketinfo.type == SocketType::TCP_SSL_NEG_REDO_ACCEPT) {
    function = "SSL_accept";
    brecv = SSL_accept(ssl);
  }
  else {
    function = "SSL_do_handshake";
    brecv = SSL_do_handshake(ssl);
  }
  if (brecv > 0) {
    socketinfo.type = SocketType::TCP_SSL;
    if (!socketinfo.closing) {
      const char* cipher = SSLManager::getCipher(ssl);
      workmanager.dispatchEventSSLSuccess(socketinfo.receiver, socketinfo.id, cipher, socketinfo.prio);
    }
    if (!socketinfo.sendqueue.empty()) {
      setPollWrite(socketinfo);
    }
  }
  else {
    std::string errortext;
    DisconnectType type = investigateSSLError(socketinfo, function, brecv, errortext);
    if (type != DisconnectType::NONE) {
      if (!socketinfo.closing) {
        workmanager.dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, type, errortext, socketinfo.prio);
      }
      closeSocketIntern(socketinfo.id);
    }
  }
}

void IOManager::handleTCPSSLNegotiationOut(SocketInfo& socketinfo) {
  SSL* ssl = SSL_new(socketinfo.type == SocketType::TCP_SSL_NEG_CONNECT ? SSLManager::getClientSSLCTX()
                                                                        : SSLManager::getServerSSLCTX());
  SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  socketinfo.ssl = ssl;
  SSL_set_fd(ssl, socketinfo.fd);
  if (socketinfo.parentid != -1) {
    auto it = socketinfomap.find(socketinfo.parentid);
    if (it != socketinfomap.end() && it->second.ssl != nullptr) {
      SSL_copy_session_id(ssl, it->second.ssl);
    }
  }
  else if (socketinfo.sessionkey != -1) {
    auto it = sessions.find(socketinfo.sessionkey);
    if (it != sessions.end()) {
      int ret = SSL_set_session(ssl, it->second);
      assert(ret == 1);
    }
  }
  int ret = -1;
  const char* function = nullptr;
  if (socketinfo.type == SocketType::TCP_SSL_NEG_CONNECT) {
    function = "SSL_connect";
    ret = SSL_connect(ssl);
  }
  else if (socketinfo.type == SocketType::TCP_SSL_NEG_ACCEPT) {
    function = "SSL_accept";
    ret = SSL_accept(ssl);
  }
  if (ret > 0) {
    socketinfo.type = SocketType::TCP_SSL;
    if (!socketinfo.closing) {
      const char* cipher = SSLManager::getCipher(ssl);
      workmanager.dispatchEventSSLSuccess(socketinfo.receiver, socketinfo.id, cipher, socketinfo.prio);
    }
    if (!socketinfo.sendqueue.empty()) {
      return;
    }
  }
  else {
    std::string errortext;
    DisconnectType type = investigateSSLError(socketinfo, function, ret, errortext);
    if (type != DisconnectType::NONE) {
      if (!socketinfo.closing) {
        workmanager.dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, type, errortext, socketinfo.prio);
      }
      closeSocketIntern(socketinfo.id);
      return;
    }
    if (socketinfo.type == SocketType::TCP_SSL_NEG_CONNECT) {
      socketinfo.type = SocketType::TCP_SSL_NEG_REDO_CONNECT;
    }
    else {
      socketinfo.type = SocketType::TCP_SSL_NEG_REDO_ACCEPT;
    }
  }
  setPollRead(socketinfo);
}

void IOManager::handleTCPSSLIn(SocketInfo& socketinfo) {
  SSL* ssl = socketinfo.ssl;
  int blocknum = 0;
  bool pause = false;
  while (true) {
    char* buf = blockpool.getBlock();
    int bufpos = 0;
    while (bufpos < blocksize) {
      ERR_clear_error();
      int brecv = SSL_read(ssl, buf + bufpos, blocksize - bufpos);
      if (brecv <= 0) {
        if (bufpos > 0) {
          if (!workmanager.dispatchFDData(socketinfo.receiver, socketinfo.id, buf, bufpos, socketinfo.prio)) {
            autoPause(socketinfo);
          }
        }
        else {
          blockpool.returnBlock(buf);
        }
        std::string errortext;
        DisconnectType type = investigateSSLError(socketinfo, "SSL_read", brecv, errortext);
        if (type != DisconnectType::NONE) {
          workmanager.dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, type, errortext, socketinfo.prio);
          closeSocketIntern(socketinfo.id);
        }
        return;
      }
      bufpos += brecv;
    }
    if (blocknum++ > 16) {
      socketinfo.prio = Prio::LOW;
    }
    if (!workmanager.dispatchFDData(socketinfo.receiver, socketinfo.id, buf, bufpos, socketinfo.prio)) {
      pause = true;
    }
  }
  if (pause) {
    autoPause(socketinfo);
  }
}

void IOManager::handleTCPSSLOut(SocketInfo& socketinfo) {
  while (!socketinfo.sendqueue.empty()) {
    DataBlock& block = socketinfo.sendqueue.front();
    SSL* ssl = socketinfo.ssl;
    ERR_clear_error();
    int bsent = SSL_write(ssl, block.data(), block.dataLength());
    if (bsent <= 0) {
      std::string errortext;
      DisconnectType type = investigateSSLError(socketinfo, "SSL_write", bsent, errortext);
      if (type != DisconnectType::NONE) {
        if (!socketinfo.closing) {
          workmanager.dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, type, errortext, socketinfo.prio);
        }
        closeSocketIntern(socketinfo.id);
      }
      return;
    }
    if (bsent < block.dataLength()) {
      block.consume(bsent);
      return;
    }
    sendblockpool->returnBlock(block.rawData());
    socketinfo.sendqueue.pop_front();
  }
  if (socketinfo.closing) {
    closeSocketIntern(socketinfo.id);
    return;
  }
  setPollRead(socketinfo);
  workmanager.dispatchEventSendComplete(socketinfo.receiver, socketinfo.id, socketinfo.prio);
}

void IOManager::handleUDPIn(SocketInfo& socketinfo) {
  char* buf = blockpool.getBlock();
  int brecv = recvfrom(socketinfo.fd, buf, blocksize, 0, nullptr, nullptr);
  workmanager.dispatchFDData(socketinfo.receiver, socketinfo.id, buf, brecv, socketinfo.prio);
}

void IOManager::handleTCPServerIn(SocketInfo& socketinfo) {
  int newfd = -1;
  int newsockid = -1;
  if (socketinfo.addrfam == AddressFamily::IPV4) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrLen = sizeof(addr);
    newfd = accept(socketinfo.fd, (struct sockaddr*)&addr, &addrLen);
    if (newfd == -1) {
      return;
    }
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, buf, INET_ADDRSTRLEN);
    newsockid = sockidcounter++;
    socketinfomap[newsockid].addr = buf;
    socketinfomap[newsockid].port = ntohs(addr.sin_port);
    getsockname(newfd, (struct sockaddr*)&addr, &addrLen);
    inet_ntop(AF_INET, &addr.sin_addr, buf, INET_ADDRSTRLEN);
    socketinfomap[newsockid].localaddr = buf;
    socketinfomap[newsockid].localport = ntohs(addr.sin_port);
  }
  else {
    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addrLen = sizeof(addr);
    newfd = accept(socketinfo.fd, (struct sockaddr*)&addr, &addrLen);
    if (newfd == -1) {
      return;
    }
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &addr.sin6_addr, buf, INET6_ADDRSTRLEN);
    newsockid = sockidcounter++;
    socketinfomap[newsockid].addr = buf;
    socketinfomap[newsockid].port = ntohs(addr.sin6_port);
    getsockname(newfd, (struct sockaddr*)&addr, &addrLen);
    inet_ntop(AF_INET6, &addr.sin6_addr, buf, INET6_ADDRSTRLEN);
    socketinfomap[newsockid].localaddr = buf;
    socketinfomap[newsockid].localport = ntohs(addr.sin6_port);
  }
  socketinfomap[newsockid].fd = newfd;
  socketinfomap[newsockid].addrfam = socketinfo.addrfam;
  socketinfomap[newsockid].id = newsockid;
  socketinfomap[newsockid].type = SocketType::TCP_PLAIN_LISTEN;
  fcntl(newfd, F_SETFL, O_NONBLOCK);
  if (!workmanager.dispatchEventNew(socketinfo.receiver, socketinfo.id,
      newsockid))
  {
    autoPause(socketinfo);
  }
}

void IOManager::run() {
  SSLManager::init();
  initialized.post();
  std::list<PollEvent> events;
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  while (true) {
    socketinfomaplock.unlock();
    polling.wait(events);
    socketinfomaplock.lock();
    for (auto& polliter : events) {
      PollEventType pollevent = polliter.type;
      int sockid = polliter.userdata;
      auto it = socketinfomap.find(sockid);
      if (it == socketinfomap.end()) {
          continue;
      }
      SocketInfo& socketinfo = it->second;
      switch (socketinfo.type) {
        case SocketType::EXTERNAL: // external
          handleExternalIn(socketinfo);
          break;
        case SocketType::TCP_CONNECTING:
          if (pollevent == PollEventType::OUT) {
            handleTCPConnectingOut(socketinfo);
          }
          break;
        case SocketType::TCP_PLAIN: // tcp plain
        case SocketType::TCP_PLAIN_LISTEN:
          if (pollevent == PollEventType::IN) { // incoming data
            handleTCPPlainIn(socketinfo);
          }
          else if (pollevent == PollEventType::OUT) {
            handleTCPPlainOut(socketinfo);
          }
          break;
        case SocketType::TCP_SSL_NEG_CONNECT: // tcp ssl redo connect
        case SocketType::TCP_SSL_NEG_ACCEPT:  // tcp ssl accept
          if (pollevent == PollEventType::OUT) {
            handleTCPSSLNegotiationOut(socketinfo);
          }
          break;
        case SocketType::TCP_SSL_NEG_REDO_CONNECT: // tcp ssl redo connect
        case SocketType::TCP_SSL_NEG_REDO_ACCEPT:  // tcp ssl accept
          if (pollevent == PollEventType::IN) { // incoming data
            handleTCPSSLNegotiationIn(socketinfo);
          }
          break;
        case SocketType::TCP_SSL: // tcp ssl
          if (pollevent == PollEventType::IN) { // incoming data
            handleTCPSSLIn(socketinfo);
          }
          else if (pollevent == PollEventType::OUT) {
            handleTCPSSLOut(socketinfo);
          }
          break;
        case SocketType::UDP: // udp
          if (pollevent == PollEventType::IN) { // incoming data
            handleUDPIn(socketinfo);
          }
          break;
        case SocketType::TCP_SERVER:          // tcp server
        case SocketType::TCP_SERVER_EXTERNAL: // also tcp server
          if (pollevent == PollEventType::IN) { // incoming connection
            handleTCPServerIn(socketinfo);
          }
          break;
        case SocketType::STOP: // stop IO thread
          SSLManager::cleanupThread();
          return;
        case SocketType::TCP_RESOLVING:
        case SocketType::UNUSED:
          assert(false);
          break;
      }
    }
  }
}

std::list<std::pair<std::string, std::string>> IOManager::listInterfaces(bool ipv4, bool ipv6) {
  std::list<std::pair<std::string, std::string>> addrs;
  struct ifaddrs *ifaddr, *ifa;
  int s;
  char host[NI_MAXHOST];
  if (getifaddrs(&ifaddr) == -1) {
    getLogger()->log("IOManager", std::string("Failed to list network interfaces: ") + util::getStrError(errno), LogLevel::ERROR);
    return addrs;
  }
  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr) {
      continue;
    }
    int family = ifa->ifa_addr->sa_family;
    if (family == AF_INET && ipv4) {
      s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
    }
    else if (family == AF_INET6 && ipv6) {
      s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
      if (std::string(host).compare(0, 4, "fe80") == 0) { // link-local address
        continue;
      }
    }
    else {
      continue;
    }
    if (s != 0) {
      getLogger()->log("IOManager", std::string("getnameinfo() failed: ") + gai_strerror(s), LogLevel::ERROR);
      continue;
    }
    addrs.emplace_back(ifa->ifa_name, host);
  }
  freeifaddrs(ifaddr);
  return addrs;
}

std::string IOManager::getBindInterface() const {
  return bindinterface;
}

std::string IOManager::getBindAddress() const {
  return bindaddress;
}

void IOManager::setBindInterface(const std::string& interface) {
  if (interface.empty()) {
    if (hasbindinterface) {
      hasbindinterface = false;
      getLogger()->log("IOManager", "Bind network interface removed", LogLevel::INFO);
    }
    return;
  }
  if (!getInterfaceAddress(interface).success) {
    getLogger()->log("IOManager", "Failed to find interface address for: " + interface, LogLevel::ERROR);
    return;
  }
  if (hasbindinterface == false || bindinterface != interface) {
    bindinterface = interface;
    hasbindinterface = true;
    getLogger()->log("IOManager", "Bind network interface set to: " + interface, LogLevel::INFO);
  }
}

void IOManager::setBindAddress(const std::string& address) {
  if (address.empty()) {
    if (hasbindaddress) {
      hasbindaddress = false;
      getLogger()->log("IOManager", "Bind IP address removed", LogLevel::INFO);
    }
    return;
  }
  if (!hasbindaddress && bindaddress != address) {
    bindaddress = address;
    hasbindaddress = true;
    getLogger()->log("IOManager", "Bind IP address set to: " + address, LogLevel::INFO);
  }
}

bool IOManager::hasBindInterface() const {
  return hasbindinterface;
}

bool IOManager::hasBindAddress() const {
  return hasbindaddress;
}

StringResult IOManager::getInterfaceAddress(const std::string& interface) const {
  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
  if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
    close(fd);
    return StringResultError("No IPv4 address found for " + interface);
  }
  close(fd);
  char buf[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr, buf, INET_ADDRSTRLEN);
  return buf;
}

StringResult IOManager::getInterfaceAddress6(const std::string& interface) const {
  struct ifaddrs *ifaddr, *ifa;
  int s;
  char host[NI_MAXHOST];
  if (getifaddrs(&ifaddr) == -1) {
    getLogger()->log("IOManager", std::string("Failed to list network interfaces: ") + util::getStrError(errno), LogLevel::ERROR);
    return StringResult();
  }
  for (ifa = ifaddr; ifa != nullptr && ifa->ifa_addr != nullptr; ifa = ifa->ifa_next) {
    if (interface == ifa->ifa_name && ifa->ifa_addr->sa_family == AF_INET6) {
      s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
      if (std::string(host).compare(0, 4, "fe80") == 0) { // link-local address
        continue;
      }
      if (s != 0) {
        getLogger()->log("IOManager", std::string("getnameinfo() failed: ") + gai_strerror(s), LogLevel::ERROR);
        continue;
      }
      freeifaddrs(ifaddr);
      return host;
    }
  }
  freeifaddrs(ifaddr);
  return StringResultError("No suitable IPv6 address found for " + interface);
}

std::string IOManager::compactIPv6Address(const std::string& address) {
  char buf[INET6_ADDRSTRLEN];
  struct in6_addr addr;
  if (!inet_pton(AF_INET6, address.c_str(), &addr)) {
    return address;
  }
  if (!inet_ntop(AF_INET6, &addr, buf, INET6_ADDRSTRLEN)) {
    return address;
  }
  return buf;
}

StringResult IOManager::getInterfaceName(const std::string& address) const {
  struct ifaddrs *ifaddr, *ifa;
  int s;
  char host[NI_MAXHOST];
  if (getifaddrs(&ifaddr) == -1) {
    std::string err = "Failed to list network interfaces: " + util::getStrError(errno);
    getLogger()->log("IOManager", err, LogLevel::ERROR);
    return StringResultError(err);
  }
  for (ifa = ifaddr; ifa != nullptr && ifa->ifa_addr != nullptr; ifa = ifa->ifa_next) {
    int family = ifa->ifa_addr->sa_family;
    if (family == AF_INET) {
      s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
    }
    else if (family == AF_INET6) {
      s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in6), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
    }
    else {
      continue;
    }
    if (s != 0) {
      getLogger()->log("IOManager", std::string("getnameinfo() failed: ") + gai_strerror(s), LogLevel::ERROR);
      continue;
    }
    if (host == address) {
      freeifaddrs(ifaddr);
      return ifa->ifa_name;
    }
  }
  freeifaddrs(ifaddr);
  return StringResultError("No matching interface found");
}

StringResult IOManager::getAddressToBind(const AddressFamily addrfam, const SocketType socktype) {
  std::string bindto;
  if (hasBindAddress()) {
    bindto = bindaddress;
  }
  else if (hasBindInterface()) {
    struct addrinfo request;
    memset(&request, 0, sizeof(request));
    request.ai_family = (addrfam == AddressFamily::IPV4 ? AF_INET : AF_INET6);
    request.ai_socktype = (socktype != SocketType::UDP ? SOCK_STREAM : SOCK_DGRAM);
    StringResult interfaceaddress = (addrfam == AddressFamily::IPV4) ? getInterfaceAddress(getBindInterface()) :
                                                                       getInterfaceAddress6(getBindInterface());
    return interfaceaddress;
  }
  return bindto;
}

void IOManager::workerReady() {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  for (auto& sockid : autopaused) {
    auto siit = socketinfomap.find(sockid);
    if (siit == socketinfomap.end()) {
      continue;
    }
    if (manuallypaused.find(sockid) == manuallypaused.end()) {
      siit->second.paused = false;
      if (siit->second.receiver == nullptr) {
        // if no EventReceiver has been registered for the socket yet
        continue;
      }
      if (siit->second.direction == Direction::IN) {
        polling.addFDIn(siit->second.fd, sockid);
      }
      else {
        polling.addFDOut(siit->second.fd, sockid);
      }
    }
  }
  autopaused.clear();
}

void IOManager::autoPause(SocketInfo& socketinfo) {
  autopaused.insert(socketinfo.id);
  socketinfo.paused = true;
  polling.removeFD(socketinfo.fd);
}

void IOManager::pause(int sockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    return;
  }
  if (manuallypaused.find(sockid) == manuallypaused.end()) {
    manuallypaused.insert(sockid);
    it->second.paused = true;
    polling.removeFD(it->second.fd);
  }
}

void IOManager::resume(int sockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    return;
  }
  manuallypaused.erase(sockid);
  if (autopaused.find(sockid) == autopaused.end()) {
    it->second.paused = false;
    if (it->second.direction == Direction::IN) {
      polling.addFDIn(it->second.fd, sockid);
    }
    else {
      polling.addFDOut(it->second.fd, sockid);
    }
  }
}

void IOManager::setLinger(int sockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    return;
  }
  struct linger ling;
  ling.l_onoff = 1;
  ling.l_linger = 0;
  setsockopt(it->second.fd, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling));
}

int IOManager::storeSession(int sockid) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = socketinfomap.find(sockid);
  if (it == socketinfomap.end() || !it->second.ssl) {
    return -1;
  }
  SSL_SESSION* session = SSL_get1_session(it->second.ssl);
  int sessionkey = sessionkeycounter++;
  sessions[sessionkey] = session;
  return sessionkey;
}

void IOManager::clearSession(int sessionkey) {
  std::lock_guard<std::mutex> lock(socketinfomaplock);
  auto it = sessions.find(sessionkey);
  if (it == sessions.end()) {
    return;
  }
  SSL_SESSION_free(it->second);
  sessions.erase(it);
}

void IOManager::clearReusedSession(int sockid) {
  int sessionkey = -1;
  {
    std::lock_guard<std::mutex> lock(socketinfomaplock);
    auto it = socketinfomap.find(sockid);
    if (it == socketinfomap.end() || it->second.sessionkey == -1) {
      return;
    }
    sessionkey = it->second.sessionkey;
  }
  clearSession(sessionkey);
}

} // namespace Core
