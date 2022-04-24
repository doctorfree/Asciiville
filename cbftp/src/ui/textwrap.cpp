#include "textwrap.h"

std::list<std::string> TextWrap::wrap(const std::string & text, unsigned int linelen) {
  std::list<std::string> out;
  size_t linestart = 0;
  size_t lastspace = 0;
  for (size_t i = 0; i < text.length(); i++) {
    if (text[i] != ' ' && i && i - linestart + 1 <= linelen) {
      continue;
    }
    if (i - linestart + 1 > linelen) {
      out.emplace_back(text.substr(linestart, lastspace - linestart));
      i = lastspace;
      lastspace++;
      linestart = lastspace;
    }
    else if (i > linestart && text[i - 1] == '.') {
      out.emplace_back(text.substr(linestart, i - linestart));
      lastspace = i + 1;
      linestart = lastspace;
    }
    else {
      lastspace = i;
    }
  }
  if (linestart < text.length()) {
    out.emplace_back(text.substr(linestart));
  }
  return out;
}
