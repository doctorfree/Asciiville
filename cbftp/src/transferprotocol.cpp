#include "transferprotocol.h"

bool transferProtocolCombinationPossible(TransferProtocol sprot, TransferProtocol dprot) {
  return !((sprot == TransferProtocol::IPV4_ONLY && dprot == TransferProtocol::IPV6_ONLY) ||
           (sprot == TransferProtocol::IPV6_ONLY && dprot == TransferProtocol::IPV4_ONLY));
}

bool useIPv6(TransferProtocol sprot, TransferProtocol dprot) {
  return ((sprot != TransferProtocol::IPV4_ONLY && dprot != TransferProtocol::IPV4_ONLY) &&
          !(sprot == TransferProtocol::PREFER_IPV4 && dprot == TransferProtocol::PREFER_IPV4));
}
