#pragma once

#include <memory>
#include <string>

#include "commandowner.h"

class FileList;
class SiteLogic;
class TransferJob;

class SiteTransferJob : public CommandOwner {
public:
  SiteTransferJob(const std::shared_ptr<TransferJob>& tj, bool source);
  std::weak_ptr<TransferJob> getTransferJob();
  std::shared_ptr<SiteLogic> getOtherSiteLogic() const;
  bool wantsList();
  bool otherWantsList();
  bool isDone() const;
  Path getPath() const;
  std::shared_ptr<FileList> getListTarget();
  int classType() const override;
  std::string getName() const override;
  unsigned int getId() const override;
  void fileListUpdated(SiteLogic* sl, const std::shared_ptr<FileList>& fl) override;
  std::shared_ptr<FileList> getFileListForFullPath(SiteLogic* sl, const Path& path) const override;
private:
  std::weak_ptr<TransferJob> transferjob;
  bool source;
};
