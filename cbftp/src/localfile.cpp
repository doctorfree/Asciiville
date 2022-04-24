#include "localfile.h"

LocalFile::LocalFile(const std::string& name, unsigned long long int size, bool isdir, const std::string& owner, const std::string& group, int year, int month, int day, int hour, int minute, bool download) :
  name(name),
  size(size),
  isdir(isdir),
  owner(owner),
  group(group),
  year(year),
  month(month),
  day(day),
  hour(hour),
  minute(minute),
  touch(0),
  download(download)
{
}

std::string LocalFile::getName() const {
  return name;
}

unsigned long long int LocalFile::getSize() const {
  return size;
}

bool LocalFile::isDirectory() const {
  return isdir;
}

std::string LocalFile::getOwner() const {
  return owner;
}

std::string LocalFile::getGroup() const {
  return group;
}

int LocalFile::getYear() const {
  return year;
}

int LocalFile::getMonth() const {
  return month;
}

int LocalFile::getDay() const {
  return day;
}

int LocalFile::getHour() const {
  return hour;
}

int LocalFile::getMinute() const {
  return minute;
}

void LocalFile::setSize(unsigned long long int size) {
  this->size = size;
}

int LocalFile::getTouch() const {
  return touch;
}

void LocalFile::setTouch(int touch) {
  this->touch = touch;
}

bool LocalFile::isDownloading() const {
  return download;
}

void LocalFile::setDownloading() {
  download = true;
}

void LocalFile::finishDownload() {
  download = false;
}
