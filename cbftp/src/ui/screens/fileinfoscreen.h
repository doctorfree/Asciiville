#pragma once

#include "../uiwindow.h"

class UIFile;

class FileInfoScreen : public UIWindow {
public:
  FileInfoScreen(Ui* ui);
  void initialize(unsigned int row, unsigned int col, UIFile* uifile);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  UIFile* uifile;
};
