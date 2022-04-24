#pragma once

#include <string>

#include "core/types.h"

class Crypto {
public:
  static void encrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata);
  static void decrypt(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata);
  static void decryptOld(const Core::BinaryData& indata, const Core::BinaryData& pass, Core::BinaryData& outdata);
  static void sha256(const Core::BinaryData& indata, Core::BinaryData& outdata);
  static void base64Encode(const Core::BinaryData& indata, Core::BinaryData& outdata);
  static void base64Decode(const Core::BinaryData& indata, Core::BinaryData& outdata);
  static bool isMostlyASCII(const Core::BinaryData& data);
  static std::string toHex(const Core::BinaryData& indata);
  static void fromHex(const std::string& indata, Core::BinaryData& outdata);
};
