#pragma once

#include "localtransfer.h"

class Path;

class LocalUpload : public LocalTransfer {
public:
  LocalUpload();
  void engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, const std::string& addr, int port, bool ssl, FTPConn* ftpconn);
  bool engage(TransferMonitor* tm, const Path& path, const std::string& filename, bool ipv6, bool ssl, FTPConn* ftpconn);
  unsigned long long int size() const override;
private:
  void FDInterConnected(int sockid) override;
  void FDInterDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
  void FDInterData(int sockid, char* data, unsigned int len) override;
  void FDSSLSuccess(int sockid, const std::string& cipher) override;
  void FDInterSendComplete(int sockid) override;
  void FDInterListening(int sockid, const Address& addr) override;
  void FDFail(int sockid, const std::string& error) override;
  void init(TransferMonitor* tm, FTPConn* ftpconn, const Path& path, const std::string& filename, bool ssl, bool passivemode, int port = 0);
  void sendChunk();
  unsigned long long int filepos;
};
