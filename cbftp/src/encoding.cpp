#include "encoding.h"

#include <map>

namespace encoding {

std::map<unsigned char, unsigned int> getCP437toUnicodeMap();
static std::map<unsigned char, unsigned int> cp437unicode = getCP437toUnicodeMap();

unsigned int cp437toUnicode(unsigned char c) {
  std::map<unsigned char, unsigned int>::iterator it = cp437unicode.find(c);
  return it != cp437unicode.end() ? it->second : c;
}

unsigned int harddoublecp437toUnicode(unsigned char c) {
  c = cp437toUnicode(c);
  // misconversions due to unrepresentable characters in double cp437
  if (c == 'U') {
    c = 0xDB;
  }
  else if (c == 'Y') {
    c = 0xDD;
  }
  else if (c == '_') {
    c = 0xDE;
  }
  return cp437toUnicode(c);
}

bool certainlyASCII(unsigned int c) {
  // some uncertain chars, may be misconverted in double cp437
  return c != 'U' && c != 'Y' && c != '_' && c != ' ' && c < 0x80;
}

std::basic_string<unsigned int> cp437toUnicode(const std::string & in) {
  std::basic_string<unsigned int> out;
  for (unsigned int i = 0; i < in.length(); i++) {
    out.push_back(cp437toUnicode(in[i]));
  }
  return out;
}

std::basic_string<unsigned int> doublecp437toUnicode(const std::string & in) {
  std::basic_string<unsigned int> out;
  for (unsigned int i = 0; i < in.length(); i++) {
    if ((i > 0 && certainlyASCII(in[i - 1])) || (i < in.length() - 1 && certainlyASCII(in[i + 1]))) {
      out.push_back(in[i]);
    }
    else {
      out.push_back(harddoublecp437toUnicode(in[i]));
    }
  }
  return out;
}

std::basic_string<unsigned int> toUnicode(const std::string & in) {
  std::basic_string<unsigned int> out;
  for (unsigned int i = 0; i < in.length(); i++) {
    out.push_back(static_cast<unsigned char>(in[i]));
  }
  return out;
}

template <typename T>
bool isCurrentPosValidUTF8(const T & in, unsigned int pos,
                           unsigned int & unicodepoint,
                           unsigned int & bytes)
{
  if (pos >= in.size()) {
    return false;
  }
  else if ((in[pos] & 0x80) == 0) {
    unicodepoint = in[pos];
    bytes = 1;
    return true;
  }
  else if ((in[pos] & 0xE0) == 0xC0 && // 2-byte char
      pos + 1 < in.size() &&
      (in[pos + 1] & 0xC0) == 0x80)
  {
    unicodepoint = ((in[pos] & 0x1F) << 6) | (in[pos + 1] & 0x3F);
    bytes = 2;
    return true;
  }
  else if ((in[pos] & 0xF0) == 0xE0 && // 3-byte char
           pos + 2 < in.size() &&
           (in[pos + 1] & 0xC0) == 0x80 &&
           (in[pos + 2] & 0xC0) == 0x80)
  {
    unicodepoint = (((in[pos] & 0xF) << 12) |
                   ((in[pos + 1] & 0x3F) << 6) |
                   (in[pos + 2] & 0x3F));
    bytes = 3;
    return true;
  }
  else if ((in[pos] & 0xF8) == 0xF0 && // 4-byte char
           pos + 3 < in.size() &&
           (in[pos + 1] & 0xC0) == 0x80 &&
           (in[pos + 2] & 0xC0) == 0x80 &&
           (in[pos + 3] & 0xC0) == 0x80)
  {
    unicodepoint = (((in[pos] & 0x7) << 18) |
                   ((in[pos + 1] & 0x3F) << 12) |
                   ((in[pos + 2] & 0x3F) << 6) |
                   (in[pos + 3] & 0x3F));
    bytes = 4;
    return true;
  }
  return false;
}

std::basic_string<unsigned int> utf8toUnicode(const std::string & in) {
  std::basic_string<unsigned int> out;
  for (unsigned int i = 0; i < in.length(); i++) {
    unsigned int unicodepoint;
    unsigned int bytes;
    if (isCurrentPosValidUTF8(in, i, unicodepoint, bytes)) {
      out.push_back(unicodepoint);
      i += bytes - 1;
    }
    else {
      out.push_back('?'); // unknown/broken char
    }
  }
  return out;
}

Encoding guessEncoding(const Core::BinaryData& data) {
  int cp437hitcharsinrow = 0;
  bool cp437hitbefore = false;
  int cp437maxcharsinrow = 0;
  int doublecp437hitcharsinrow = 0;
  bool doublecp437hitbefore = false;
  int doublecp437maxcharsinrow = 0;
  int validmultibyteutf8 = 0;
  int invalidutf8 = 0;
  for (unsigned int i = 0; i < data.size(); i++) {
    if (data[i] == 0xDB || data[i] == 0xDC || data[i] == 0xDD ||
        data[i] == 0xDE || data[i] == 0xDF || data[i] == 0xB0 ||
        data[i] == 0xB1 || data[i] == 0xB2) // most common ANSI drawing chars
    {
      if (cp437hitbefore) {
        ++cp437hitcharsinrow;
      }
      else {
        if (cp437hitcharsinrow > cp437maxcharsinrow) {
          cp437maxcharsinrow = cp437hitcharsinrow;
        }
        cp437hitcharsinrow = 1;
        cp437hitbefore = true;
      }
    }
    else {
      cp437hitbefore = false;
    }
    if (data[i] == 0x9A || data[i] == 0xE1 || data[i] == 0xF8 ||
        data[i] == 0xF1 || data[i] == 0xFD) // signs of double cp437
    {
      if (doublecp437hitbefore) {
        ++doublecp437hitcharsinrow;
      }
      else {
        if (doublecp437hitcharsinrow > doublecp437maxcharsinrow) {
          doublecp437maxcharsinrow = doublecp437hitcharsinrow;
        }
        doublecp437hitcharsinrow = 1;
        doublecp437hitbefore = true;
      }
    }
    else {
      doublecp437hitbefore = false;
    }
    if (data[i] >= 128) {
      unsigned int unicodepoint;
      unsigned int bytes;
      if (isCurrentPosValidUTF8(data, i, unicodepoint, bytes)) {
        ++validmultibyteutf8;
        i += bytes - 1;
      }
      else {
        ++invalidutf8;
      }
    }
  }

  if (validmultibyteutf8 > invalidutf8) {
    return ENCODING_UTF8;
  }
  else if (cp437maxcharsinrow >= 3 && cp437maxcharsinrow >= doublecp437maxcharsinrow) {
    return ENCODING_CP437;
  }
  else if (doublecp437maxcharsinrow >= 3) {
    return ENCODING_CP437_DOUBLE;
  }
  else {
    return ENCODING_ISO88591;
  }
}

std::map<unsigned char, unsigned int> getCP437toUnicodeMap() {
  std::map<unsigned char, unsigned int> map;
  map[0x80] = 0x00C7;
  map[0x81] = 0x00FC;
  map[0x82] = 0x00E9;
  map[0x83] = 0x00E2;
  map[0x84] = 0x00E4;
  map[0x85] = 0x00E0;
  map[0x86] = 0x00E5;
  map[0x87] = 0x00E7;
  map[0x88] = 0x00EA;
  map[0x89] = 0x00EB;
  map[0x8A] = 0x00E8;
  map[0x8B] = 0x00EF;
  map[0x8C] = 0x00EE;
  map[0x8D] = 0x00EC;
  map[0x8E] = 0x00C4;
  map[0x8F] = 0x00C5;
  map[0x90] = 0x00C9;
  map[0x91] = 0x00E6;
  map[0x92] = 0x00C6;
  map[0x93] = 0x00F4;
  map[0x94] = 0x00F6;
  map[0x95] = 0x00F2;
  map[0x96] = 0x00FB;
  map[0x97] = 0x00F9;
  map[0x98] = 0x00FF;
  map[0x99] = 0x00D6;
  map[0x9A] = 0x00DC;
  map[0x9B] = 0x00A2;
  map[0x9C] = 0x00A3;
  map[0x9D] = 0x00A5;
  map[0x9E] = 0x20A7;
  map[0x9F] = 0x0192;
  map[0xA0] = 0x00E1;
  map[0xA1] = 0x00ED;
  map[0xA2] = 0x00F3;
  map[0xA3] = 0x00FA;
  map[0xA4] = 0x00F1;
  map[0xA5] = 0x00D1;
  map[0xA6] = 0x00AA;
  map[0xA7] = 0x00BA;
  map[0xA8] = 0x00BF;
  map[0xA9] = 0x2310;
  map[0xAA] = 0x00AC;
  map[0xAB] = 0x00BD;
  map[0xAC] = 0x00BC;
  map[0xAD] = 0x00A1;
  map[0xAE] = 0x00AB;
  map[0xAF] = 0x00BB;
  map[0xB0] = 0x2591; // light shade
  map[0xB1] = 0x2592; // medium shade
  map[0xB2] = 0x2593; // dark shade
  map[0xB3] = 0x2502;
  map[0xB4] = 0x2524;
  map[0xB5] = 0x2561;
  map[0xB6] = 0x2562;
  map[0xB7] = 0x2556;
  map[0xB8] = 0x2555;
  map[0xB9] = 0x2563;
  map[0xBA] = 0x2551;
  map[0xBB] = 0x2557;
  map[0xBC] = 0x255D;
  map[0xBD] = 0x255C;
  map[0xBE] = 0x255B;
  map[0xBF] = 0x2510;
  map[0xC0] = 0x2514;
  map[0xC1] = 0x2534;
  map[0xC2] = 0x252C;
  map[0xC3] = 0x251C;
  map[0xC4] = 0x2500;
  map[0xC5] = 0x253C;
  map[0xC6] = 0x255E;
  map[0xC7] = 0x255F;
  map[0xC8] = 0x255A;
  map[0xC9] = 0x2554;
  map[0xCA] = 0x2569;
  map[0xCB] = 0x2566;
  map[0xCC] = 0x2560;
  map[0xCD] = 0x2550;
  map[0xCE] = 0x256C;
  map[0xCF] = 0x2567;
  map[0xD0] = 0x2568;
  map[0xD1] = 0x2564;
  map[0xD2] = 0x2565;
  map[0xD3] = 0x2559;
  map[0xD4] = 0x2558;
  map[0xD5] = 0x2552;
  map[0xD6] = 0x2553;
  map[0xD7] = 0x256B;
  map[0xD8] = 0x256A;
  map[0xD9] = 0x2518;
  map[0xDA] = 0x250C;
  map[0xDB] = 0x2588; // full block
  map[0xDC] = 0x2584; // lower half block
  map[0xDD] = 0x258C; // left half block
  map[0xDE] = 0x2590; // right half block
  map[0xDF] = 0x2580; // upper half block
  map[0xE0] = 0x03B1;
  map[0xE1] = 0x00DF;
  map[0xE2] = 0x0393;
  map[0xE3] = 0x03C0;
  map[0xE4] = 0x03A3;
  map[0xE5] = 0x03C3;
  map[0xE6] = 0x00B5;
  map[0xE7] = 0x03C4;
  map[0xE8] = 0x03A6;
  map[0xE9] = 0x0398;
  map[0xEA] = 0x03A9;
  map[0xEB] = 0x03B4;
  map[0xEC] = 0x221E;
  map[0xED] = 0x03C6;
  map[0xEE] = 0x0385;
  map[0xEF] = 0x2229;
  map[0xF0] = 0x2261;
  map[0xF1] = 0x00B1;
  map[0xF2] = 0x2265;
  map[0xF3] = 0x2264;
  map[0xF4] = 0x2320;
  map[0xF5] = 0x2321;
  map[0xF6] = 0x00F7;
  map[0xF7] = 0x2248;
  map[0xF8] = 0x00B0;
  map[0xF9] = 0x2219;
  map[0xFA] = 0x0087;
  map[0xFB] = 0x221A;
  map[0xFC] = 0x207F;
  map[0xFD] = 0x00B2;
  map[0xFE] = 0x25FC; // black medium square
  map[0xFF] = 0x00A0;
  return map;
}

}
