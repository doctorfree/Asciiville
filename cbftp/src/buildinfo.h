#pragma once

#include <string>

class BuildInfo {
public:
  static std::string version();
  static std::string compileTime();
  static std::string tag();
};
