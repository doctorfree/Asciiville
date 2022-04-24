#pragma once

#include <string>

class Addr;

class FTPConnectOwner {
public:
  virtual void ftpConnectInfo(int id, const std::string &) = 0;
  virtual void ftpConnectSuccess(int id, const Address& addr) = 0;
  virtual void ftpConnectFail(int id) = 0;
  virtual ~FTPConnectOwner() {
  }
};
