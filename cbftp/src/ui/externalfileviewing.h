#pragma once

#include <string>
#include <map>

#include "../subprocessmanager.h"

class Path;

class ExternalFileViewing : public SubProcessCallback {
public:
  ExternalFileViewing();
  bool isViewable(const Path &) const;
  int view(const Path &);
  int viewThenDelete(const Path &);
  void killViewer(int pid);
  bool hasDisplay() const;
  bool stillViewing(int) const;
  std::string getVideoViewer() const;
  std::string getAudioViewer() const;
  std::string getImageViewer() const;
  std::string getPDFViewer() const;
  void setVideoViewer(const std::string &);
  void setAudioViewer(const std::string &);
  void setImageViewer(const std::string &);
  void setPDFViewer(const std::string &);
  std::string getViewApplication(const Path &) const;
private:
  int view(const Path &, bool);
  void checkDeleteFile(int);
  void processExited(int pid, int status) override;
  std::map<int, Path> files;
  std::string videoviewer;
  std::string audioviewer;
  std::string imageviewer;
  std::string pdfviewer;
  bool display;
};
