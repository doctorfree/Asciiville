#pragma once

#include <list>
#include <regex>
#include <string>
#include <unordered_map>

#include "skiplistitem.h"

class Site;

struct SkipListMatch {
  SkipListMatch(SkipListAction action, bool matched, const std::string& matchpattern, const std::string& matchedpath);
  SkipListMatch(SkipListAction action, bool matched, const std::string& matchpattern, const std::regex& matchregexpattern, const std::string& matchedpath);
  SkipListAction action;
  bool matched;
  bool regex;
  std::string matchpattern;
  std::regex matchregexpattern;
  std::string matchedpath;
};

class SkipList {
private:
  std::list<SkiplistItem> entries;
  mutable std::unordered_map<std::string, SkipListMatch> matchcache;
  const SkipList * globalskip;
  void addDefaultEntries();
  SkipListMatch fallThrough(const std::string & element, const bool dir, const bool inrace, const SkipList * fallthrough) const;
public:
  SkipList(bool adddefault = true);
  SkipList(const SkipList *);
  void addEntry(bool regex, const std::string & pattern, bool file, bool dir, int scope, SkipListAction action);
  void clearEntries();
  std::list<SkiplistItem>::const_iterator entriesBegin() const;
  std::list<SkiplistItem>::const_iterator entriesEnd() const;
  SkipListMatch check(const std::string & element, const bool dir, const bool inrace = true, const SkipList * fallthrough = nullptr) const;
  unsigned int size() const;
  void wipeCache();
  void setGlobalSkip(SkipList *);
};
