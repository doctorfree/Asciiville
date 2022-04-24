#include "fileinfoscreen.h"

#include "../../util.h"

#include "../ui.h"
#include "../uifile.h"

FileInfoScreen::FileInfoScreen(Ui* ui) : UIWindow(ui, "FileInfoScreen"), uifile(nullptr) {
  allowimplicitgokeybinds = false;
}

void FileInfoScreen::initialize(unsigned int row, unsigned int col, UIFile* uifile) {
  this->uifile = uifile;
  init(row, col);
}

void FileInfoScreen::redraw() {
  vv->clear();
  ui->hideCursor();

  unsigned int i = 1;
  vv->putStr(i++, 1, "Name: " + uifile->getName());
  std::string type = "File";
  if (uifile->isDirectory()) {
    type = "Directory";
  }
  else if (uifile->isLink()) {
    type = "Link";
  }
  vv->putStr(i++, 1, "Type: " + type);
  vv->putStr(i++, 1, "Size: " + uifile->getSizeRepr());
  vv->putStr(i++, 1, "Last modified: " + uifile->getLastModified());
  vv->putStr(i++, 1, "Owner: " + uifile->getOwner() + "/" + uifile->getGroup());
  if (uifile->isLink()) {
    vv->putStr(i++, 1, "Link target: " + uifile->getLinkTarget());
  }
}

bool FileInfoScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case ' ':
    case 'c':
    case 10:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string FileInfoScreen::getLegendText() const {
  return "[Esc/Space/Enter/c] Return";
}

std::string FileInfoScreen::getInfoLabel() const {
  return "FILE INFO";
}
