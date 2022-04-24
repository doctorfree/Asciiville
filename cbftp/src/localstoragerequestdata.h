#include <memory>

#include "path.h"
#include "util.h"

class LocalFileList;
class LocalPathInfo;

enum class LocalStorageRequestType {
  GET_FILE_LIST,
  GET_PATH_INFO,
  DELETE,
  MKDIR,
  MOVE
};

struct LocalStorageRequestData {
  LocalStorageRequestData();
  int requestid;
  bool care;
  LocalStorageRequestType type;
};

struct FileListTaskData : public LocalStorageRequestData {
  FileListTaskData();
  Path path;
  std::shared_ptr<LocalFileList> filelist;
};

struct PathInfoTaskData : public LocalStorageRequestData {
  PathInfoTaskData();
  std::list<Path> paths;
  std::shared_ptr<LocalPathInfo> pathinfo;
};

struct DeleteFileTaskData : public LocalStorageRequestData {
  DeleteFileTaskData();
  Path file;
  bool success;
};

struct MakeDirTaskData : public LocalStorageRequestData {
  MakeDirTaskData();
  Path path;
  std::string dirname;
  util::Result result;
};

struct MoveTaskData : public LocalStorageRequestData {
  MoveTaskData();
  Path srcpath;
  Path dstpath;
  util::Result result;
};
