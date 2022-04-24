#include "filelistdata.h"

FileListData::FileListData(const std::shared_ptr<FileList>& filelist, RawBuffer* cwdrawbuffer)
  : filelist(filelist), cwdrawbuffer(*cwdrawbuffer)
{

}

std::shared_ptr<FileList> FileListData::getFileList() const {
  return filelist;
}

const RawBuffer& FileListData::getCwdRawBuffer() const {
  return cwdrawbuffer;
}
