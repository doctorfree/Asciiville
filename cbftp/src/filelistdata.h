#pragma once

#include <memory>

#include "rawbuffer.h"

class FileList;

class FileListData {
public:
  FileListData(const std::shared_ptr<FileList>& filelist, RawBuffer* cwdrawbuffer);
  std::shared_ptr<FileList> getFileList() const;
  const RawBuffer& getCwdRawBuffer() const;
private:
  std::shared_ptr<FileList> filelist;
  RawBuffer cwdrawbuffer;
};
