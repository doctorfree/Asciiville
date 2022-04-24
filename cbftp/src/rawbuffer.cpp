#include "rawbuffer.h"

#include <cassert>

#include "uibase.h"
#include "globalcontext.h"
#include "util.h"
#include "rawbuffercallback.h"
#include "logmanager.h"

RawBuffer::RawBuffer(const std::string& site, const std::string& id) :
  latestp(0),
  latestpcopy(0),
  maxlength(global->getLogManager()->getMaxRawbufLines()),
  bookmarklines(0),
  site(site),
  id(id),
  inprogress(false),
  uiwatching(false),
  threads(true),
  eventlog(false),
  callback(nullptr),
  hasregexfilter(false)
{
  writeLine("Log window initialized. Site: " + site + " Thread id: " + id);
}

RawBuffer::RawBuffer(const std::string& site) :
  latestp(0),
  latestpcopy(0),
  maxlength(global->getLogManager()->getMaxRawbufLines()),
  bookmarklines(0),
  site(site),
  inprogress(false),
  uiwatching(false),
  threads(false),
  eventlog(false),
  callback(nullptr),
  hasregexfilter(false)
{
  writeLine("Raw command window initialized. Site: " + site);
}

RawBuffer::RawBuffer() :
  latestp(0),
  latestpcopy(0),
  maxlength(1024),
  bookmarklines(0),
  inprogress(false),
  uiwatching(false),
  threads(false),
  eventlog(true),
  callback(nullptr),
  hasregexfilter(false)
{
  writeLine("Event log initialized.");
}

void RawBuffer::setCallback(RawBufferCallback * callback) {
  this->callback = callback;
}

void RawBuffer::unsetCallback() {
  callback = NULL;
}

void RawBuffer::bookmark() {
  bookmarklines = 0;
}

unsigned int RawBuffer::linesSinceBookmark() const {
  return bookmarklines;
}

void RawBuffer::lineFinished() {
  inprogress = false;
  addFilterLine(getLine(0), logfiltered, latestpfiltered);
  if (callback != nullptr) {
    callback->newRawBufferLine(getLine(0));
  }
}
void RawBuffer::write(const std::string & s) {
  write(getTag(), s);
}

void RawBuffer::write(const std::string & tag, const std::string & s) {
  size_t pos = 0;
  size_t len = s.length();
  while (pos < len) {
    size_t split = s.find('\n', pos);
    if (split == pos) {
      lineFinished();
      ++pos;
    }
    else if (split == std::string::npos) {
      addLogText(tag, s.substr(pos));
      pos = len;
    }
    else {
      size_t len = split - pos;
      if (s[split - 1] == '\r') {
        --len;
      }
      addLogText(tag, s.substr(pos, len));
      pos = split;
    }
  }
  if (uiwatching) {
    global->getUIBase()->backendPush();
  }
}

void RawBuffer::addLogText(const std::string& tag, const std::string& text) {
  if (inprogress) {
    log[(latestp > 0 ? latestp : maxlength) - 1].second.append(text);
    return;
  }
  if (log.size() < maxlength) {
    log.emplace_back(tag, text);
  }
  else {
    log[latestp] = std::make_pair(tag, text);
  }
  ++latestp %= maxlength;
  ++bookmarklines;
  inprogress = true;
}

void RawBuffer::writeLine(const std::string & s) {
  writeLine(getTag(), s);
}

void RawBuffer::writeLine(const std::string & tag, const std::string & s) {
  write(tag, s + "\n");
}

void RawBuffer::rename(std::string name) {
  writeLine("Changing site name to: " + name);
  site = name;
}

std::string RawBuffer::getTag() const {
  return "[" + util::ctimeLog() + (eventlog ? "" : " " + site + (threads ? " " + id : "")) + "]";
}

void RawBuffer::setId(int id) {
  this->id = std::to_string(id);
}

const std::pair<std::string, std::string>& RawBuffer::getLineCopy(unsigned int num) const {
  unsigned int size = getCopySize();
  assert(num < size);
  int pos = (num < latestpcopy ? latestpcopy - num - 1 : size + latestpcopy - num - 1);
  return logcopy[pos];
}

const std::pair<std::string, std::string>& RawBuffer::getLine(unsigned int num) const {
  unsigned int size = getSize();
  assert(num < size);
  int pos = (num < latestp ? latestp - num - 1 : size + latestp - num - 1);
  return log[pos];
}

