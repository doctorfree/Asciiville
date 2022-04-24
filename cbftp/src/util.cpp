#include "util.h"

#include <sstream>
#include <ctime>
#include <csignal>
#include <cctype>

namespace util {

Result::Result() : success(true) {

}

Result::Result(bool success, const std::string& error) :
    success(success), error(error)
{

}

int getSizeGranularity();
std::vector<unsigned long long int> getPowers();

static unsigned int sizegranularity = getSizeGranularity();
static std::vector<unsigned long long int> powers = getPowers();

std::string trim(const std::string & trimtarget) {
  int spos = 0;
  int epos = trimtarget.length() - 1;
  while (spos < epos && (trimtarget[spos] == ' ' || trimtarget[spos] == '\r' || trimtarget[spos] == '\n')) {
    ++spos;
  }
  while (epos >= spos && (trimtarget[epos] == ' ' || trimtarget[epos] == '\r' || trimtarget[epos] == '\n')) {
    --epos;
  }
  if (epos >= spos) {
    return trimtarget.substr(spos, epos - spos + 1);
  }
  return "";
}

std::list<std::string> trim(const std::list<std::string> & in) {
  if (!in.empty() && !in.front().empty() && !in.back().empty()) {
    return in;
  }
  std::list<std::string> out = in;
  if (!out.empty() && out.front().empty()) {
    out.pop_front();
  }
  if (!out.empty() && out.back().empty()) {
    out.pop_back();
  }
  return out;
}

std::vector<std::string> trim(const std::vector<std::string> & in) {
  if (!in.empty() && !in.front().empty() && !in.back().empty()) {
    return in;
  }
  std::vector<std::string> out = in;
  if (!out.empty() && out.front().empty()) {
    out.erase(out.begin());
  }
  if (!out.empty() && out.back().empty()) {
    out.pop_back();
  }
  return out;
}

std::string simpleTimeFormat(int seconds) {
  std::string time;
  if (seconds >= 86400) {
    int days = seconds / 86400;
    time = std::to_string(days) + "d";
    seconds = seconds % 86400;
  }
  if (seconds >= 3600) {
    int hours = seconds / 3600;
    time += std::to_string(hours) + "h";
    seconds = seconds % 3600;
  }
  if (seconds >= 60) {
    int minutes = seconds / 60;
    time += std::to_string(minutes) + "m";
    seconds = seconds % 60;
  }
  if (seconds || !time.length()) {
    time += std::to_string(seconds) + "s";
  }
  return time;
}

std::string ctimeLog() {
  time_t rawtime = time(NULL);
  char timebuf[26];
  ctime_r(&rawtime, timebuf);
  return std::string(timebuf + 11, 8);
}

std::string & debugString(const char * s) {
    return *(new std::string(s));
}

std::string parseSize(unsigned long long int size) {
  int iprefix;
  for (iprefix = 0; iprefix < 6 && size / powers[iprefix] >= 1000; iprefix++);
  unsigned long long int currentpower = powers[iprefix];
  std::string result;
  int whole = size / currentpower;
  if (iprefix == 0) {
    result = std::to_string(whole) + " B";
  }
  else {
    unsigned long long int decim = ((size % currentpower) * sizegranularity) / currentpower + 5;
    if (decim >= sizegranularity) {
      whole++;
      decim = 0;
    }
    std::string decimstr = std::to_string(decim);
    while (decimstr.length() <= SIZEDECIMALS) {
      decimstr = "0" + decimstr;
    }
    result = std::to_string(whole) + "." + decimstr.substr(0, SIZEDECIMALS) + " ";
    switch (iprefix) {
      case 1:
        result.append("kB");
        break;
      case 2:
        result.append("MB");
        break;
      case 3:
        result.append("GB");
        break;
      case 4:
        result.append("TB");
        break;
      case 5:
        result.append("PB");
        break;
      case 6:
        result.append("EB");
        break;
    }
  }
  return result;
}

int getSizeGranularity() {
  int gran = 1;
  for (int i = 0; i <= SIZEDECIMALS; i++) {
    gran *= 10;
  }
  return gran;
}

std::vector<unsigned long long int> getPowers() {
  std::vector<unsigned long long int> vec;
  vec.reserve(7);
  unsigned long long int pow = 1;
  for (int i = 0; i < 7; i++) {
    vec.push_back(pow);
    pow *= 1024;
  }
  return vec;
}

std::string getGroupNameFromRelease(const std::string & release) {
  size_t splitpos = release.rfind("-");
  if (splitpos != std::string::npos) {
    std::string group = release.substr(splitpos + 1);
    size_t inttag = util::toLower(group).rfind("_int");
    if (inttag != std::string::npos) {
      group = group.substr(0, inttag);
    }
    return group;
  }
  else {
    return "";
  }
}

std::string toLower(const std::string & in) {
  std::string ret = in;
  for (unsigned int i = 0; i < in.length(); i++) {
    ret[i] = tolower(ret[i]);
  }
  return ret;
}

int wildcmp(const char *wild, const char *string) {
  const char *cp = NULL, *mp = NULL;
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
  const char *cp = NULL, *mp = NULL;
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

std::list<std::string> split(const std::string& in, const std::string& sep) {
  std::list<std::string> out;
  size_t start = 0;
  size_t end;
  size_t seplength = sep.length();
  while ((end = in.find(sep, start)) != std::string::npos) {
    out.push_back(in.substr(start, end - start));
    start = end + seplength;
  }
  out.push_back(in.substr(start));
  return out;
}

std::vector<std::string> splitVec(const std::string& in, const std::string& sep) {
  std::vector<std::string> out;
  size_t start = 0;
  size_t end;
  size_t seplength = sep.length();
  while ((end = in.find(sep, start)) != std::string::npos) {
    out.push_back(in.substr(start, end - start));
    start = end + seplength;
  }
  out.push_back(in.substr(start));
  return out;
}

std::string join(const std::list<std::string>& in, const std::string& sep) {
  std::string out;
  for (std::list<std::string>::const_iterator it = in.begin(); it != in.end(); it++) {
    out += *it + sep;
  }
  return out.substr(0, out.length() - sep.length());
}

std::string join(const std::vector<std::string>& in, const std::string& sep) {
  std::string out;
  for (std::vector<std::string>::const_iterator it = in.begin(); it != in.end(); it++) {
    out += *it + sep;
  }
  return out.substr(0, out.length() - sep.length());
}

std::string join(const std::set<std::string>& in, const std::string& sep) {
  std::string out;
  for (std::set<std::string>::const_iterator it = in.begin(); it != in.end(); it++) {
    out += *it + sep;
  }
  return out.substr(0, out.length() - sep.length());
}

int chrstrfind(const char * buf, unsigned int buflen, const char * pattern, unsigned int patternlen) {
  unsigned int matchedchars = 0;
  for (unsigned int i = 0; i < buflen;) {
    if (buf[i + matchedchars] == pattern[matchedchars]) {
      ++matchedchars;
      if (matchedchars == patternlen) {
        return i;
      }
    }
    else {
      if (matchedchars) {
        matchedchars = 0;
      }
      ++i;
    }
  }
  return -1;
}

int chrfind(const char * buf, unsigned int buflen, char target) {
  for (unsigned int i = 0; i < buflen; i++) {
    if (buf[i] == target) {
      return i;
    }
  }
  return -1;
}

bool eightCharUserCompare(const std::string & a, const std::string & b) {
  size_t alen = a.length();
  if (alen > 8) {
    size_t blen = b.length();
    if (blen > 8) {
      return a == b;
    }
  }
  return a.compare(0, 8, b, 0, 8) == 0;
}

std::regex regexParse(const std::string & pattern) {
  if (!pattern.compare(0, 4, "(?i)")) {
    return std::regex(pattern.substr(4), std::regex::icase | std::regex::optimize);
  }
  return std::regex(pattern, std::regex::optimize);
}

bool naturalComparator::operator()(const std::string& a, const std::string& b) const {
  size_t i = 0;
  size_t j = 0;
  while (i < a.length() && j < b.length()) {
    if (isdigit(a[i]) && isdigit(b[j])) {
      size_t adigitstart = i;
      size_t bdigitstart = j;
      while (i < a.length() && isdigit(a[i]) && i - adigitstart < 18) {
        ++i;
      }
      while ( j < b.length() && isdigit(b[j]) && j - bdigitstart < 18) {
        ++j;
      }
      long long int anum = std::stoll(a.substr(adigitstart, i - adigitstart));
      long long int bnum = std::stoll(b.substr(bdigitstart, j - bdigitstart));
      if (anum < bnum) {
        return true;
      }
      if (anum > bnum) {
        return false;
      }
    }
    else {
      char ca = tolower(a[i]);
      char cb = tolower(b[j]);
      if (ca < cb) {
        return true;
      }
      if (ca > cb) {
        return false;
      }
      ++i;
      ++j;
    }
  }
  return a.length() < b.length();
}

}
