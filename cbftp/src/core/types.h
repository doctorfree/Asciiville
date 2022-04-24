#pragma once

#include <string>
#include <vector>

namespace Core {

typedef std::vector<unsigned char> BinaryData;

enum class AddressFamily
{
  NONE,
  IPV4,
  IPV6,
  IPV4_IPV6,
};

struct StringResult {
  StringResult();
  StringResult(const char* result);
  StringResult(const std::string& result);
  bool success;
  std::string result;
  std::string error;
};

struct StringResultError : public StringResult {
  StringResultError(const std::string& error);
};

} // namespace Core
