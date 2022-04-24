#include "uifilelist.h"

#include <algorithm>
#include <regex>
#include <unordered_map>

#include "../filelist.h"
#include "../file.h"
#include "../localfilelist.h"
#include "../localfile.h"
#include "../util.h"

UIFileList::UIFileList() :
  currentposition(0),
  currentcursored(NULL),
  path("/"),
  numfiles(0),
  numdirs(0),
  totalsize(0),
  filterednumfiles(0),
  filterednumdirs(0),
  filteredtotalsize(0),
  sortmethod(SortMethod::COMBINED),
  separators(false),
  hasregexfilter(false),
  comparemode(CompareMode::NONE),
  initialized(false)
{
}

bool combinedSort(UIFile * a, UIFile * b) {
  bool aisdir = a->isDirectory();
  bool bisdir = b->isDirectory();
  bool aislink = a->isLink();
  bool bislink = b->isLink();
  if ((aislink || aisdir) && !bislink && !bisdir) return true;
  if (!aislink && !aisdir && (bislink || bisdir)) return false;
  if ((aisdir || aislink) && (bisdir || bislink)) return timeSortDesc(a, b);
  return sizeSortDesc(a, b);
}

bool nameSortAsc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  return util::naturalComparator()(a->getName(), b->getName());
}

bool nameSortDesc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  return util::naturalComparator()(b->getName(), a->getName());
}

bool timeSortAsc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  int diff = a->getModifyTime() - b->getModifyTime();
  if (diff == 0) return nameSortAsc(a, b);
  return diff < 0;
}

bool timeSortDesc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  int diff = a->getModifyTime() - b->getModifyTime();
  if (diff == 0) return nameSortDesc(a, b);
  return diff > 0;
}

bool sizeSortAsc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  long int diff = a->getSize() - b->getSize();
  if (diff == 0) return nameSortAsc(a, b);
  return diff < 0;
}

bool sizeSortDesc(UIFile * a, UIFile * b) {
  if ((a->isLink() || a->isDirectory()) && !b->isLink() && !b->isDirectory()) return true;
  if (!a->isLink() && !a->isDirectory() && (b->isLink() || b->isDirectory())) return false;
  long int diff = a->getSize() - b->getSize();
  if (diff == 0) return nameSortAsc(a, b);
  return diff > 0;
}

bool ownerSortAsc(UIFile * a, UIFile * b) {
  int diff = a->getOwner().compare(b->getOwner());
  if (diff == 0) return nameSortAsc(a, b);
  return diff < 0;
}

bool ownerSortDesc(UIFile * a, UIFile * b) {
  int diff = a->getOwner().compare(b->getOwner());
  if (diff == 0) return nameSortAsc(a, b);
  return diff > 0;
}

void UIFileList::sortMethod(SortMethod sortmethod) {
  this->sortmethod = sortmethod;
  removeSeparators();
  switch (sortmethod) {
    case SortMethod::COMBINED:
      std::sort(sortedfiles.begin(), sortedfiles.end(), combinedSort);
      insertDateSeparators();
      break;
    case SortMethod::NAME_ASC:
      std::sort(sortedfiles.begin(), sortedfiles.end(), nameSortAsc);
      insertFirstLetterSeparators();
      break;
    case SortMethod::NAME_DESC:
      std::sort(sortedfiles.begin(), sortedfiles.end(), nameSortDesc);
      insertFirstLetterSeparators();
      break;
    case SortMethod::SIZE_ASC:
      std::sort(sortedfiles.begin(), sortedfiles.end(), sizeSortAsc);
      break;
    case SortMethod::SIZE_DESC:
      std::sort(sortedfiles.begin(), sortedfiles.end(), sizeSortDesc);
      break;
    case SortMethod::TIME_ASC:
      std::sort(sortedfiles.begin(), sortedfiles.end(), timeSortAsc);
      insertDateSeparators();
      break;
    case SortMethod::TIME_DESC:
      std::sort(sortedfiles.begin(), sortedfiles.end(), timeSortDesc);
      insertDateSeparators();
      break;
    case SortMethod::OWNER_ASC:
      std::sort(sortedfiles.begin(), sortedfiles.end(), ownerSortAsc);
      insertOwnerSeparators();
      break;
    case SortMethod::OWNER_DESC:
      std::sort(sortedfiles.begin(), sortedfiles.end(), ownerSortDesc);
      insertOwnerSeparators();
      break;
  }
  setNewCurrentPosition();
}

void UIFileList::insertDateSeparators() {
  if (separators) {
    int lastdate = 0;
    bool complete = false;
    while (!complete) {
      complete = true;
      std::vector<UIFile *>::iterator lastit = sortedfiles.end();
      for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
        if (*it != NULL && (*it)->getModifyDate() != lastdate) {
          lastdate = (*it)->getModifyDate();
          if (*lastit != NULL && (it != sortedfiles.begin() || !(*it)->isDirectory())) {
            sortedfiles.insert(it, NULL);
            complete = false;
            break;
          }
        }
        lastit = it;
      }
    }
  }
}

