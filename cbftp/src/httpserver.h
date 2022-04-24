#pragma once

#include <list>

#include "core/eventreceiver.h"

#define DEFAULT_API_PORT 55477

class HTTPConn;

class HTTPServer : private Core::EventReceiver {
public:
  HTTPServer();
  ~HTTPServer();
  bool getEnabled() const;
  int getPort() const;
  void setEnabled(bool enabled);
  void setPort(int port);
private:
  void bindListen();
  void close();
  void FDNew(int sockid, int newsockid) override;
  void FDFail(int sockid, const std::string& error) override;
  bool enabled;
  int port;
  int sockid4;
  int sockid6;
  std::list<HTTPConn> conns;
};
