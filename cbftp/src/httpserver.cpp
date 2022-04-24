#include "httpserver.h"

#include "core/iomanager.h"
#include "core/types.h"
#include "eventlog.h"
#include "globalcontext.h"
#include "httpconn.h"
#include "remotecommandhandler.h"

HTTPServer::HTTPServer() : enabled(false), port(DEFAULT_API_PORT), sockid4(-1), sockid6(-1) {

}

HTTPServer::~HTTPServer() {
  close();
}

bool HTTPServer::getEnabled() const {
  return enabled;
}

int HTTPServer::getPort() const {
  return port;
}

void HTTPServer::setEnabled(bool enabled) {
  if (port != -1 && enabled && !this->enabled) {
    bindListen();
  }
  else if (port != -1 && !enabled && this->enabled) {
    close();
  }
  this->enabled = enabled;
}

void HTTPServer::setPort(int port) {
  if (this->port == port) {
    return;
  }
  this->port = port;
  if (port <= 0 || !enabled) {
    return;
  }
  close();
  bindListen();
}

void HTTPServer::bindListen() {
  sockid4 = global->getIOManager()->registerTCPServerSocket(this, port, Core::AddressFamily::IPV4);
  sockid6 = global->getIOManager()->registerTCPServerSocket(this, port, Core::AddressFamily::IPV6);
  global->getEventLog()->log("HTTPServer", "Listening on TCP port " + std::to_string(port));
}

void HTTPServer::close() {
  global->getIOManager()->closeSocket(sockid4);
  global->getIOManager()->closeSocket(sockid6);
  global->getEventLog()->log("HTTPServer", "Closing TCP sockets.");
}

void HTTPServer::FDNew(int sockid, int newsockid) {
  Core::AddressFamily addrfam = (sockid == sockid4) ? Core::AddressFamily::IPV4 : Core::AddressFamily::IPV6;
  for (HTTPConn& conn : conns) {
    if (conn.activate(newsockid, addrfam, true)) {
      return;
    }
  }
  conns.emplace_back();
  conns.back().activate(newsockid, addrfam, true);
}

void HTTPServer::FDFail(int sockid, const std::string& err) {
  global->getEventLog()->log("HTTPServer",
      "Failed to listen on TCP port " + std::to_string(port) + ": " + err, Core::LogLevel::WARNING);
}
