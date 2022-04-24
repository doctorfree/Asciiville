#pragma once

#include <memory>
#include <string>

#include "core/types.h"

class FileList;
class TransferStatus;

struct DownloadFileData {
  ~DownloadFileData();
  std::shared_ptr<FileList> fl;
  std::shared_ptr<TransferStatus> ts;
  Core::BinaryData data;
  std::string file;
  bool inmemory;
};
