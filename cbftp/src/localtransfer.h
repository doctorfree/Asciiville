#pragma once

#include <string>
#include <fstream>

#include "eventreceiverproxyintermediate.h"
#include "path.h"

#define CHUNK 524288

class TransferMonitor;
class FTPConn;

class LocalTransfer : public EventReceiverProxyIntermediate {
public:
  LocalTransfer();
  bool active() const;
  bool openFile(bool);
  int getPort() const;
  virtual unsigned long long int size() const = 0;
  FTPConn * getConn() const;
protected:
  void activate();
  void deactivate();
  bool ssl;
  bool inmemory;
  bool passivemode;
  int port;
  TransferMonitor * tm;
  bool fileopened;
  FTPConn * ftpconn;
  std::fstream filestream;
  Path path;
  std::string filename;
  char * buf;
  unsigned int buflen;
  unsigned int bufpos;
  bool timeoutticker;
private:
  void FDInterNew(int sockid, int newsockid) override;
  void FDInterInfo(int sockid, const std::string& info) override;
  void tick(int) override;
  bool inuse;
};