const std::pair<std::string, std::string>& RawBuffer::getFilteredLineCopy(unsigned int num) const {
  unsigned int size = getFilteredCopySize();
  assert(num < size);
  int pos = (num < latestpcopyfiltered ? latestpcopyfiltered - num - 1 : size + latestpcopyfiltered - num - 1);
  return logcopyfiltered[pos];
}

const std::pair<std::string, std::string>& RawBuffer::getFilteredLine(unsigned int num) const {
  unsigned int size = getFilteredSize();
  assert(num < size);
  int pos = (num < latestpfiltered ? latestpfiltered - num - 1 : size + latestpfiltered - num - 1);
  return logfiltered[pos];
}

unsigned int RawBuffer::getSize() const {
  return log.size();
}

unsigned int RawBuffer::getFilteredSize() const {
  return logfiltered.size();
}

unsigned int RawBuffer::getCopySize() const {
  return logcopy.size();
}

unsigned int RawBuffer::getFilteredCopySize() const {
  return logcopyfiltered.size();
}

void RawBuffer::freezeCopy() {
  logcopy = log;
  if (isFiltered()) {
    logcopyfiltered = logfiltered;
  }
  latestpcopy = latestp;
}

void RawBuffer::setUiWatching(bool watching) {
  uiwatching = watching;
}

void RawBuffer::clear() {
  log.clear();
  logcopy.clear();
  unsetFilters();
  latestp = 0;
  latestpcopy = 0;
  inprogress = false;
}

bool RawBuffer::isFiltered() const {
  return hasWildcardFilters() || hasRegexFilter();
}

bool RawBuffer::hasWildcardFilters() const {
  return wildcardfilters.size();
}

bool RawBuffer::hasRegexFilter() const {
  return hasregexfilter;
}

std::list<std::string> RawBuffer::getWildcardFilters() const {
  return wildcardfilters;
}

std::regex RawBuffer::getRegexFilter() const {
  return regexfilter;
}

void RawBuffer::setWildcardFilters(const std::list<std::string> & filters) {
  this->wildcardfilters = filters;
  hasregexfilter = false;
  applyFilters();
}

void RawBuffer::setRegexFilter(const std::regex & regexfilter) {
  this->regexfilter = regexfilter;
  hasregexfilter = true;
  wildcardfilters.clear();
  applyFilters();
}

void RawBuffer::unsetFilters() {
  wildcardfilters.clear();
  hasregexfilter = false;
  logfiltered.clear();
  logcopyfiltered.clear();
}

void RawBuffer::applyFilters() {
  applyFilters(false);
  applyFilters(true);
}

void RawBuffer::applyFilters(bool copy) {
  const std::vector<std::pair<std::string, std::string>>& source = copy ? logcopy : log;
  std::vector<std::pair<std::string, std::string>>& target = copy ? logcopyfiltered : logfiltered;
  target.clear();
  unsigned int& latest = copy ? latestpcopyfiltered : latestpfiltered;
  latest = 0;
  for (unsigned int i = 0; i < source.size(); ++i) {
    unsigned int pos = source.size() - i - 1;
    const std::pair<std::string, std::string>& line = copy ? getLineCopy(pos) : getLine(pos);
    addFilterLine(line, target, latest);
  }
}

void RawBuffer::addFilterLine(const std::pair<std::string, std::string>& line, std::vector<std::pair<std::string, std::string>>& target, unsigned int& latest) {
  if (!wildcardfilters.empty()) {
    bool negativepass = true;
    bool foundpositive = false;
    bool positivematch = false;
    for (std::list<std::string>::const_iterator it = wildcardfilters.begin(); it != wildcardfilters.end(); it++) {
      const std::string & filter = *it;
      if (filter[0] == '!') {
        if (util::wildcmp(filter.substr(1).c_str(), line.second.c_str())) {
          negativepass = false;
          break;
        }
      }
      else {
        foundpositive = true;
        if (util::wildcmp(filter.c_str(), line.second.c_str())) {
          positivematch = true;
        }
      }
    }
    if (!negativepass || (foundpositive && !positivematch)) {
      return;
    }
  }
  else if (hasregexfilter && !std::regex_match(line.second, regexfilter)) {
    return;
  }
  if (target.size() < maxlength) {
    target.emplace_back(line.first, line.second);
  }
  else {
    target[latest] = line;
  }
  ++latest %= maxlength;
}
