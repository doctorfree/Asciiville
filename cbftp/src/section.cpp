#include "section.h"

#include "globalcontext.h"

Section::Section() : jobs(0), skiplist(global->getSkipList()), hotkey(-1) {

}

Section::Section(const std::string & name) : name(name), jobs(0), skiplist(global->getSkipList()), hotkey(-1) {

}

std::string Section::getName() const {
  return name;
}

SkipList & Section::getSkipList() {
  return skiplist;
}

const SkipList & Section::getSkipList() const {
  return skiplist;
}

int Section::getNumJobs() const {
  return jobs;
}

void Section::setName(const std::string & newname) {
  name = newname;
}

void Section::setNumJobs(int jobs) {
  this->jobs = jobs;
}

void Section::addJob() {
  ++jobs;
}

int Section::getHotKey() const {
  return hotkey;
}

void Section::setHotKey(int hotkey) {
  this->hotkey = hotkey;
}
