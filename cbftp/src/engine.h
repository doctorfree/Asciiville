#pragma once

#include <cassert>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "core/eventreceiver.h"

enum class PrioType;
enum class SitePriority;
class CommandOwner;
class Race;
class TransferJob;
class SiteTransferJob;
class SiteRace;
class FileList;
class File;
class ScoreBoard;
class SiteLogic;
class PendingTransfer;
class Site;
class PreparedRace;
class Path;
class SkipList;
class ScoreBoardElement;
class TransferStatus;

template <class T> class JobList {
public:
  typename std::list<T>::iterator begin() {
    return items.begin();
  }
  typename std::list<T>::const_iterator begin() const {
    return items.begin();
  }
  typename std::list<T>::iterator end() {
    return items.end();
  }
  typename std::list<T>::const_iterator end() const {
    return items.end();
  }
  T& front() {
    return items.front();
  }
  T& back() {
    return items.back();
  }
  void push_back(const T& t) {
    items.push_back(t);
    typename std::pair<typename std::unordered_map<unsigned int, typename std::list<T>::iterator>::iterator, bool> res = index.emplace(t->getId(), --items.end());
    assert(res.second);
  }
  bool empty() const {
    return items.empty();
  }
  typename std::list<T>::const_iterator find(unsigned int id) const {
    typename std::unordered_map<unsigned int, typename std::list<T>::iterator>::const_iterator it = index.find(id);
    if (it != index.end()) {
      return it->second;
    }
    return items.end();
  }
  typename std::list<T>::const_iterator find(const T& t) const {
    return find(t->getId());
  }
 typename std::list<T>::iterator find(unsigned int id) {
    typename std::unordered_map<unsigned int, typename std::list<T>::iterator>::iterator it = index.find(id);
    if (it != index.end()) {
      return it->second;
    }
    return items.end();
  }
  typename std::list<T>::iterator find(const T& t) {
    return find(t->getId());
  }
  bool contains(unsigned int id) {
    return index.find(id) != index.end();
  }
  bool contains(const T& t) {
    return contains(t->getId());
  }
  typename std::list<T>::iterator erase(unsigned int id) {
    typename std::unordered_map<unsigned int, typename std::list<T>::iterator>::iterator it = index.find(id);
    if (it != index.end()) {
      typename std::list<T>::iterator it2 = items.erase(it->second);
      index.erase(id);
      return it2;
    }
    return items.end();
  }
  typename std::list<T>::iterator erase(const T& t) {
    return erase(t->getId());
  }
  typename std::list<T>::iterator erase(typename std::list<T>::iterator it) {
    return erase((*it)->getId());
  }
  typename std::list<T>::iterator remove(const T& t) {
    return erase(t->getId());
  }
  void clear() {
    index.clear();
    items.clear();
  }
  size_t size() const {
    return index.size();
  }
private:
  std::unordered_map<unsigned int, typename std::list<T>::iterator> index;
  std::list<T> items;
};

