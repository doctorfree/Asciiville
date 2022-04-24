#pragma once

#include <list>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <set>
#include <vector>

#include "uifile.h"

#include "../path.h"

class FileList;
class LocalFileList;

enum class CompareMode {
  NONE,
  UNIQUE,
  IDENTICAL
};

class UIFileList {
public:
  enum class SortMethod {
    COMBINED = 0,
    NAME_ASC = 1,
    NAME_DESC = 2,
    SIZE_ASC = 3,
    SIZE_DESC = 4,
    TIME_ASC = 5,
    TIME_DESC = 6,
    OWNER_ASC = 7,
    OWNER_DESC = 8,
  };
private:
  std::vector<UIFile> files;
  std::vector<UIFile *> sortedfiles;
  std::list<unsigned int> selectedfiles;
  unsigned int currentposition;
  UIFile * currentcursored;
  Path path;
  unsigned int numfiles;
  unsigned int numdirs;
  unsigned long long int totalsize;
  unsigned int filterednumfiles;
  unsigned int filterednumdirs;
  unsigned long long int filteredtotalsize;
  SortMethod sortmethod;
  bool separators;
  std::list<std::string> wildcardfilters;
  bool hasregexfilter;
  std::regex regexfilter;
  std::string regexfilterstr;
  CompareMode comparemode;
  std::set<std::string> comparelist;
  bool initialized;
  void setNewCurrentPosition();
  void removeSeparators();
  void fillSortedFiles();
  void insertDateSeparators();
  void insertFirstLetterSeparators();
  void insertOwnerSeparators();
public:
  UIFileList();
  void sortMethod(SortMethod sortmethod);
  void parse(const std::shared_ptr<FileList>& fl);
  void parse(const std::shared_ptr<LocalFileList>& fl);
  UIFile* cursoredFile() const;
  bool selectFileName(const std::string& filename);
  bool goNext();
  bool goPrevious();
  void toggleSelectCurrent();
  void unSelect();
  unsigned int size() const;
  unsigned int sizeFiles() const;
  unsigned int sizeDirs() const;
  unsigned long long int getTotalSize() const;
  unsigned int filteredSizeFiles() const;
  unsigned int filteredSizeDirs() const;
  unsigned long long int getFilteredTotalSize() const;
  const std::vector<UIFile*>* getSortedList() const;
  unsigned int currentCursorPosition() const;
  const Path & getPath() const;
  std::string getSortMethod() const;
  static std::string getSortMethod(SortMethod sortmethod);
  bool separatorsEnabled() const;
  bool isInitialized() const;
  void removeFile(std::string);
  void toggleSeparators();
  void setCursorPosition(unsigned int);
  bool hasWildcardFilters() const;
  bool hasRegexFilter() const;
  std::list<std::string> getWildcardFilters() const;
  std::regex getRegexFilter() const;
  std::string getRegexFilterString() const;
  void setWildcardFilters(const std::list<std::string>& filters);
  void setRegexFilter(const std::regex& filter, const std::string& regexfilterstr);
  void unsetFilters();
  void setCompareList(const std::set<std::string>& list, CompareMode mode);
  CompareMode getCompareListMode() const;
  std::set<std::string> getCompareList() const;
  void clearCompareListMode();
  bool contains(const std::string& filename) const;
  void hardFlipSoftSelected();
  bool clearSoftSelected();
  bool clearSelected();
  const UIFile* getFile(const std::string& file) const;
  const std::list<UIFile*> getSelectedFiles() const;
  std::list<std::pair<std::string, bool> > getSelectedNames() const;
  std::list<std::pair<std::string, bool> > getSelectedDirectoryNames() const;
  std::list<std::pair<std::string, bool> > getSelectedFileNames(bool files, bool dirs) const;
};

bool combinedSort(UIFile *, UIFile *);
bool nameSortAsc(UIFile *, UIFile *);
bool nameSortDesc(UIFile *, UIFile *);
bool timeSortAsc(UIFile *, UIFile *);
bool timeSortDesc(UIFile *, UIFile *);
bool sizeSortAsc(UIFile *, UIFile *);
bool sizeSortDesc(UIFile *, UIFile *);
bool ownerSortAsc(UIFile *, UIFile *);
bool ownerSortDesc(UIFile *, UIFile *);
