#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>

class CommandOwner;
class ScoreBoardElement;
class TransferMonitor;
class TransferStatus;
class SiteLogic;
class FileList;
class TransferStatus;
class LocalFileList;

class TransferManager {
  private:
    std::list<std::shared_ptr<TransferMonitor> > transfermonitors;
    std::list<std::shared_ptr<TransferStatus> > ongoingtransfers;
    std::list<std::shared_ptr<TransferStatus> > finishedtransfers;
    unsigned int totalfinishedtransfers;
    std::shared_ptr<TransferMonitor> getAvailableTransferMonitor();
    void moveTransferStatusToFinished(const std::shared_ptr<TransferStatus> &);
    int maxtransferhistory;
    int maxtransfertimeseconds;
  public:
    TransferManager();
    ~TransferManager();
    void getFileList(const std::shared_ptr<SiteLogic> &, int, bool, const std::shared_ptr<FileList>& fl, bool ipv6, const std::shared_ptr<CommandOwner> & co = std::shared_ptr<CommandOwner>());
    std::shared_ptr<TransferStatus> attemptTransfer(
      const std::string &, const std::shared_ptr<SiteLogic> &, const std::shared_ptr<FileList>& fls,
      const std::shared_ptr<SiteLogic> &, const std::shared_ptr<FileList>& fld, const std::shared_ptr<CommandOwner> &, const std::shared_ptr<CommandOwner> &);
    std::shared_ptr<TransferStatus> attemptTransfer(
      const std::string &, const std::shared_ptr<SiteLogic> &, const std::shared_ptr<FileList>& fls,
      const std::string &, const std::shared_ptr<SiteLogic> &, const std::shared_ptr<FileList>& fld,
      const std::shared_ptr<CommandOwner> &, const std::shared_ptr<CommandOwner> &);
    std::shared_ptr<TransferStatus> attemptDownload(
      const std::string &, const std::shared_ptr<SiteLogic> &,const std::shared_ptr<FileList>& fls,
      const std::shared_ptr<LocalFileList> &, const std::shared_ptr<CommandOwner> & co = std::shared_ptr<CommandOwner>());
    std::shared_ptr<TransferStatus> attemptDownload(const std::string& file, const std::shared_ptr<SiteLogic>& sl,
      int srcslotid, const std::shared_ptr<FileList>& fls, const std::shared_ptr<LocalFileList>& localfl = nullptr);
    std::shared_ptr<TransferStatus> attemptUpload(
      const std::string &, const std::shared_ptr<LocalFileList> &,
      const std::shared_ptr<SiteLogic> &, const std::shared_ptr<FileList>& fld, const std::shared_ptr<CommandOwner> &);
    void transferSuccessful(const std::shared_ptr<TransferStatus> &);
    void transferFailed(const std::shared_ptr<TransferStatus> &, int);
    std::list<std::shared_ptr<TransferStatus> >::const_iterator ongoingTransfersBegin() const;
    std::list<std::shared_ptr<TransferStatus> >::const_iterator ongoingTransfersEnd() const;
    std::list<std::shared_ptr<TransferStatus> >::const_iterator finishedTransfersBegin() const;
    std::list<std::shared_ptr<TransferStatus> >::const_iterator finishedTransfersEnd() const;
    unsigned int ongoingTransfersSize() const;
    unsigned int finishedTransfersSize() const;
    unsigned int totalFinishedTransfers() const;
    void addNewTransferStatus(const std::shared_ptr<TransferStatus> &);
    int getMaxTransferHistory() const;
    void setMaxTransferHistory(int history);
    int getMaxTransferTimeSeconds() const;
    void setMaxTransferTimeSeconds(int seconds);
};
