#include "externalfileviewing.h"

#include <cstdlib>

#include "../globalcontext.h"
#include "../localstorage.h"
#include "../eventlog.h"
#include "../path.h"
#include "../file.h"

ExternalFileViewing::ExternalFileViewing() {
  videoviewer = "mplayer";
  audioviewer = "mplayer";
  imageviewer = "eog";
  pdfviewer = "evince";
  display = false;
  char * displayenv = getenv("DISPLAY");
  if (displayenv != nullptr) {
    display = true;
  }
}

bool ExternalFileViewing::isViewable(const Path & path) const {
  return getViewApplication(path) != "";
}

int ExternalFileViewing::view(const Path & path) {
  return view(path, false);
}

int ExternalFileViewing::viewThenDelete(const Path & path) {
  return view(path, true);
}

int ExternalFileViewing::view(const Path & path, bool deleteafter) {
  std::string application = getViewApplication(path);
  global->getEventLog()->log("ExternalFileViewing", "Opening " + path.toString() + " with " + application);
  std::vector<std::string> args = {path.toString()};
  std::shared_ptr<SubProcess> subprocess = global->getSubProcessManager()->runProcess(this, application, args);
  if (deleteafter) {
    files[subprocess->pid] = path;
  }
  return subprocess->pid;
}

void ExternalFileViewing::killViewer(int pid) {
  global->getSubProcessManager()->killProcess(pid);
  checkDeleteFile(pid);
}

std::string ExternalFileViewing::getViewApplication(const Path & path) const {
  std::string extension = File::getExtension(path.baseName());
  std::string application;
  if (extension == "mkv" || extension == "mp4" || extension == "avi" ||
      extension == "wmv" || extension == "vob" || extension == "mov" ||
      extension == "mpg" || extension == "mpeg") {
    application = getVideoViewer();
  }
  else if (extension == "mp3" || extension == "wav" || extension == "flac" ||
      extension == "ogg" || extension == "wma" || extension == "mid") {
    application = getAudioViewer();
  }
  else if (extension == "png" || extension == "gif" || extension == "jpeg" ||
      extension == "jpg" || extension == "bmp") {
    application = getImageViewer();
  }
  else if (extension == "pdf") {
    application = getPDFViewer();
  }
  return application;
}

void ExternalFileViewing::checkDeleteFile(int pid) {
  if (files.find(pid) != files.end()) {
    global->getEventLog()->log("ExternalFileViewing", "Deleting temporary file: " + files[pid].toString());
    global->getLocalStorage()->deleteFile(files[pid]);
    files.erase(pid);
  }
}

bool ExternalFileViewing::hasDisplay() const {
  return display;
}

bool ExternalFileViewing::stillViewing(int pid) const {
  return global->getSubProcessManager()->getIsRunning(pid);
}

void ExternalFileViewing::processExited(int pid, int status) {
  checkDeleteFile(pid);
}

std::string ExternalFileViewing::getVideoViewer() const {
  return videoviewer;
}

std::string ExternalFileViewing::getAudioViewer() const {
  return audioviewer;
}

std::string ExternalFileViewing::getImageViewer() const {
  return imageviewer;
}

std::string ExternalFileViewing::getPDFViewer() const {
  return pdfviewer;
}

void ExternalFileViewing::setVideoViewer(const std::string & viewer) {
  videoviewer = viewer;
}

void ExternalFileViewing::setAudioViewer(const std::string & viewer) {
  audioviewer = viewer;
}

void ExternalFileViewing::setImageViewer(const std::string & viewer) {
  imageviewer = viewer;
}

void ExternalFileViewing::setPDFViewer(const std::string & viewer) {
  pdfviewer = viewer;
}
