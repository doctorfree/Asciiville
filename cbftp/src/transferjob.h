#pragma once

#include <list>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <string>

#include "core/eventreceiver.h"
#include "transferstatuscallback.h"
#include "path.h"

enum TransferJobType {
  TRANSFERJOB_DOWNLOAD,
  TRANSFERJOB_UPLOAD,
  TRANSFERJOB_FXP
};

enum TransferJobStatus {
  TRANSFERJOB_QUEUED,
  TRANSFERJOB_RUNNING,
  TRANSFERJOB_DONE,
  TRANSFERJOB_ABORTED
};

#define TRANSFERJOB_UPDATE_INTERVAL 250

class SiteTransferJob;
class SiteLogic;
class FileList;
class TransferStatus;
class LocalFileList;
class File;
class SkipList;

class TransferJob : public Core::EventReceiver, private TransferStatusCallback {
public:
  std::string getName() const;
  TransferJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcfile, const Path& dstpath, const std::string& dstfile);
  TransferJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const Path& srcpath, const std::string& srcsection, const std::string& srcfile, const Path& dstpath, const std::string& dstfile);
  TransferJob(unsigned int id, const Path& srcpath, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstfile);
  TransferJob(unsigned int id, const Path& srcpath, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const Path& dstpath, const std::string& dstsection, const std::string& dstfile);
  TransferJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstfile);
  TransferJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const Path& srcpath, const std::string& srcsection, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const Path& dstpath, const std::string& dstsection, const std::string& dstfile);
  ~TransferJob();
  void createSiteTransferJobs(const std::shared_ptr<TransferJob>& tj);
  int getType() const;
  const Path & getSrcPath() const;
  const Path & getDstPath() const;
  const Path & getPath(bool source) const;
  std::string getSrcSection() const;
  std::string getDstSection() const;
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator srcFileListsBegin() const;
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator srcFileListsEnd() const;
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator dstFileListsBegin() const;
  std::unordered_map<std::string, std::shared_ptr<FileList>>::const_iterator dstFileListsEnd() const;
  std::unordered_map<std::string, std::shared_ptr<LocalFileList>>::const_iterator localFileListsBegin() const;
  std::unordered_map<std::string, std::shared_ptr<LocalFileList>>::const_iterator localFileListsEnd() const;
  std::list<std::shared_ptr<TransferStatus>>::const_iterator transfersBegin() const;
  std::list<std::shared_ptr<TransferStatus>>::const_iterator transfersEnd() const;
  std::unordered_map<std::string, unsigned long long int>::const_iterator pendingTransfersBegin() const;
  std::unordered_map<std::string, unsigned long long int>::const_iterator pendingTransfersEnd() const;
  bool isDone() const;
  bool isDirectory() const;
  TransferJobStatus getStatus() const;
  bool wantsList(bool source);
  std::shared_ptr<LocalFileList> wantedLocalDstList(const std::string &);
  std::shared_ptr<FileList> getListTarget(bool source) const;
  void fileListUpdated(bool source, const std::shared_ptr<FileList>& fl);
  std::shared_ptr<FileList> findDstList(const std::string &) const;
  std::shared_ptr<FileList> getFileListForFullPath(bool source, const Path &) const;
  std::shared_ptr<LocalFileList> findLocalFileList(const std::string &) const;
  const std::shared_ptr<SiteLogic> & getSrc() const;
  const std::shared_ptr<SiteLogic> & getDst() const;
  SkipList* getDstSectionSkipList() const;
  int maxSlots() const;
  void setSlots(int);
  int maxPossibleSlots() const;
  bool listsRefreshed() const;
  bool refreshOrAlmostDone();
  void clearRefreshLists();
  void start();
  void addPendingTransfer(const Path &, unsigned long long int);
  void addTransfer(const std::shared_ptr<TransferStatus> &);
  void targetExists(const Path &);
  int getProgress() const;
  int getMilliProgress() const;
  int timeSpent() const;
  int timeRemaining() const;
  unsigned long long int sizeProgress() const;
  unsigned long long int totalSize() const;
  unsigned int getSpeed() const;
  std::string timeQueued() const;
  std::string timeStarted() const;
  std::string typeString() const;
  int filesProgress() const;
  int filesTotal() const;
  unsigned int getId() const;
  void abort();
  void clearExisting();
  bool hasFailedTransfer(const std::string &) const;
  bool anyListNeedsRefreshing() const;
  std::shared_ptr<SiteTransferJob> & getSrcTransferJob();
  std::shared_ptr<SiteTransferJob> & getDstTransferJob();
  void updateStatus();
  void reset();
