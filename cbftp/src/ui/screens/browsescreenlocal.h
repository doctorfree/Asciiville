#pragma once

#include <memory>
#include <string>
#include <list>
#include <utility>

#include "browsescreensub.h"

#include "../menuselectoption.h"
#include "../uifilelist.h"
#include "../menuselectoptiontextfield.h"

#include "../../localstorage.h"
#include "../../path.h"

class Ui;
class VirtualView;
class LocalFileList;
class KeyBinds;

class BrowseScreenLocal : public BrowseScreenSub {
public:
  BrowseScreenLocal(Ui *, KeyBinds& keybinds, const Path& path = Path());
  ~BrowseScreenLocal();
  BrowseScreenType type() const override;
  void redraw(unsigned int, unsigned int, unsigned int) override;
  void update() override;
  void command(const std::string &, const std::string &) override;
  BrowseScreenAction keyPressed(unsigned int) override;
  std::string getLegendText(int scope) const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  void setFocus(bool) override;
  void tick(int) override;
  std::shared_ptr<LocalFileList> fileList() const;
  UIFile * selectedFile() const;
  UIFileList * getUIFileList() override;
  void initiateMove(const std::string& dstpath) override;
private:
  void refreshFilelist();
  bool handleReadyRequests();
  void disableGotoMode();
  void gotoPath(int requestid, const Path & path);
  void clearSoftSelects();
  void viewCursored();
  bool keyDown();
  Ui* ui;
  VirtualView* vv;
  unsigned int row;
  unsigned int col;
  unsigned int coloffset;
  unsigned int currentviewspan;
  bool focus;
  MenuSelectOption table;
  UIFileList list;
  std::list<BrowseScreenRequest> requests;
  mutable int spinnerpos;
  mutable int tickcount;
  bool resort;
  UIFileList::SortMethod sortmethod;
  bool gotomode;
  bool gotomodefirst;
  int gotomodeticker;
  bool filtermodeinput;
  bool filtermodeinputregex;
  bool gotopathinput;
  std::string gotomodestring;
  std::list<std::pair<Path, std::string> > selectionhistory;
  std::shared_ptr<LocalFileList> filelist;
  MenuSelectOptionTextField bottomlinetextfield;
  bool temphighlightline;
  bool softselecting;
  LastInfo lastinfo;
  std::string lastinfotarget;
  bool refreshfilelistafter;
  unsigned long long int freespace;
  bool nameonly;
};
