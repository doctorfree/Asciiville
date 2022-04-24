#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>

#include "core/eventreceiver.h"
#include "core/types.h"
#include "path.h"
#include "localpathinfo.h"
#include "localfile.h"
#include "localstoragerequestdata.h"
#include "transferprotocol.h"
#include "util.h"

class LocalTransfer;
class LocalDownload;
class LocalUpload;
class TransferMonitor;
class FTPConn;
class LocalFileList;

#define MAXREAD 524288

class LocalStorage : public Core::EventReceiver {
public:
  LocalStorage();
  ~LocalStorage();
  LocalTransfer * passiveModeDownload(TransferMonitor* tm, const std::string& file, bool ipv6, const std::string& host, int port, bool ssl, FTPConn* ftpconn);
  LocalTransfer * passiveModeDownload(TransferMonitor* tm, const Path& path, const std::string & file, bool ipv6, const std::string & host, int port, bool ssl, FTPConn* ftpconn);
  LocalTransfer * passiveModeDownload(TransferMonitor* tm, bool ipv6, const std::string& host, int port, bool ssl, FTPConn* ftpconn);
  LocalTransfer * passiveModeUpload(TransferMonitor* tm, const Path& path, const std::string& file, bool ipv6, const std::string& host, int port, bool ssl, FTPConn* ftpconn);
  LocalTransfer * activeModeDownload(TransferMonitor* tm, const Path& path, const std::string& file, bool ipv6, bool ssl, FTPConn* ftpconn);
  LocalTransfer * activeModeDownload(TransferMonitor* tm, bool ipv6, bool ssl, FTPConn* ftpconn);
  LocalTransfer * activeModeUpload(TransferMonitor* tm, const Path& path, const std::string& file, bool ipv6, bool ssl, FTPConn* ftpconn);
  Core::BinaryData getTempFileContent(const std::string &) const;
  Core::BinaryData getFileContent(const Path &) const;
  const Core::BinaryData & getStoreContent(int) const;
  void purgeStoreContent(int);
  int requestDelete(const Path & filename, bool care = false);
  bool deleteFile(const Path & filename);
  static bool deleteFileAbsolute(const Path & filename);
  static bool deleteRecursive(const Path & path);
  int requestMakeDirectory(const Path& path, const std::string& dirname);
  int requestMove(const Path& srcpath, const Path& dstpath);
  util::Result getMakeDirResult(int requestid);
  util::Result getMoveResult(int requestid);
  bool getDeleteResult(int requestid);
  static LocalPathInfo getPathInfo(const Path & path);
  static LocalPathInfo getPathInfo(const std::list<Path> & paths);
  int requestPathInfo(const Path & path);
  int requestPathInfo(const std::list<Path> & paths);
  LocalPathInfo getPathInfo(int requestid);
  static LocalFile getLocalFile(const Path & path);
  const Path & getTempPath() const;
  void setTempPath(const std::string &);
  void storeContent(int, const Core::BinaryData &);
  const Path & getDownloadPath() const;
  void setDownloadPath(const Path &);
  static std::shared_ptr<LocalFileList> getLocalFileList(const Path & path);
  static bool updateLocalFileList(const std::shared_ptr<LocalFileList>& filelist);
  int requestLocalFileList(const Path & path);
  bool requestReady(int requestid) const;
  std::shared_ptr<LocalFileList> getLocalFileList(int requestid);
  void asyncTaskComplete(int type, void * data);
  unsigned long long int getFileSize(const Path &);
  bool getUseActiveModeAddress() const;
  const std::string& getActiveModeAddress4() const;
  const std::string& getActiveModeAddress6() const;
  int getActivePortFirst() const;
  int getActivePortLast() const;
  void setUseActiveModeAddress(bool);
  void setActiveModeAddress4(const std::string& address);
  void setActiveModeAddress6(const std::string& address);
  void setActivePortFirst(int);
  void setActivePortLast(int);
  int getNextActivePort();
  Core::StringResult getAddress4(int fallbacksockid) const;
  Core::StringResult getAddress6(int fallbacksockid) const;
  TransferProtocol getTransferProtocol() const;
  void setTransferProtocol(TransferProtocol protocol);
  void executeAsyncRequest(LocalStorageRequestData * data);
private:
  void deleteRequestData(LocalStorageRequestData * reqdata);
  static LocalPathInfo getPathInfo(const Path & path, int currentdepth);
  std::map<int, Core::BinaryData> content;
  LocalDownload * getAvailableLocalDownload();
  LocalUpload * getAvailableLocalUpload();
  std::list<LocalDownload *> localdownloads;
  std::list<LocalUpload *> localuploads;
  std::map<int, LocalStorageRequestData *> readyrequests;
  Path temppath;
  Path downloadpath;
  int storeidcounter;
  bool useactivemodeaddress;
  std::string activemodeaddress4;
  std::string activemodeaddress6;
  int activeportfirst;
  int activeportlast;
  int currentactiveport;
  int requestidcounter;
  TransferProtocol transferprotocol;
};
