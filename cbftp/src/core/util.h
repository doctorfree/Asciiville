#pragma once

#include <string>

namespace Core {
namespace util {

const int ERR_BUF_SIZE = 256;

/* Simple wildcard compare where * and ? are supported (not case sensitive) */
int wildcmp(const char* wild, const char* string);

/* Simple wildcard compare where * and ? are supported */
int wildcmpCase(const char* wild, const char* string);

/* convenience for thread-safe XSI-compatible strerror usage */
std::string getStrError(int error);

} // namespace util
} // namespace Core