void UIFileList::insertFirstLetterSeparators() {
  if (separators) {
    std::string lastletter = "";
    bool complete = false;
    while (!complete) {
      complete = true;
      std::vector<UIFile *>::iterator lastit = sortedfiles.end();
      for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
        if (*it != NULL) {
          std::string firstchar = (*it)->getName().substr(0, 1);
          if (firstchar != lastletter) {
            lastletter = firstchar;
            if (*lastit != NULL && it != sortedfiles.begin()) {
              sortedfiles.insert(it, NULL);
              complete = false;
              break;
            }
          }
        }
        lastit = it;
      }
    }
  }
}

void UIFileList::insertOwnerSeparators() {
  if (separators) {
    std::string lastowner;
    bool complete = false;
    while (!complete) {
      complete = true;
      std::vector<UIFile *>::iterator lastit = sortedfiles.end();
      for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
        if (*it != NULL && (*it)->getOwner() != lastowner) {
          lastowner = (*it)->getOwner();
          if (*lastit != NULL && it != sortedfiles.begin()) {
            sortedfiles.insert(it, NULL);
            complete = false;
            break;
          }
        }
        lastit = it;
      }
    }
  }
}

void UIFileList::setNewCurrentPosition() {
  if (currentcursored != NULL) {
    for (unsigned int i = 0; i < sortedfiles.size(); i++) {
      if (sortedfiles[i] == currentcursored) {
        currentposition = i;
        return;
      }
    }
  }
  currentposition = 0;
  currentcursored = NULL;
  if (sortedfiles.size() > 0) {
    currentcursored = sortedfiles[currentposition];
  }
}

bool UIFileList::selectFileName(const std::string & filename) {
  for (unsigned int i = 0; i < sortedfiles.size(); i++) {
    if (sortedfiles[i] != NULL && sortedfiles[i]->getName() == filename) {
      currentposition = i;
      currentcursored = sortedfiles[i];
      return true;
    }
  }
  return false;
}

bool UIFileList::contains(const std::string & filename) const {
  for (unsigned int i = 0; i < sortedfiles.size(); i++) {
    if (sortedfiles[i] != NULL && sortedfiles[i]->getName() == filename) {
      return true;
    }
  }
  return false;
}

void UIFileList::fillSortedFiles() {
  sortedfiles.clear();
  filterednumfiles = 0;
  filterednumdirs = 0;
  filteredtotalsize = 0;
  for (unsigned int i = 0; i < files.size(); i++) {
    UIFile & file = files[i];
    if (!wildcardfilters.empty()) {
      bool negativepass = true;
      bool foundpositive = false;
      bool positivematch = false;
      for (std::list<std::string>::const_iterator it = wildcardfilters.begin(); it != wildcardfilters.end(); it++) {
        const std::string & filter = *it;
        if (filter[0] == '!') {
          if (util::wildcmp(filter.substr(1).c_str(), file.getName().c_str())) {
            negativepass = false;
            break;
          }
        }
        else {
          foundpositive = true;
          if (util::wildcmp(filter.c_str(), file.getName().c_str())) {
            positivematch = true;
          }
        }
      }
      if (!negativepass || (foundpositive && !positivematch)) {
        continue;
      }
    }
    else if (hasregexfilter && !std::regex_match(file.getName(), regexfilter)) {
      continue;
    }
    if (comparemode == CompareMode::UNIQUE && comparelist.find(file.getName()) != comparelist.end()) {
      continue;
    }
    else if (comparemode == CompareMode::IDENTICAL && comparelist.find(file.getName()) == comparelist.end()) {
      continue;
    }
    sortedfiles.push_back(&file);
    if (file.isDirectory()) {
      filterednumdirs++;
    }
    else {
      filterednumfiles++;
    }
    filteredtotalsize += file.getSize();
  }
  sortMethod(sortmethod);
}

void UIFileList::parse(const std::shared_ptr<FileList>& fl) {
  files.clear();
  numfiles = 0;
  numdirs = 0;
  totalsize = 0;
  selectedfiles.clear();
  currentposition = 0;
  currentcursored = nullptr;
  separators = false;
  wildcardfilters.clear();
  hasregexfilter = false;
  std::list<File*>::iterator it;
  int size = fl->getSize();
  files.reserve(size);
  sortedfiles.reserve(size);
  for (it = fl->begin(); it != fl->end(); it++) {
    files.push_back(UIFile(*it));
    totalsize += (*it)->getSize();
    if ((*it)->isDirectory()) {
      numdirs++;
    }
    else {
      numfiles++;
    }
  }
  fillSortedFiles();
  path = fl->getPath();
  initialized = true;
}

