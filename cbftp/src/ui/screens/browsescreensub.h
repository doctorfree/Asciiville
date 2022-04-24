#pragma once

#include <list>
#include <string>
#include <memory>

#include "../../core/eventreceiver.h"

#include "../../path.h"

class BrowseScreenAction;
class KeyBinds;
class ResizableElement;
class MenuSelectOption;
class UIFileList;
class UIFile;
class Ui;

enum class BrowseScreenType {
  SITE,
  LOCAL,
  SELECTOR
};

enum class LastInfo {
  CWD_FAILED,
  CHANGED_SORT,
  DELETE_SUCCESS,
  DELETE_FAILED,
  WIPE_SUCCESS,
  WIPE_FAILED,
  NUKE_SUCCESS,
  NUKE_FAILED,
  MKDIR_SUCCESS,
  MKDIR_FAILED,
  MOVE_SUCCESS,
  MOVE_FAILED,
  NONE
};

enum class BrowseScreenRequestType {
  FILELIST,
  PATH_INFO,
  DELETE,
  WIPE,
  NUKE,
  MKDIR,
  MOVE
};

enum BrowseScreenKeyAction {
  KEYACTION_RETURN,
  KEYACTION_SPREAD,
  KEYACTION_VIEW_FILE,
  KEYACTION_DOWNLOAD,
  KEYACTION_BIND_SECTION,
  KEYACTION_SORT,
  KEYACTION_SORT_DEFAULT,
  KEYACTION_RAW_COMMAND,
  KEYACTION_WIPE,
  KEYACTION_NUKE,
  KEYACTION_MKDIR,
  KEYACTION_TOGGLE_SEPARATORS,
  KEYACTION_COMMAND_LOG,
  KEYACTION_SOFT_SELECT_UP,
  KEYACTION_SOFT_SELECT_DOWN,
  KEYACTION_HARD_SELECT,
  KEYACTION_SELECT_ALL,
  KEYACTION_MOVE,
  KEYACTION_REFRESH,
  KEYACTION_SPLIT,
  KEYACTION_TRANSFER,
  KEYACTION_COMPARE_UNIQUE,
  KEYACTION_COMPARE_IDENTICAL,
  KEYACTION_TOGGLE_SHOW_NAMES_ONLY
};

enum BrowseScreenKeyScopes {
  KEYSCOPE_SPLIT_SITE_SITE,
  KEYSCOPE_SPLIT_SITE_LOCAL
};

struct BrowseScreenRequest {
  int id;
  BrowseScreenRequestType type;
  Path path;
  std::list<std::pair<std::string, bool>> files;
};

class BrowseScreenSub : protected Core::EventReceiver {
public:
  BrowseScreenSub(KeyBinds& keybinds);
  virtual void redraw(unsigned int, unsigned int, unsigned int) = 0;
  virtual void update() = 0;
  virtual BrowseScreenType type() const = 0;
  virtual std::string getLegendText(int scope) const = 0;
  virtual std::string getInfoLabel() const = 0;
  virtual std::string getInfoText() const = 0;
  virtual void command(const std::string & command, const std::string & arg);
  virtual BrowseScreenAction keyPressed(unsigned int) = 0;
  virtual void setFocus(bool) = 0;
  virtual void refreshFileList();
  virtual UIFileList * getUIFileList();
  static void addFileDetails(MenuSelectOption & table, unsigned int coloffset,
                             unsigned int y, const std::string & name,
                             const std::string & prepchar = "", const std::string & size = "",
                             const std::string & lastmodified = "", const std::string & owner = "",
                             bool selectable = false, bool cursored = false, UIFile * origin = nullptr,
                             bool nameonly = false);
  static void printFlipped(Ui * ui, const std::shared_ptr<ResizableElement> & re);
  static std::string targetName(const std::list<std::pair<std::string, bool>> & items);
  virtual BrowseScreenAction tryJumpSection(const std::string& section);
  virtual void initiateMove(const std::string& dstpath);
protected:
  KeyBinds& keybinds;
};
