#pragma once

#include <list>
#include <utility>
#include <string>

#include "browsescreensub.h"

#include "../uiwindow.h"
#include "../uifilelist.h"
#include "../menuselectoption.h"
#include "../menuselectoptiontextfield.h"
#include "../../path.h"
#include "../../rawbuffer.h"

class BrowseScreen;
class SiteLogic;
class Site;
class FileList;
class Ui;
class VirtualView;
class UIFile;
class BrowseScreenAction;
class KeyBinds;

enum class ConfirmAction {
  NONE,
  DELETE,
  WIPE
};

class BrowseScreenSite : public BrowseScreenSub {
public:
  BrowseScreenSite(Ui* ui, BrowseScreen* parent, KeyBinds& keybinds,
      const std::string& sitestr, const Path& path = Path());
  BrowseScreenSite(Ui* ui, BrowseScreen* parent, KeyBinds& keybinds,
      const std::string& sitestr, const std::string& section);
  ~BrowseScreenSite();
  BrowseScreenType type() const override;
  void redraw(unsigned int, unsigned int, unsigned int) override;
  void update() override;
  void command(const std::string & command, const std::string & arg) override;
  BrowseScreenAction keyPressed(unsigned int) override;
  std::string getLegendText(int scope) const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  void setFocus(bool) override;
  void tick(int) override;
  std::string siteName() const;
  std::shared_ptr<FileList> fileList() const;
  UIFile * selectedFile() const;
  UIFileList * getUIFileList() override;
  const std::shared_ptr<Site> & getSite() const;
  void gotoPath(const Path & path);
  void initiateMove(const std::string& dstpath) override;
  void refreshFileList() override;
  BrowseScreenAction tryJumpSection(const std::string& section) override;
  std::string getLastJumpSection() const;
  Path getLastJumpPath() const;
private:
  Ui* ui;
  VirtualView* vv;
  BrowseScreen* parent;
  unsigned int row;
  unsigned int col;
  unsigned int coloffset;
  MenuSelectOption table;
  unsigned int currentviewspan;
  bool resort;
  mutable int tickcount;
  std::list<BrowseScreenRequest> requests;
  bool gotomode;
  bool gotomodefirst;
  int gotomodeticker;
  bool filtermodeinput;
  bool filtermodeinputregex;
  bool gotopathinput;
  std::string gotomodestring;
  UIFileList::SortMethod sortmethod;
  std::shared_ptr<SiteLogic> sitelogic;
  std::shared_ptr<Site> site;
  UIFileList list;
  mutable int spinnerpos;
  std::shared_ptr<FileList> filelist;
  bool withinraceskiplistreach;
  Path closestracesectionpath;
  std::string closestracesection;
  std::string separatortext;
  std::list<std::pair<Path, std::string> > selectionhistory;
  bool focus;
  MenuSelectOptionTextField bottomlinetextfield;
  bool temphighlightline;
  RawBuffer cwdrawbuffer;
  bool softselecting;
  LastInfo lastinfo;
  std::string lastinfotarget;
  ConfirmAction confirmaction;
  bool refreshfilelistafter;
  std::string lastjumpsection;
  Path lastjumppath;
  bool nameonly;
  void disableGotoMode();
  void clearSoftSelects();
  bool handleReadyRequests();
  void loadFileListFromRequest();
  void viewCursored();
  bool keyDown();
};
