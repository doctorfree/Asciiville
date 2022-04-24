#pragma once

#include <regex>
#include <string>

enum Scope {
  SCOPE_IN_RACE = 0,
  SCOPE_ALL = 1
};

enum SkipListAction {
  SKIPLIST_ALLOW,
  SKIPLIST_DENY,
  SKIPLIST_UNIQUE,
  SKIPLIST_SIMILAR,
  SKIPLIST_NONE
};

class SkiplistItem {
public:
  SkiplistItem(bool regex, const std::string & pattern, bool file, bool dir, int scope, SkipListAction action);
  bool matchRegex() const;
  const std::string & matchPattern() const;
  const std::regex & matchRegexPattern() const;
  bool matchFile() const;
  bool matchDir() const;
  SkipListAction getAction() const;
  int matchScope() const;
private:
  bool regex;
  std::string pattern;
  std::regex regexpattern;
  bool file;
  bool dir;
  int scope;
  SkipListAction action;
};
