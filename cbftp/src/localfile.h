#pragma once

#include <string>

class LocalFile {
public:
  LocalFile(const std::string& name, unsigned long long int size, bool isdir,
      const std::string& owner, const std::string& group, int year, int month,
      int day, int hour, int minute, bool download = false);
  std::string getName() const;
  unsigned long long int getSize() const;
  bool isDirectory() const;
  std::string getOwner() const;
  std::string getGroup() const;
  int getYear() const;
  int getMonth() const;
  int getDay() const;
  int getHour() const;
  int getMinute() const;
  void setSize(unsigned long long int size);
  int getTouch() const;
  void setTouch(int touch);
  bool isDownloading() const;
  void setDownloading();
  void finishDownload();
private:
  std::string name;
  unsigned long long int size;
  bool isdir;
  std::string owner;
  std::string group;
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int touch;
  bool download;
};
