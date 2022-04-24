#pragma once

#include <list>
#include <map>
#include <memory>
#include <utility>
#include <string>

#include "../uiwindow.h"
#include "../keybinds.h"
#include "../../path.h"

class BrowseScreenSub;
enum class CompareMode;
class ExternalScripts;

class BrowseScreen : public UIWindow {
public:
  BrowseScreen(Ui* ui);
  ~BrowseScreen();
  void initializeSite(unsigned int row, unsigned int col,
      const std::string& site, const Path& path = Path());
  void initializeSite(unsigned int row, unsigned int col,
      const std::string& site, const std::string& section);
  void initializeLocal(unsigned int row, unsigned int col,
      const Path& path = Path());
  void initializeSplit(unsigned int row, unsigned int col,
      const std::string& site);
  void initializeSiteLocal(unsigned int row, unsigned int col,
      const std::string& site, const Path& sitepath = Path(),
      const Path& localpath = Path());
  void initializeSiteSite(unsigned int row, unsigned int col,
      const std::string& leftsite, const std::string& rightsite,
      const Path& leftpath = Path(), const Path& rightpath = Path());
  void redraw();
  void update();
  void command(const std::string& command, const std::string& arg);
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  bool isInitialized() const;
  void suggestOtherRefresh(BrowseScreenSub* sub);
private:
  void switchSide();
  void closeSide();
  bool keyPressedNoSubAction(unsigned int);
  void toggleCompareListMode(CompareMode mode);
  void clearCompareListMode();
  void jumpSectionHotkey(int hotkey);
  void goSite(bool leftside, const std::string& site, const std::string& section = "");
  void loadExtraKeyBinds();
  std::shared_ptr<BrowseScreenSub> left;
  std::shared_ptr<BrowseScreenSub> right;
  std::shared_ptr<BrowseScreenSub> active;
  bool split;
  bool initsplitupdate;
  KeyBinds sitekeybinds;
  KeyBinds localkeybinds;
  std::shared_ptr<ExternalScripts> externalscripts;
  std::list<std::pair<int, std::string>> externalscriptbinds;
  bool reloadextrakeybinds;
};
