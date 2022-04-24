#include "localstoragerequestdata.h"

LocalStorageRequestData::LocalStorageRequestData() : requestid(-1), care(true)
{

}

FileListTaskData::FileListTaskData() {
  type = LocalStorageRequestType::GET_FILE_LIST;
}

PathInfoTaskData::PathInfoTaskData() {
  type = LocalStorageRequestType::GET_PATH_INFO;
}

DeleteFileTaskData::DeleteFileTaskData() {
  type = LocalStorageRequestType::DELETE;
}

MakeDirTaskData::MakeDirTaskData() {
  type = LocalStorageRequestType::MKDIR;
}

MoveTaskData::MoveTaskData() {
  type = LocalStorageRequestType::MOVE;
}
