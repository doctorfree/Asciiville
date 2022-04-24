#pragma once

#include <memory>

#include "core/eventreceiver.h"
#include "statistics.h"

#define COMMANDOWNER_SITERACE 543
#define COMMANDOWNER_TRANSFERJOB 544

class FileList;
class SiteLogic;
class Path;

class CommandOwner : public Core::EventReceiver {
public:
  virtual ~CommandOwner() {
  }
  virtual int classType() const = 0;
  virtual std::string getName() const = 0;
  virtual unsigned int getId() const = 0;
  virtual void fileListUpdated(SiteLogic* sl, const std::shared_ptr<FileList>& fl) = 0;
  virtual std::shared_ptr<FileList> getFileListForFullPath(SiteLogic* sl, const Path& path) const = 0;
  virtual void addTransferStatsFile(StatsDirection, const std::string &, unsigned long long int, unsigned int) { }
};
