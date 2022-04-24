#pragma once

#include <string>

#include "skiplist.h"

class Section {
public:
  Section();
  Section(const std::string & name);
  std::string getName() const;
  SkipList & getSkipList();
  const SkipList & getSkipList() const;
  int getNumJobs() const;
  void setName(const std::string & newname);
  void setNumJobs(int jobs);
  void addJob();
  int getHotKey() const;
  void setHotKey(int hotkey);
private:
  std::string name;
  int jobs;
  SkipList skiplist;
  int hotkey;
};
