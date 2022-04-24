#include "pendingtransfer.h"

#include "filelist.h"
#include "localfilelist.h"

PendingTransfer::PendingTransfer(const std::shared_ptr<SiteLogic> & slsrc, const std::shared_ptr<FileList>& flsrc, std::string srcfilename, const std::shared_ptr<SiteLogic> & sldst, const std::shared_ptr<FileList>& fldst, std::string dstfilename) :
  slsrc(slsrc),
  sldst(sldst),
  flsrc(flsrc),
  fldst(fldst),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_FXP) {
}

PendingTransfer::PendingTransfer(const std::shared_ptr<SiteLogic> & sl, const std::shared_ptr<FileList>& fl, std::string srcfilename, std::shared_ptr<LocalFileList> fllocal, std::string dstfilename) :
  slsrc(sl),
  flsrc(fl),
  fllocal(fllocal),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_DOWNLOAD) {
}

PendingTransfer::PendingTransfer(std::shared_ptr<LocalFileList> fllocal, std::string srcfilename, const std::shared_ptr<SiteLogic> & sl, const std::shared_ptr<FileList>& fl, std::string dstfilename) :
  sldst(sl),
  fldst(fl),
  fllocal(fllocal),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_UPLOAD) {
}

PendingTransfer::~PendingTransfer() {

}

int PendingTransfer::type() const {
  return transfertype;
}

const std::shared_ptr<SiteLogic> & PendingTransfer::getSrc() const {
  return slsrc;
}

const std::shared_ptr<SiteLogic> & PendingTransfer::getDst() const {
  return sldst;
}

std::shared_ptr<FileList> PendingTransfer::getSrcFileList() const {
  return flsrc;
}

std::shared_ptr<FileList> PendingTransfer::getDstFileList() const {
  return fldst;
}

std::shared_ptr<LocalFileList> & PendingTransfer::getLocalFileList() {
  return fllocal;
}

std::string PendingTransfer::getSrcFileName() const {
  return srcfilename;
}

std::string PendingTransfer::getDstFileName() const {
  return dstfilename;
}