void UIFileList::parse(const std::shared_ptr<LocalFileList>& fl) {
  files.clear();
  numfiles = 0;
  numdirs = 0;
  totalsize = 0;
  selectedfiles.clear();
  currentposition = 0;
  currentcursored = nullptr;
  separators = false;
  wildcardfilters.clear();
  hasregexfilter = false;
  std::unordered_map<std::string, LocalFile>::const_iterator it;
  int size = fl->size();
  files.reserve(size);
  sortedfiles.reserve(size);
  for (it = fl->begin(); it != fl->end(); it++) {
    const LocalFile& f = it->second;
    files.push_back(UIFile(f));
    totalsize += f.getSize();
    if (f.isDirectory()) {
      numdirs++;
    }
    else {
      numfiles++;
    }
  }
  fillSortedFiles();
  path = fl->getPath();
  initialized = true;
}

UIFile * UIFileList::cursoredFile() const {
  return currentcursored;
}

bool UIFileList::goNext() {
  if (size() > 0 && currentposition < size() - 1) {
    while ((currentcursored = sortedfiles[++currentposition]) == NULL) {
      if (currentposition >= size() - 1) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool UIFileList::goPrevious() {
  if (size() > 0 && currentposition > 0) {
    while ((currentcursored = sortedfiles[--currentposition]) == NULL) {
      if (!currentposition) {
        return false;
      }
    }
    return true;
  }
  return false;
}

void UIFileList::toggleSelectCurrent() {
  std::list<unsigned int>::iterator it;
  for (it = selectedfiles.begin(); it != selectedfiles.end(); it++) {
    if (*it == currentposition) {
      selectedfiles.erase(it);
      return;
    }
  }
  selectedfiles.push_back(currentposition);
}

void UIFileList::unSelect() {
  selectedfiles.clear();
}

unsigned int UIFileList::size() const {
  return sortedfiles.size();
}

unsigned int UIFileList::sizeFiles() const {
  return numfiles;
}

unsigned int UIFileList::sizeDirs() const {
  return numdirs;
}

unsigned long long int UIFileList::getTotalSize() const {
  return totalsize;
}

unsigned int UIFileList::filteredSizeFiles() const {
  return filterednumfiles;
}

unsigned int UIFileList::filteredSizeDirs() const {
  return filterednumdirs;
}

unsigned long long int UIFileList::getFilteredTotalSize() const {
  return filteredtotalsize;
}

const std::vector<UIFile *> * UIFileList::getSortedList() const {
  return &sortedfiles;
}

unsigned int UIFileList::currentCursorPosition() const {
  return currentposition;
}

const Path & UIFileList::getPath() const {
  return path;
}

std::string UIFileList::getSortMethod() const {
  return getSortMethod(sortmethod);
}

std::string UIFileList::getSortMethod(SortMethod sortmethod) {
  switch (sortmethod) {
    case SortMethod::COMBINED:
      return "Combined";
    case SortMethod::NAME_ASC:
      return "Name (ascending)";
    case SortMethod::NAME_DESC:
      return "Name (descending)";
    case SortMethod::SIZE_ASC:
      return "Size (ascending)";
    case SortMethod::SIZE_DESC:
      return "Size (descending)";
    case SortMethod::TIME_ASC:
      return "Time (ascending)";
    case SortMethod::TIME_DESC:
      return "Time (descending)";
    case SortMethod::OWNER_ASC:
      return "Owner (ascending)";
    case SortMethod::OWNER_DESC:
      return "Owner (descending)";
  }
  return "";
}

bool UIFileList::separatorsEnabled() const {
  return separators;
}

bool UIFileList::isInitialized() const {
  return initialized;
}

void UIFileList::removeFile(std::string file) {
  for (unsigned int i = 0; i < sortedfiles.size(); i++) {
    if (sortedfiles[i] != NULL && sortedfiles[i]->getName() == file) {
      totalsize -= sortedfiles[i]->getSize();
      if (sortedfiles[i]->isDirectory()) {
        numdirs--;
      }
      else {
        numfiles--;
      }
      if (currentcursored == sortedfiles[i]) {
        if (sortedfiles.size() > i + 1) {
          if (!goNext()) {
            currentcursored = NULL;
          }
        }
        else if (sortedfiles.size() == 1) {
          currentcursored = NULL;
        }
        else {
          if (!goPrevious()) {
            currentcursored = NULL;
          }
        }
      }
      sortedfiles.erase(sortedfiles.begin() + i);
      setNewCurrentPosition();
      break;
    }
  }
}

void UIFileList::toggleSeparators() {
  if (!separators) {
    separators = true;
  }
  else {
    separators = false;
  }
}

void UIFileList::removeSeparators() {
  bool complete = false;
  while(!complete) {
    complete = true;
    for (std::vector<UIFile *>::iterator it = sortedfiles.begin(); it != sortedfiles.end(); it++) {
      if (*it == NULL) {
        sortedfiles.erase(it);
        complete = false;
        break;
      }
    }
  }
}

bool UIFileList::hasWildcardFilters() const {
  return wildcardfilters.size();
}

bool UIFileList::hasRegexFilter() const {
  return hasregexfilter;
}

std::list<std::string> UIFileList::getWildcardFilters() const {
  return wildcardfilters;
}

std::regex UIFileList::getRegexFilter() const {
  return regexfilter;
}

std::string UIFileList::getRegexFilterString() const {
  return regexfilterstr;
}

void UIFileList::setWildcardFilters(const std::list<std::string>& filters) {
  this->wildcardfilters = filters;
  hasregexfilter = false;
  fillSortedFiles();
}

void UIFileList::setRegexFilter(const std::regex& filter, const std::string& regexfilterstr) {
  regexfilter = filter;
  this->regexfilterstr = regexfilterstr;
  hasregexfilter = true;
  wildcardfilters.clear();
  fillSortedFiles();
}

void UIFileList::unsetFilters() {
  wildcardfilters.clear();
  hasregexfilter = false;
  fillSortedFiles();
}

void UIFileList::setCompareList(const std::set<std::string>& comparelist, CompareMode mode) {
  this->comparemode = mode;
  this->comparelist = comparelist;
  fillSortedFiles();
}

CompareMode UIFileList::getCompareListMode() const {
  return comparemode;
}

std::set<std::string> UIFileList::getCompareList() const {
  return comparelist;
}

void UIFileList::clearCompareListMode() {
  comparelist.clear();
  comparemode = CompareMode::NONE;
  fillSortedFiles();
}

void UIFileList::hardFlipSoftSelected() {
  for (unsigned int i = 0; i < files.size(); i++) {
    if (files[i].isSoftSelected()) {
      if (files[i].isHardSelected()) {
        files[i].unHardSelect();
      }
      else {
        files[i].hardSelect();
      }
      files[i].unSoftSelect();
    }
  }
}

bool UIFileList::clearSoftSelected() {
  bool cleared = false;
  for (unsigned int i = 0; i < files.size(); i++) {
    if (files[i].isSoftSelected()) {
      files[i].unSoftSelect();
      cleared = true;
    }
  }
  return cleared;
}

bool UIFileList::clearSelected() {
  bool cleared = false;
  for (unsigned int i = 0; i < files.size(); i++) {
    if (files[i].isSoftSelected()) {
      files[i].unSoftSelect();
      cleared = true;
    }
    if (files[i].isHardSelected()) {
      files[i].unHardSelect();
      cleared = true;
    }
  }
  return cleared;
}

void UIFileList::setCursorPosition(unsigned int position) {
  if (sortedfiles.size()) {
    while (position >= sortedfiles.size()) {
      position--;
    }
    currentposition = position;
    currentcursored = sortedfiles[currentposition];
  }
  else {
    currentposition = 0;
    currentcursored = NULL;
  }
}

const UIFile * UIFileList::getFile(const std::string & file) const {
  for (unsigned int i = 0; i < files.size(); i++) {
    if (files[i].getName() == file) {
      return &files[i];
    }
  }
  return NULL;
}

const std::list<UIFile *> UIFileList::getSelectedFiles() const {
  std::list<UIFile *> selectedfiles;
  UIFile * cursored = cursoredFile();
  for (unsigned int i = 0; i < sortedfiles.size(); i++) {
    UIFile * file = sortedfiles[i];
    if (file == NULL) {
      continue;
    }
    if (file->isSoftSelected() || file->isHardSelected() || file == cursored) {
      selectedfiles.push_back(file);
    }
  }
  return selectedfiles;
}

std::list<std::pair<std::string, bool> > UIFileList::getSelectedNames() const {
  return getSelectedFileNames(true, true);
}

std::list<std::pair<std::string, bool> > UIFileList::getSelectedDirectoryNames() const {
  return getSelectedFileNames(false, true);
}

std::list<std::pair<std::string, bool> > UIFileList::getSelectedFileNames(bool files, bool dirs) const {
  std::list<std::pair<std::string, bool> > selectedfiles;
  UIFile * cursored = cursoredFile();
  for (unsigned int i = 0; i < sortedfiles.size(); i++) {
    UIFile * file = sortedfiles[i];
    if (file == NULL) {
      continue;
    }
    if (file->isSoftSelected() || file->isHardSelected() || file == cursored) {
      if ((dirs && file->isDirectory()) || (files && !file->isDirectory())) {
        selectedfiles.push_back(std::pair<std::string, bool>(file->getName(), file->isDirectory()));
      }
    }
  }
  return selectedfiles;
}
