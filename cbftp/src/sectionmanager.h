#pragma once

#include <unordered_map>

#include "section.h"

class SectionManager {
public:
  std::unordered_map<std::string, Section>::const_iterator begin() const;
  std::unordered_map<std::string, Section>::const_iterator end() const;
  std::unordered_map<std::string, Section>::iterator begin();
  std::unordered_map<std::string, Section>::iterator end();
  size_t size() const;
  Section * getSection(const std::string & name);
  Section * getSection(int hotkey);
  bool addSection(const std::string & name);
  bool addSection(const Section & section);
  bool replaceSection(const Section & section, const std::string & oldname);
  void removeSection(const std::string & section);
private:
  std::unordered_map<std::string, Section> sections;
};