class Engine : public Core::EventReceiver {
public:
  Engine();
  ~Engine();
  std::shared_ptr<Race> newRace(const std::string& release, const std::string& section, const std::list<std::string>& sites, bool reset = false, const std::list<std::string>& dlonlysites = {});
  bool prepareRace(const std::string& release, const std::string& section, const std::list<std::string>& sites, bool reset = false, const std::list<std::string>& dlonlysites = {});
  std::shared_ptr<Race> newDistribute(const std::string& release, const std::string& section, const std::list<std::string>& sites, bool reset = false, const std::list<std::string>& dlonlysites = {});
  void startPreparedRace(unsigned int);
  void deletePreparedRace(unsigned int);
  void startLatestPreparedRace();
  void toggleStartNextPreparedRace();
  unsigned int newTransferJobDownload(const std::string& srcsite, const std::shared_ptr<FileList>& srcfilelist, const std::string& file, const Path& dstpath);
  unsigned int newTransferJobDownload(const std::string& srcsite, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcfile, const Path& dstpath, const std::string& dstfile);
  unsigned int newTransferJobDownload(const std::string& srcsite, const Path& srcpath, const std::string& srcsection, const std::string& srcfile, const Path& dstpath, const std::string& dstfile);
  unsigned int newTransferJobUpload(const Path& srcpath, const std::string& file, const std::string& dstsite, const std::shared_ptr<FileList>& dstfilelist);
  unsigned int newTransferJobUpload(const Path& srcpath, const std::string& srcfile, const std::string& dstsite, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstfile);
  unsigned int newTransferJobUpload(const Path& srcpath, const std::string& srcfile, const std::string& dstsite, const Path& dstpath, const std::string& dstsection, const std::string& dstfile);
  unsigned int newTransferJobFXP(const std::string& srcsite, const std::shared_ptr<FileList>& srcfilelist, const std::string& dstsite, const std::shared_ptr<FileList>& dstfilelist, const std::string& file);
  unsigned int newTransferJobFXP(const std::string& srcsite, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcfile, const std::string& dstsite, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstfile);
  unsigned int newTransferJobFXP(const std::string& srcsite, const Path& srcpath, const std::string& srcsection, const std::string& srcfile, const std::string& dstsite, const Path& dstpath, const std::string& dstsection, const std::string& dstfile);
  void removeSiteFromRace(const std::shared_ptr<Race> &, const std::string &);
  void removeSiteFromAllRunningSpreadJobs(const std::string& site);
  void removeSiteFromRaceDeleteFiles(const std::shared_ptr<Race>& race, const std::string& site, bool allfiles, bool deleteoncomplete);
  void abortRace(const std::shared_ptr<Race>& race);
  void resetRace(const std::shared_ptr<Race>& race, bool hard);
  void deleteOnAllSites(const std::shared_ptr<Race>& race, bool allfiles = false, bool deleteoncomplete = false);
  void deleteOnSites(const std::shared_ptr<Race>& race, std::list<std::shared_ptr<Site>>, bool allfiles = false);
  void abortTransferJob(const std::shared_ptr<TransferJob>& tj);
  void resetTransferJob(const std::shared_ptr<TransferJob>& tj);
  void jobFileListRefreshed(SiteLogic *, const std::shared_ptr<CommandOwner> & commandowner, const std::shared_ptr<FileList>& fl);
  bool transferJobActionRequest(const std::shared_ptr<SiteTransferJob> &);
  void raceActionRequest();
  void setPreparedRaceExpiryTime(int expirytime);
  void setNextPreparedRaceStarterTimeout(int timeout);
  unsigned int preparedRaces() const;
  unsigned int currentRaces() const;
  unsigned int allRaces() const;
  unsigned int currentTransferJobs() const;
  unsigned int allTransferJobs() const;
  std::shared_ptr<Race> getRace(unsigned int id) const;
  std::shared_ptr<Race> getRace(const std::string& race) const;
  std::shared_ptr<TransferJob> getTransferJob(unsigned int id) const;
  std::shared_ptr<TransferJob> getTransferJob(const std::string& tj) const;
  std::list<std::shared_ptr<PreparedRace>>::const_iterator getPreparedRacesBegin() const;
  std::list<std::shared_ptr<PreparedRace>>::const_iterator getPreparedRacesEnd() const;
  std::list<std::shared_ptr<Race>>::const_iterator getRacesBegin() const;
  std::list<std::shared_ptr<Race>>::const_iterator getRacesEnd() const;
  std::list<std::shared_ptr<Race>>::const_iterator getCurrentRacesBegin() const;
  std::list<std::shared_ptr<Race>>::const_iterator getCurrentRacesEnd() const;
  std::list<std::shared_ptr<Race>>::const_iterator getFinishedRacesBegin() const;
  std::list<std::shared_ptr<Race>>::const_iterator getFinishedRacesEnd() const;
  std::list<std::shared_ptr<TransferJob>>::const_iterator getTransferJobsBegin() const;
  std::list<std::shared_ptr<TransferJob>>::const_iterator getTransferJobsEnd() const;
  void tick(int);
  void addSiteToRace(const std::shared_ptr<Race>& race, const std::string& site, bool downloadonly);
  std::shared_ptr<ScoreBoard> getScoreBoard() const;
  int getMaxPointsRaceTotal() const;
  int getMaxPointsFileSize() const;
  int getMaxPointsAvgSpeed() const;
  int getMaxPointsPriority() const;
  int getMaxPointsPercentageOwned() const;
  int getMaxPointsLowProgress() const;
  int getPriorityPoints(SitePriority priority) const;
  int getSpeedPoints(int) const;
  int getPreparedRaceExpiryTime() const;
  bool getNextPreparedRaceStarterEnabled() const;
  int getNextPreparedRaceStarterTimeout() const;
  int getNextPreparedRaceStarterTimeRemaining() const;
  bool isIncompleteEnoughForDelete(const std::shared_ptr<Race> & race, const std::shared_ptr<SiteRace> & siterace) const;
  void transferFailed(const std::shared_ptr<TransferStatus> & ts, int err);
  int getMaxSpreadJobsHistory() const;
  int getMaxTransferJobsHistory() const;
  void setMaxSpreadJobsHistory(int jobs);
  void setMaxTransferJobsHistory(int jobs);
  int getMaxSpreadJobTimeSeconds() const;
  void setMaxSpreadJobTimeSeconds(int seconds);
 private:
  std::shared_ptr<Race> newSpreadJob(int profile, const std::string& release, const std::string& section, const std::list<std::string>& sites, bool reset, const std::list<std::string>& dlonlysites);
  void estimateRaceSizes();
  void estimateRaceSize(const std::shared_ptr<Race>&, bool forceupdate = false);
  void reportCurrentSize(const SkipList&, const SkipList&, const std::shared_ptr<SiteRace>& srs, const std::shared_ptr<FileList>& fl, bool final);
  void addToScoreBoard(const std::shared_ptr<FileList>& fl, const std::shared_ptr<SiteRace>& sr, const std::shared_ptr<SiteLogic>& sl);
  void addToScoreBoardForPair(const std::shared_ptr<SiteLogic>& sls,
      const std::shared_ptr<Site> & ss, const std::shared_ptr<SiteRace>& srs,
      const std::shared_ptr<FileList>& fls, const std::shared_ptr<SiteLogic>& sld, const std::shared_ptr<Site>& ds,
      const std::shared_ptr<SiteRace>& srd, const std::shared_ptr<FileList>& fld, const SkipList& dstskip,
      const SkipList& secskip,
      const std::shared_ptr<Race>& race, const Path& subpath, SitePriority priority,
      bool racemode);
  void updateScoreBoard();
  void refreshScoreBoard();
  void issueOptimalTransfers();
  void setSpeedScale();
  unsigned short calculateScore(PrioType priotype, unsigned long long int filesize, const std::shared_ptr<Race>&, const std::shared_ptr<FileList>& fls, const std::shared_ptr<SiteRace>& srs, const std::shared_ptr<FileList>& fld, const std::shared_ptr<SiteRace> & srd, int, SitePriority priority, bool) const;
  unsigned short calculateScore(ScoreBoardElement* sbe) const;
  void checkIfRaceComplete(const std::shared_ptr<SiteLogic>& sls, std::shared_ptr<Race>& race);
  void raceComplete(const std::shared_ptr<Race>& race);
  void transferJobComplete(const std::shared_ptr<TransferJob>& tj);
  void issueGlobalComplete(const std::shared_ptr<Race>& race);
  void refreshPendingTransferList(const std::shared_ptr<TransferJob>& tj);
  void checkStartPoke();
  void addPendingTransfer(std::list<PendingTransfer>&, PendingTransfer&);
  std::shared_ptr<Race> getCurrentRace(const std::string &) const;
  void preSeedPotentialData(const std::shared_ptr<Race>& race);
  bool raceTransferPossible(const std::shared_ptr<SiteLogic>& sls, const std::shared_ptr<SiteRace>& srs, const std::shared_ptr<SiteLogic>& sld, const std::shared_ptr<SiteRace>& srd, const std::shared_ptr<Race>& race) const;
  void wipeFromScoreBoard(const std::shared_ptr<SiteRace>& sr);
  bool waitingInScoreBoard(const std::shared_ptr<Race>& race) const;
  bool transferExpectedSoon(ScoreBoardElement* sbe) const;
  void restoreFromFailed(const std::shared_ptr<Race>& race);
  void removeFromFinished(const std::shared_ptr<Race>& race);
  void clearSkipListCaches();
  void rotateSpreadJobsHistory();
  void rotateTransferJobsHistory();
  JobList<std::shared_ptr<Race>> allraces;
  JobList<std::shared_ptr<Race>> currentraces;
  JobList<std::shared_ptr<Race>> finishedraces;
  JobList<std::shared_ptr<PreparedRace>> preparedraces;
  JobList<std::shared_ptr<TransferJob>> alltransferjobs;
  JobList<std::shared_ptr<TransferJob>> currenttransferjobs;
  std::shared_ptr<ScoreBoard> scoreboard;
  std::shared_ptr<ScoreBoard> failboard;
  std::unordered_map<std::shared_ptr<TransferJob>, std::list<PendingTransfer>> pendingtransfers;
  int maxavgspeed;
  bool pokeregistered;
  unsigned int dropped;
  unsigned int nextid;
  int maxpointsfilesize;
  int maxpointsavgspeed;
  int maxpointspriority;
  int maxpointspercentageowned;
  int maxpointslowprogress;
  int preparedraceexpirytime;
  int startnextpreparedtimeout;
  bool startnextprepared;
  int nextpreparedtimeremaining;
  std::unordered_map<std::shared_ptr<FileList>, std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic>>> spreadjobfilelistschanged;
  bool forcescoreboard;
  std::unordered_set<std::shared_ptr<Site>> skiplistcachesites;
  std::unordered_set<std::string> skiplistcachesections;
  int maxspreadjobshistory;
  int maxtransferjobshistory;
  int maxspreadjobtimeseconds;
};