private:
  void downloadJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcsection, const std::string& srcfile, const Path& dstpath, const std::string& dstfile);
  void uploadJob(unsigned int id, const Path& srcpath, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstsection, const std::string& dstfile);
  void fxpJob(unsigned int id, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<FileList>& srcfilelist, const std::string& srcsection, const std::string& srcfile, const std::shared_ptr<SiteLogic>& dstsl, const std::shared_ptr<FileList>& dstfilelist, const std::string& dstsection, const std::string& dstfile);
  void addTransferAttempt(const std::shared_ptr<TransferStatus> &, bool);
  void addSubDirectoryFileLists(std::unordered_map<std::string, std::shared_ptr<FileList>>&, const std::shared_ptr<FileList>& fl, const Path &);
  void addSubDirectoryFileLists(std::unordered_map<std::string, std::shared_ptr<FileList>>&, const std::shared_ptr<FileList>& fl, const Path &, File *);
  void init(unsigned int id, TransferJobType type, const std::shared_ptr<SiteLogic>& srcsl, const std::shared_ptr<SiteLogic>& dstsl, const Path& srcpath, const Path& dstpath, const std::string& srcsection, const std::string& dstsection, const std::string& srcfile, const std::string& dstfile);
  void countTotalFiles();
  void setDone();
  void updateLocalFileLists();
  void updateLocalFileLists(const Path &, const Path &);
  bool checkFileListExists(const std::shared_ptr<FileList>& fl) const;
  void resetValues();
  CallbackType callbackType() const override;
  void transferSuccessful(const std::shared_ptr<TransferStatus> &) override;
  void transferFailed(const std::shared_ptr<TransferStatus> &, int) override;
  void tick(int message) override;
  int type;
  std::shared_ptr<SiteLogic> src;
  std::shared_ptr<SiteLogic> dst;
  Path srcpath;
  Path dstpath;
  std::string srcsection;
  std::string dstsection;
  SkipList* dstsectionskiplist;
  std::string srcfile;
  std::string dstfile;
  std::unordered_map<std::string, std::shared_ptr<FileList>> srcfilelists;
  std::unordered_map<std::string, std::shared_ptr<FileList>> dstfilelists;
  std::unordered_map<std::string, std::shared_ptr<LocalFileList> > localfilelists;
  std::unordered_map<std::string, unsigned long long int> pendingtransfers;
  std::unordered_set<std::string> existingtargets;
  std::list<std::shared_ptr<TransferStatus> > transfers;
  int slots;
  TransferJobStatus status;
  std::shared_ptr<FileList> srclisttarget;
  std::shared_ptr<FileList> dstlisttarget;
  std::unordered_map<std::shared_ptr<FileList>, int> filelistsrefreshed;
  unsigned long long int expectedfinalsize;
  unsigned int speed;
  unsigned long long int sizeprogress;
  int progress;
  int milliprogress;
  int timespentmillis;
  int timespentsecs;
  int timeremaining;
  std::string timequeued;
  std::string timestarted;
  int filesprogress;
  int filestotal;
  bool almostdone;
  unsigned int id;
  std::unordered_map<std::string, int> transferattempts;
  int idletime;
  std::shared_ptr<SiteTransferJob> srcsitetransferjob;
  std::shared_ptr<SiteTransferJob> dstsitetransferjob;
};
