#pragma once

#include <string>

#include "core/types.h"

struct Address {
  Address();
  Address(const std::string& host, int port, Core::AddressFamily addrfam = Core::AddressFamily::IPV4_IPV6, bool brackets = false);
  std::string toString(bool includeaddrfam = true) const;
  bool operator==(const Address& other) const;
  Core::AddressFamily addrfam;
  bool brackets;
  std::string host;
  unsigned short port;
};
