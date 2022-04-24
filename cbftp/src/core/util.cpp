#include "util.h"

#include <csignal>
#include <cstring>
#include <sstream>

namespace Core {
namespace util {


int wildcmp(const char* wild, const char* string) {
  const char* cp = nullptr;
  const char* mp = nullptr;
  while ((*string) && (*wild != '*')) {
    if (*wild != *string && *wild != '?' &&
        !(*wild >= 65 && *wild <= 90 && *wild + 32 == *string) &&
        !(*wild >= 97 && *wild <= 122 && *wild - 32 == *string)) {
      return 0;
    }
    wild++;
    string++;
  }
  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if (*wild == *string || *wild == '?' ||
    (*wild >= 65 && *wild <= 90 && *wild + 32 == *string) ||
    (*wild >= 97 && *wild <= 122 && *wild - 32 == *string)) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }
  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

int wildcmpCase(const char *wild, const char *string) {
  const char* cp = nullptr;
  const char* mp = nullptr;
  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }
  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }
  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

std::string getStrError(int error) {
  char buf[ERR_BUF_SIZE];
  // some casting for XSI compatibility
  char* ret = reinterpret_cast<char*>(strerror_r(error, buf, sizeof(buf)));
  if (ret) {
    if (reinterpret_cast<long long int>(ret) == -1) {
      return getStrError(errno);
    }
    return ret;
  }
  return buf;
}

} // namespace util
} // namespace Core
