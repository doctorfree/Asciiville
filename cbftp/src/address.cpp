#include "address.h"

Address::Address() : addrfam(Core::AddressFamily::IPV4_IPV6), brackets(false), port(21) {

}

Address::Address(const std::string& host, int port, Core::AddressFamily addrfam, bool brackets) : addrfam(addrfam), brackets(brackets), host(host), port(port) {

}

std::string Address::toString(bool includeaddrfam) const {
  std::string addrstr;
  if (includeaddrfam) {
    if (addrfam == Core::AddressFamily::IPV4) {
      addrstr += "(4)";
    }
    else if (addrfam == Core::AddressFamily::IPV6) {
      addrstr += "(6)";
    }
  }
  if (brackets) {
    addrstr += "[";
  }
  addrstr += host;
  if (brackets) {
    addrstr += "]";
  }
  if (port != 21) {
    addrstr += ":" + std::to_string(port);
  }
  return addrstr;
}

bool Address::operator==(const Address& other) const {
  return addrfam == other.addrfam && brackets == other.brackets && host == other.host && port == other.port;
}
