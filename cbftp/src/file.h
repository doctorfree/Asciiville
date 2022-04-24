#pragma once

#include <memory>
#include <string>

class Site;
class CommandOwner;

class File {
  friend class FileList;
private:
  File(const File &);
  bool parseUNIXSTATLine(const std::string &);
  bool parseBrokenUNIXSTATLine(const std::string &, size_t, size_t &);
  bool parseMSDOSSTATLine(const std::string &);
  std::string digitsOnly(const std::string &) const;
  std::string name;
  std::string linktarget;
  std::string extension;
  unsigned long long int size;
  std::string owner;
  std::string group;
  std::string lastmodified;
  unsigned int updatespeed;
  std::shared_ptr<Site> updatesrc;
  std::shared_ptr<Site> updatedst;
  std::shared_ptr<CommandOwner> updatecosrc;
  std::shared_ptr<CommandOwner> updatecodst;
  bool updateflag;
  bool directory;
  bool softlink;
  int touch;
  bool uploading;
  int downloading;
  bool valid;
protected:
  File(const std::string &, const std::string &);
  File(const std::string &, int);
  void setUpdateFlag(const std::shared_ptr<Site> &, const std::shared_ptr<Site> &, const std::shared_ptr<CommandOwner> & srcco, const std::shared_ptr<CommandOwner> & dstco, unsigned int);
  void unsetUpdateFlag();
  bool setSize(unsigned long long int);
  bool setLastModified(const std::string &);
  bool setOwner(const std::string &);
  bool setGroup(const std::string &);
  void setTouch(int);
  void download();
  void upload();
  void finishUpload();
  void finishDownload();
public:
  static std::string getExtension(const std::string &);
  bool isDirectory() const;
  bool isLink() const;
  std::string getOwner() const;
  std::string getGroup() const;
  unsigned long long int getSize() const;
  std::string getLastModified() const;
  std::string getName() const;
  std::string getLinkTarget() const;
  std::string getExtension() const;
  const std::shared_ptr<Site> & getUpdateSrc() const;
  const std::shared_ptr<Site> & getUpdateDst() const;
  const std::shared_ptr<CommandOwner> & getUpdateSrcCommandOwner() const;
  const std::shared_ptr<CommandOwner> & getUpdateDstCommandOwner() const;
  unsigned int getUpdateSpeed() const;
  bool updateFlagSet() const;
  bool isDownloading() const;
  bool isUploading() const;
  int getTouch() const;
  bool isValid() const;
};
