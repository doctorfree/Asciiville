#pragma once

enum class TransferProtocol {
  IPV4_ONLY = 0,
  PREFER_IPV4 = 1,
  PREFER_IPV6 = 2,
  IPV6_ONLY = 3
};

bool transferProtocolCombinationPossible(TransferProtocol sprot, TransferProtocol dprot);
bool useIPv6(TransferProtocol sprot, TransferProtocol dprot);
