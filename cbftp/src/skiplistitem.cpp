#include "skiplistitem.h"

#include "util.h"

SkiplistItem::SkiplistItem(bool regex, const std::string & pattern, bool file, bool dir, int scope, SkipListAction action) :
  regex(regex), pattern(pattern), file(file), dir(dir), scope(scope), action(action)
{
  if (regex) {
    regexpattern = util::regexParse(pattern);
  }
}

bool SkiplistItem::matchRegex() const {
  return regex;
}

const std::regex & SkiplistItem::matchRegexPattern() const {
  return regexpattern;
}

const std::string & SkiplistItem::matchPattern() const {
  return pattern;
}

bool SkiplistItem::matchFile() const {
  return file;
}

bool SkiplistItem::matchDir() const {
  return dir;
}

SkipListAction SkiplistItem::getAction() const {
  return action;
}

int SkiplistItem::matchScope() const {
  return scope;
}
