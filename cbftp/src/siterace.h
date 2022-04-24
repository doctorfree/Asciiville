#pragma once

#include <list>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "commandowner.h"
#include "path.h"
#include "racestatus.h"

class Race;
class FileList;
class SiteLogic;
class SkipList;

class SiteRace : public CommandOwner, public std::enable_shared_from_this<SiteRace> {
  private:
    std::weak_ptr<Race> race;
    Path section;
    std::string jobname;
    Path path;
    std::string group;
    std::string username;
    std::string sitename;
    std::list<std::string> recentlyvisited;
    std::list<std::string> completesubdirs;
    std::unordered_map<std::string, std::shared_ptr<FileList>> filelists;
    RaceStatus status;
    std::unordered_set<std::shared_ptr<FileList>> sizeestimated;
    std::unordered_map<std::shared_ptr<FileList>, unsigned long long int> observestarts;
    std::unordered_map<std::shared_ptr<FileList>, unsigned long long int> sfvobservestarts;
    unsigned long long int maxfilesize;
    unsigned long long int totalfilesize;
    unsigned int numuploadedfiles;
    int profile;
    const SkipList& skiplist;
    std::unordered_map<std::string, unsigned long long int> sitessizedown;
    std::unordered_map<std::string, unsigned int> sitesfilesdown;
    std::unordered_map<std::string, unsigned int> sitesspeeddown;
    std::unordered_map<std::string, unsigned long long int> sitessizeup;
    std::unordered_map<std::string, unsigned int> sitesfilesup;
    std::unordered_map<std::string, unsigned int> sitesspeedup;
    unsigned long long int sizedown;
    unsigned int filesdown;
    unsigned int speeddown;
    unsigned long long int sizeup;
    unsigned int filesup;
    unsigned int speedup;
    bool downloadonly;
    unsigned int id;
    void updateNumFilesUploaded(const std::shared_ptr<FileList>& fl);
    void addNewDirectories(const std::shared_ptr<FileList>& fl);
    void markNonExistent(const std::shared_ptr<FileList>& fl);
    void reset();
  public:
    std::string getName() const;
    int classType() const;
    std::string getSiteName() const;
    const Path& getSection() const;
    std::string getGroup() const;
    const Path& getPath() const;
    unsigned int getId() const;
    std::string getRelevantSubPath();
    bool anyFileListNotNonExistent() const;
    SiteRace(const std::shared_ptr<Race>& race, const std::string& sitename, const Path& section, const std::string& jobname, const std::string& username, const SkipList& skiplist, bool downloadonly);
    ~SiteRace();
    std::shared_ptr<FileList> getFileListForPath(const std::string& path) const;
    std::shared_ptr<FileList> getFileListForFullPath(SiteLogic* co, const Path& path) const;
    std::string getSubPathForFileList(const std::shared_ptr<FileList>& fl) const;
    std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator fileListsBegin() const;
    std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator fileListsEnd() const;
    std::weak_ptr<Race> getRace() const;
    bool addSubDirectory(const std::string& subpath, bool knownexists = false);
    std::string getSubPath(const std::shared_ptr<FileList>& fl) const;
    void fileListUpdated(SiteLogic* sl, const std::shared_ptr<FileList>& fl);
    bool sizeEstimated(const std::shared_ptr<FileList>& fl) const;
    unsigned int getNumUploadedFiles() const;
    unsigned long long int getMaxFileSize() const;
    unsigned long long int getTotalFileSize() const;
    bool isDone() const;
    RaceStatus getStatus() const;
    bool isGlobalDone() const;
    int getProfile() const;
    void complete(bool);
    void abort();
    void timeout();
    void softReset();
    void hardReset();
    void subPathComplete(const std::shared_ptr<FileList>& fl);
    void resetListsRefreshed();
    bool isSubPathComplete(const std::string& subpath) const;
    bool isSubPathComplete(const std::shared_ptr<FileList>& fl) const;
    void reportSize(const std::shared_ptr<FileList>& fl, const std::unordered_set<std::string>& uniques, bool final);
    int getObservedTime(const std::shared_ptr<FileList>& fl);
    int getSFVObservedTime(const std::shared_ptr<FileList>& fl);
    bool listsChangedSinceLastCheck();
    void addTransferStatsFile(StatsDirection, const std::string &, unsigned long long int, unsigned int);
    unsigned long long int getSizeDown() const;
    unsigned int getFilesDown() const;
    unsigned int getSpeedDown() const;
    unsigned long long int getSizeUp() const;
    unsigned int getFilesUp() const;
    unsigned int getSpeedUp() const;
    bool allListsRefreshed() const;
    bool isDownloadOnly() const;
    std::unordered_map<std::string, unsigned long long int>::const_iterator sizeUpBegin() const;
    std::unordered_map<std::string, unsigned int>::const_iterator filesUpBegin() const;
    std::unordered_map<std::string, unsigned int>::const_iterator speedUpBegin() const;
    std::unordered_map<std::string, unsigned long long int>::const_iterator sizeDownBegin() const;
    std::unordered_map<std::string, unsigned int>::const_iterator filesDownBegin() const;
    std::unordered_map<std::string, unsigned int>::const_iterator speedDownBegin() const;
    std::unordered_map<std::string, unsigned long long int>::const_iterator sizeUpEnd() const;
    std::unordered_map<std::string, unsigned int>::const_iterator filesUpEnd() const;
    std::unordered_map<std::string, unsigned int>::const_iterator speedUpEnd() const;
    std::unordered_map<std::string, unsigned long long int>::const_iterator sizeDownEnd() const;
    std::unordered_map<std::string, unsigned int>::const_iterator filesDownEnd() const;
    std::unordered_map<std::string, unsigned int>::const_iterator speedDownEnd() const;
};
