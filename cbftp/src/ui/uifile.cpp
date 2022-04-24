#include "uifile.h"

#include <cctype>

#include "../file.h"
#include "../localfile.h"
#include "../util.h"
#include "../timereference.h"

UIFile::UIFile(File * file) :
  name(file->getName()),
  size(file->getSize()),
  sizerepr(util::parseSize(size)),
  owner(file->getOwner()),
  group(file->getGroup()),

  linktarget(file->getLinkTarget()),

  directory(file->isDirectory()),
  softlink(file->isLink()),
  softselected(false),
  hardselected(false)
{
  parseTimeStamp(file->getLastModified());
}

UIFile::UIFile(const LocalFile & file) :
  name(file.getName()),
  size(file.getSize()),
  sizerepr(util::parseSize(size)),
  owner(file.getOwner()),
  group(file.getGroup()),
  directory(file.isDirectory()),
  softlink(false),
  softselected(false),
  hardselected(false)
{
  setLastModified(file.getYear(), file.getMonth(), file.getDay(), file.getHour(), file.getMinute());
}

std::string UIFile::getName() const {
  return name;
}

std::string UIFile::getLinkTarget() const {
  return linktarget;
}

std::string UIFile::getOwner() const {
  return owner;
}

std::string UIFile::getGroup() const {
  return group;
}

std::string UIFile::getLastModified() const {
  return lastmodifiedrepr;
}

int UIFile::getModifyTime() const {
  return lastmodified;
}

int UIFile::getModifyDate() const {
  return lastmodifieddate;
}

unsigned long long int UIFile::getSize() const {
  return size;
}

std::string UIFile::getSizeRepr() const {
  return sizerepr;
}

bool UIFile::isDirectory() const {
  return directory;
}

bool UIFile::isLink() const {
  return softlink;
}

bool UIFile::isSoftSelected() const {
  return softselected;
}

bool UIFile::isHardSelected() const {
  return hardselected;
}

void UIFile::softSelect() {
  softselected = true;
}

void UIFile::hardSelect() {
  hardselected = true;
  softselected = false;
}

void UIFile::unSoftSelect() {
  softselected = false;
}

void UIFile::unHardSelect() {
  hardselected = false;
}

void UIFile::parseTimeStamp(const std::string & uglytime) {
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  if (isdigit(uglytime[0]) && isdigit(uglytime[1])) {
    parseWindowsTimeStamp(uglytime, year, month, day, hour, minute);
  }
  else {
    parseUNIXTimeStamp(uglytime, year, month, day, hour, minute);
  }
  setLastModified(year, month, day, hour, minute);
}

void UIFile::parseUNIXTimeStamp(const std::string & uglytime, int & year, int & month, int & day, int & hour, int & minute) {
  if (uglytime.empty()) {
    return;
  }
  size_t len = uglytime.length();
  size_t pos = 0; // month start at 0
  while (pos < len - 1 && uglytime[++pos] != ' ');
  std::string monthtmp = uglytime.substr(0, pos);
  while (pos < len - 1 && uglytime[++pos] == ' '); // day start at pos
  int start = pos;
  while (pos < len - 1 && uglytime[++pos] != ' ');
  std::string daytmp = uglytime.substr(start, pos - start);
  while (pos < len - 1 && uglytime[++pos] == ' '); // meta start at pos
  std::string meta = uglytime.substr(pos);
  month = 0;
  day = std::stoi(daytmp);
  if (monthtmp == "Jan") month = 1;
  else if (monthtmp == "Feb") month = 2;
  else if (monthtmp == "Mar") month = 3;
  else if (monthtmp == "Apr") month = 4;
  else if (monthtmp == "May") month = 5;
  else if (monthtmp == "Jun") month = 6;
  else if (monthtmp == "Jul") month = 7;
  else if (monthtmp == "Aug") month = 8;
  else if (monthtmp == "Sep") month = 9;
  else if (monthtmp == "Oct") month = 10;
  else if (monthtmp == "Nov") month = 11;
  else if (monthtmp == "Dec") month = 12;
  size_t metabreak = meta.find(":");
  if (metabreak == std::string::npos) {
    year = std::stoi(meta);
    hour = 0;
    minute = 0;
  }
  else {
    year = TimeReference::currentYear();
    int currentmonth = TimeReference::currentMonth();
    if (month > currentmonth) {
      year--;
    }
    hour = std::stoi(meta.substr(0, metabreak));
    minute = std::stoi(meta.substr(metabreak + 1));
  }
}

void UIFile::parseWindowsTimeStamp(const std::string & uglytime, int & year, int & month, int & day, int & hour, int & minute) {
  if (uglytime.empty()) {
    return;
  }
  size_t len = uglytime.length();
  size_t pos = 0, start = 0; // date start at 0
  while (pos < len - 1 && uglytime[++pos] != ' ');
  std::string datestamp = uglytime.substr(start, pos - start);
  if (datestamp.length() < 8) { // bad format
    return;
  }
  while (pos < len - 1 && uglytime[++pos] == ' ');
  std::string timestamp = uglytime.substr(pos);
  if (timestamp.length() < 5) { // bad format
    return;
  }
  if (isdigit(datestamp[2])) { // euro format
    year = std::stoi(datestamp.substr(0, 4));
    month = std::stoi(datestamp.substr(3, 2));
    day = std::stoi(datestamp.substr(6, 2));
  }
  else { // US format
    month = std::stoi(datestamp.substr(0, 2));
    day = std::stoi(datestamp.substr(3, 2));
    year = std::stoi(datestamp.substr(6));
    if (datestamp.length() == 8) { // US short year
      year += 2000;
    }
  }
  hour = std::stoi(timestamp.substr(0, 2));
  minute = std::stoi(timestamp.substr(3, 2));
  if (timestamp.length() == 7) { // US format
    hour %= 12;
    if (timestamp[5] == 'P') {
      hour += 12;
    }
  }
}

void UIFile::setLastModified(int year, int month, int day, int hour, int minute) {
  std::string yearstr = std::to_string(year);
  std::string monthstr = std::to_string(month);
  if (month < 10) {
    monthstr = "0" + monthstr;
  }
  std::string daystr = std::to_string(day);
  if (day < 10) {
    daystr = "0" + daystr;
  }
  std::string hourstr = std::to_string(hour);
  if (hour < 10) {
    hourstr = "0" + hourstr;
  }
  std::string minutestr = std::to_string(minute);
  if (minute < 10) {
    minutestr = "0" + minutestr;
  }

  // somewhat incorrect formula, but since the exact stamp will only be used for sorting,
  // there's no need to bother
  lastmodifieddate = ((year - 1970) * 372 * 24 * 60) +
                      (month * 31 * 24 * 60) +
                      (day * 24 * 60);
  lastmodified = lastmodifieddate +
                   (hour * 60) +
                   minute;
  lastmodifiedrepr = yearstr + "-" + monthstr + "-" + daystr + " " + hourstr + ":" + minutestr;
}
