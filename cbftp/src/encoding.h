#pragma once

#include <string>

#include "core/types.h"

namespace encoding {

enum Encoding {
  ENCODING_CP437,
  ENCODING_CP437_DOUBLE,
  ENCODING_ISO88591,
  ENCODING_UTF8
};

unsigned int cp437toUnicode(unsigned char);
std::basic_string<unsigned int> cp437toUnicode(const std::string &);
std::basic_string<unsigned int> doublecp437toUnicode(const std::string &);
std::basic_string<unsigned int> toUnicode(const std::string &);
std::basic_string<unsigned int> utf8toUnicode(const std::string &);
Encoding guessEncoding(const Core::BinaryData& data);

}
