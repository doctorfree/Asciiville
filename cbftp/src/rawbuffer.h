#pragma once

#include <list>
#include <regex>
#include <string>
#include <vector>
#include <utility>

class RawBufferCallback;

class RawBuffer {
  private:
    void lineFinished();
    void addLogText(const std::string& tag, const std::string& text);
    void applyFilters();
    void applyFilters(bool copy);
    void addFilterLine(const std::pair<std::string, std::string>& line, std::vector<std::pair<std::string, std::string>>& target, unsigned int& latest);
    std::vector<std::pair<std::string, std::string>> log;
    std::vector<std::pair<std::string, std::string>> logfiltered;
    std::vector<std::pair<std::string, std::string>> logcopy;
    std::vector<std::pair<std::string, std::string>> logcopyfiltered;
    unsigned int latestp;
    unsigned int latestpfiltered;
    unsigned int latestpcopy;
    unsigned int latestpcopyfiltered;
    unsigned int maxlength;
    unsigned int bookmarklines;
    std::string site;
    std::string id;
    bool inprogress;
    bool uiwatching;
    bool threads;
    bool eventlog;
    RawBufferCallback * callback;
    std::list<std::string> wildcardfilters;
    bool hasregexfilter;
    std::regex regexfilter;
  public:
    RawBuffer(const std::string& site, const std::string& id);
    RawBuffer(const std::string& site);
    RawBuffer();
    void setCallback(RawBufferCallback *);
    void unsetCallback();
    void bookmark();
    unsigned int linesSinceBookmark() const;
    void setId(int);
    void write(const std::string &);
    void write(const std::string &, const std::string &);
    void writeLine(const std::string &);
    void writeLine(const std::string &, const std::string &);
    void rename(std::string);
    std::string getTag() const;
    unsigned int getSize() const;
    unsigned int getFilteredSize() const;
    unsigned int getCopySize() const;
    unsigned int getFilteredCopySize() const;
    const std::pair<std::string, std::string> & getLine(unsigned int) const;
    const std::pair<std::string, std::string> & getLineCopy(unsigned int) const;
    const std::pair<std::string, std::string> & getFilteredLine(unsigned int) const;
    const std::pair<std::string, std::string> & getFilteredLineCopy(unsigned int) const;
    void freezeCopy();
    void setUiWatching(bool watching);
    void clear();
    bool isFiltered() const;
    bool hasWildcardFilters() const;
    bool hasRegexFilter() const;
    std::list<std::string> getWildcardFilters() const;
    std::regex getRegexFilter() const;
    void setWildcardFilters(const std::list<std::string> & filters);
    void setRegexFilter(const std::regex & filter);
    void unsetFilters();
};
