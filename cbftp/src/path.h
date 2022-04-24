#pragma once

#include <string>
#include <list>
#include <ostream>

class Path {
public:
  Path();
  Path(const char *);
  Path(const std::string &);
  const std::string & toString() const;
  const std::string & unixPath() const;
  const std::string & dirName() const;
  const std::string & baseName() const;
  bool isAbsolute() const;
  bool isRelative() const;
  Path operator/(const Path &) const;
  Path operator/(const std::string &) const;
  Path operator/(const char *) const;
  bool operator==(const Path &) const;
  bool operator!=(const Path &) const;
  bool contains(const Path &) const;
  Path operator-(const Path &) const;
  std::list<std::string> split() const;
  int levels() const;
  Path cutLevels(int) const;
  Path level(int) const;
  bool operator<(const Path& other) const;
  bool empty() const;
private:
  Path(const char *, char, bool);
  Path(const std::string &, char, bool);
  Path(const Path &, const Path &);
  void setSeparatorAbsolute();
  void flipSlashes();
  void cleanPath();
  void setPathParts();
  std::string path;
  std::string unixpath;
  std::string dirname;
  std::string basename;
  char separatorchar;
  bool absolute;
  bool separatorcertain;
};

std::ostream & operator<<(std::ostream &, const Path &);
