#include "buildinfo.h"

#ifndef BOXTAG
#define BOXTAG ""
#endif

#ifndef VERSION
#define VERSION "unknown"
#endif

#ifndef BUILDTIME
#define BUILDTIME "unknown"
#endif

std::string BuildInfo::version() {
  return VERSION;
}

std::string BuildInfo::compileTime() {
  return BUILDTIME;
}

std::string BuildInfo::tag() {
  return BOXTAG;
}
