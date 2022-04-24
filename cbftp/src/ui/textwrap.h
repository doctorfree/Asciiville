#pragma once

#include <list>
#include <string>

class TextWrap {
public:
  static std::list<std::string> wrap(const std::string & text, unsigned int linelen);
};
