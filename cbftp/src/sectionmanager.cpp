#include "sectionmanager.h"

#include "globalcontext.h"
#include "sitemanager.h"

std::unordered_map<std::string, Section>::const_iterator SectionManager::begin() const {
  return sections.begin();
}

std::unordered_map<std::string, Section>::const_iterator SectionManager::end() const {
  return sections.end();
}

std::unordered_map<std::string, Section>::iterator SectionManager::begin() {
  return sections.begin();
}

std::unordered_map<std::string, Section>::iterator SectionManager::end() {
  return sections.end();
}

size_t SectionManager::size() const {
  return sections.size();
}

Section * SectionManager::getSection(const std::string & name) {
  auto it = sections.find(name);
  if (it != sections.end()) {
    return &it->second;
  }
  return nullptr;
}

Section * SectionManager::getSection(int hotkey) {
  for (auto it = sections.begin(); it != sections.end(); it++) {
    if (it->second.getHotKey() == hotkey) {
      return &it->second;
    }
  }
  return nullptr;
}

bool SectionManager::addSection(const std::string & name) {
  auto it = sections.find(name);
  if (it == sections.end()) {
    sections.insert(std::pair<std::string, Section>(name, name));
    return true;
  }
  else {
    return false;
  }
}

bool SectionManager::addSection(const Section & section) {
  const std::string & name = section.getName();
  if (name.empty()) {
    return false;
  }
  auto it = sections.find(name);
  if (it == sections.end()) {
    sections.insert(std::pair<std::string, Section>(section.getName(), section));
    return true;
  }
  else {
    return false;
  }
}

bool SectionManager::replaceSection(const Section & section, const std::string & oldname) {
  const std::string & newname = section.getName();
  if (newname.empty()) {
    return false;
  }
  auto it = sections.find(newname);
  if (oldname != newname) {
    if (it != sections.end()) {
      return false;
    }
    sections.erase(oldname);
    sections.insert(std::pair<std::string, Section>(section.getName(), section));
    for (auto it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); ++it) {
      if ((*it)->hasSection(oldname)) {
        (*it)->renameSection(oldname, newname);
      }
    }
  }
  else {
    if (it == sections.end()) {
      return false;
    }
    it->second = section;
  }
  return true;
}

void SectionManager::removeSection(const std::string & section) {
  for (auto it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); ++it) {
    if ((*it)->hasSection(section)) {
      (*it)->removeSection(section);
    }
  }
  sections.erase(section);
}
