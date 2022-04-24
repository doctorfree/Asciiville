#include "browsescreen.h"

#include <algorithm>
#include <cctype>
#include <memory>

#include "../../globalcontext.h"
#include "../../engine.h"
#include "../../externalscripts.h"
#include "../../externalscriptsmanager.h"
#include "../../localfilelist.h"
#include "../../timereference.h"
#include "../../filelist.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../sectionmanager.h"
#include "../../section.h"

#include "../ui.h"
#include "../uifilelist.h"
#include "../termint.h"

#include "browsescreensub.h"
#include "browsescreensite.h"
#include "browsescreenselector.h"
#include "browsescreenaction.h"
#include "browsescreenlocal.h"

BrowseScreen::BrowseScreen(Ui* ui) : UIWindow(ui, "BrowseScreen"),
  sitekeybinds("BrowseScreenSite"), localkeybinds("BrowseScreenLocal"), externalscripts(std::make_shared<ExternalScripts>(name)),
  reloadextrakeybinds(false)
{
  keybinds.addBind(27, KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind({'c', KEY_LEFT}, KEYACTION_CLOSE, "Close");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Jump top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Jump bottom");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Page up");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Page down");
  keybinds.addBind(10, KEYACTION_ENTER, "Browse");
  keybinds.addBind('b', KEYACTION_BROWSE, "Browse");
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Browse");
  keybinds.addBind('q', KEYACTION_QUICK_JUMP, "Quick jump");
  keybinds.addBind('\t', KEYACTION_SPLIT, "Split/switch side");
  sitekeybinds.addScope(KEYSCOPE_SPLIT_SITE_SITE, "When split browsing site-site");
  sitekeybinds.addScope(KEYSCOPE_SPLIT_SITE_LOCAL, "When split browsing site-local");
  sitekeybinds.addBind('t', KEYACTION_TRANSFER, "Transfer (FXP)", KEYSCOPE_SPLIT_SITE_SITE);
  sitekeybinds.addBind('u', KEYACTION_COMPARE_UNIQUE, "Unique compare", KEYSCOPE_SPLIT_SITE_SITE);
  sitekeybinds.addBind('I', KEYACTION_COMPARE_IDENTICAL, "Identical compare", KEYSCOPE_SPLIT_SITE_SITE);
  sitekeybinds.addBind('t', KEYACTION_TRANSFER, "Transfer (Download)", KEYSCOPE_SPLIT_SITE_LOCAL);
  sitekeybinds.addBind('u', KEYACTION_COMPARE_UNIQUE, "Unique compare", KEYSCOPE_SPLIT_SITE_LOCAL);
  sitekeybinds.addBind('I', KEYACTION_COMPARE_IDENTICAL, "Identical compare", KEYSCOPE_SPLIT_SITE_LOCAL);
  sitekeybinds.addBind(27, KEYACTION_BACK_CANCEL, "Cancel");
  sitekeybinds.addBind('c', KEYACTION_CLOSE, "Close");
  sitekeybinds.addBind(10, KEYACTION_ENTER, "Open");
  sitekeybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Open");
  sitekeybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Return");
  sitekeybinds.addBind(KEY_BACKSPACE, KEYACTION_RETURN, "Return");
  sitekeybinds.addBind('r', KEYACTION_SPREAD, "Spread");
  sitekeybinds.addBind('v', KEYACTION_VIEW_FILE, "View file");
  sitekeybinds.addBind('D', KEYACTION_DOWNLOAD, "Download");
  sitekeybinds.addBind('b', KEYACTION_BIND_SECTION, "Bind to section");
  sitekeybinds.addBind('s', KEYACTION_SORT, "Sort");
  sitekeybinds.addBind('S', KEYACTION_SORT_DEFAULT, "Default sort");
  sitekeybinds.addBind('w', KEYACTION_RAW_COMMAND, "Raw command");
  sitekeybinds.addBind('W', KEYACTION_WIPE, "Wipe");
  sitekeybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete");
  sitekeybinds.addBind('n', KEYACTION_NUKE, "Nuke");
  sitekeybinds.addBind('M', KEYACTION_MKDIR, "Make directory");
  sitekeybinds.addBind('P', KEYACTION_TOGGLE_SEPARATORS, "Toggle separators");
  sitekeybinds.addBind('q', KEYACTION_QUICK_JUMP, "Quick jump");
  sitekeybinds.addBind('f', KEYACTION_FILTER, "Toggle filter");
  sitekeybinds.addBind('F', KEYACTION_FILTER_REGEX, "Regex filter");
  sitekeybinds.addBind('g', KEYACTION_GOTO_PATH, "Go to path");
  sitekeybinds.addBind('l', KEYACTION_COMMAND_LOG, "View command log");
  sitekeybinds.addBind(337, KEYACTION_SOFT_SELECT_UP, "Soft select up");
  sitekeybinds.addBind(336, KEYACTION_SOFT_SELECT_DOWN, "Soft select down");
  sitekeybinds.addBind(' ', KEYACTION_HARD_SELECT, "Hard select");
  sitekeybinds.addBind({'A', 1}, KEYACTION_SELECT_ALL, "Select all");
  sitekeybinds.addBind('m', KEYACTION_MOVE, "Move/rename");
  sitekeybinds.addBind('i', KEYACTION_INFO, "File info");
  sitekeybinds.addBind(KEY_F(5), KEYACTION_REFRESH, "Refresh dir");
  sitekeybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  sitekeybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  sitekeybinds.addBind(KEY_HOME, KEYACTION_TOP, "Jump top");
  sitekeybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Jump bottom");
  sitekeybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Page up");
  sitekeybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Page down");
  sitekeybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
  sitekeybinds.addBind('\t', KEYACTION_SPLIT, "Split/switch side");
  sitekeybinds.addBind('h', KEYACTION_TOGGLE_SHOW_NAMES_ONLY, "Show names only");
  sitekeybinds.addBind('x', KEYACTION_EXTERNAL_SCRIPTS, "Configure external scripts");
  localkeybinds.addScope(KEYSCOPE_SPLIT_SITE_LOCAL, "When split browsing local-site");
  localkeybinds.addBind('t', KEYACTION_TRANSFER, "Transfer (Upload)", KEYSCOPE_SPLIT_SITE_LOCAL);
  localkeybinds.addBind('u', KEYACTION_COMPARE_UNIQUE, "Unique compare", KEYSCOPE_SPLIT_SITE_LOCAL);
  localkeybinds.addBind('I', KEYACTION_COMPARE_IDENTICAL, "Identical compare", KEYSCOPE_SPLIT_SITE_LOCAL);
  localkeybinds.addBind(27, KEYACTION_BACK_CANCEL, "Cancel");
  localkeybinds.addBind('c', KEYACTION_CLOSE, "Close");
  localkeybinds.addBind(10, KEYACTION_ENTER, "Open");
  localkeybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Open");
  localkeybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Return");
  localkeybinds.addBind(KEY_BACKSPACE, KEYACTION_RETURN, "Return");
  localkeybinds.addBind('v', KEYACTION_VIEW_FILE, "View file");
  localkeybinds.addBind('s', KEYACTION_SORT, "Sort");
  localkeybinds.addBind('S', KEYACTION_SORT_DEFAULT, "Default sort");
  localkeybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete");
  localkeybinds.addBind('W', KEYACTION_WIPE, "Delete");
  localkeybinds.addBind('M', KEYACTION_MKDIR, "Make directory");
  localkeybinds.addBind('P', KEYACTION_TOGGLE_SEPARATORS, "Toggle separators");
  localkeybinds.addBind('q', KEYACTION_QUICK_JUMP, "Quick jump");
  localkeybinds.addBind('f', KEYACTION_FILTER, "Toggle filter");
  localkeybinds.addBind('F', KEYACTION_FILTER_REGEX, "Regex filter");
  localkeybinds.addBind('g', KEYACTION_GOTO_PATH, "Go to path");
  localkeybinds.addBind(337, KEYACTION_SOFT_SELECT_UP, "Soft select up");
  localkeybinds.addBind(336, KEYACTION_SOFT_SELECT_DOWN, "Soft select down");
  localkeybinds.addBind(' ', KEYACTION_HARD_SELECT, "Hard select");
  localkeybinds.addBind({'A', 1}, KEYACTION_SELECT_ALL, "Select all");
  localkeybinds.addBind('m', KEYACTION_MOVE, "Move/rename");
  localkeybinds.addBind(KEY_F(5), KEYACTION_REFRESH, "Refresh dir");
  localkeybinds.addBind('i', KEYACTION_INFO, "File info");
  localkeybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  localkeybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  localkeybinds.addBind(KEY_HOME, KEYACTION_TOP, "Jump top");
  localkeybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Jump bottom");
  localkeybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Page up");
  localkeybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Page down");
  localkeybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
  localkeybinds.addBind('\t', KEYACTION_SPLIT, "Split/switch side");
  localkeybinds.addBind('h', KEYACTION_TOGGLE_SHOW_NAMES_ONLY, "Show names only");
  localkeybinds.addBind('x', KEYACTION_EXTERNAL_SCRIPTS, "Configure external scripts");
  ui->addKeyBinds(&sitekeybinds);
  ui->addKeyBinds(&localkeybinds);
  global->getExternalScriptsManager()->addExternalScripts(name, externalscripts.get());
  allowimplicitgokeybinds = false;
}

BrowseScreen::~BrowseScreen() {
  ui->removeKeyBinds(&sitekeybinds);
  ui->removeKeyBinds(&localkeybinds);
  expectbackendpush = true;
}

void BrowseScreen::loadExtraKeyBinds() {
  for (std::list<std::pair<int, std::string>>::const_iterator it = externalscriptbinds.begin(); it != externalscriptbinds.end();) {
    if (!externalscripts->hasScript(it->first)) {
      sitekeybinds.removeBind(it->first + KEYACTION_EXTERNAL_SCRIPT_START, KEYSCOPE_ALL);
      localkeybinds.removeBind(it->first + KEYACTION_EXTERNAL_SCRIPT_START, KEYSCOPE_ALL);
      it = externalscriptbinds.erase(it);
    }
    else {
      std::string newname = externalscripts->getScriptName(it->first);
      if (newname != it->second) {
        sitekeybinds.setBindDescription(KEYACTION_EXTERNAL_SCRIPT_START + it->first, "Run external script (" + newname + ")", KEYSCOPE_ALL);
        localkeybinds.setBindDescription(KEYACTION_EXTERNAL_SCRIPT_START + it->first, "Run external script (" + newname + ")", KEYSCOPE_ALL);
      }
      ++it;
    }
  }
  for (std::list<ExternalScript>::const_iterator it = externalscripts->begin(); it != externalscripts->end(); ++it) {
    bool found = false;
    for (std::list<std::pair<int, std::string>>::const_iterator it2 = externalscriptbinds.begin(); it2 != externalscriptbinds.end(); ++it2) {
      if (it->id == it2->first) {
        found = true;
        break;
      }
    }
    if (!found) {
      externalscriptbinds.emplace_back(it->id, it->name);
      sitekeybinds.addUnboundBind(KEYACTION_EXTERNAL_SCRIPT_START + it->id, "Run external script (" + it->name + ")");
      localkeybinds.addUnboundBind(KEYACTION_EXTERNAL_SCRIPT_START + it->id, "Run external script (" + it->name + ")");
    }
  }
}

void BrowseScreen::initializeSite(unsigned int row, unsigned int col,
    const std::string& site, const Path& path)
{
  TimeReference::updateTime();
  split = false;
  left = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, site, path);
  right.reset();
  active = left;
  loadExtraKeyBinds();
  init(row, col);
}

void BrowseScreen::initializeSite(unsigned int row, unsigned int col,
    const std::string& site, const std::string& section)
{
  TimeReference::updateTime();
  split = false;
  left = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, site, section);
  right.reset();
  active = left;
  loadExtraKeyBinds();
  init(row, col);
}

void BrowseScreen::initializeLocal(unsigned int row, unsigned int col,
    const Path& path)
{
  TimeReference::updateTime();
  split = false;
  left = std::make_shared<BrowseScreenLocal>(ui, localkeybinds, path);
  right.reset();
  active = left;
  loadExtraKeyBinds();
  init(row, col);
}

void BrowseScreen::initializeSplit(unsigned int row, unsigned int col,
    const std::string& site)
{
  TimeReference::updateTime();
  split = true;
  left = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, site);
  right = std::make_shared<BrowseScreenSelector>(ui, keybinds);
  left->setFocus(false);
  active = right;
  loadExtraKeyBinds();
  init(row, col);
}

void BrowseScreen::initializeSiteLocal(unsigned int row, unsigned int col,
    const std::string& site, const Path& sitepath, const Path& localpath)
{
  TimeReference::updateTime();
  split = true;
  left = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, site, sitepath);
  right = std::make_shared<BrowseScreenLocal>(ui, localkeybinds, localpath);
  right->setFocus(false);
  active = left;
  loadExtraKeyBinds();
  init(row, col);
}

void BrowseScreen::initializeSiteSite(unsigned int row, unsigned int col,
    const std::string& leftsite, const std::string& rightsite,
    const Path& leftpath, const Path& rightpath)
{
  TimeReference::updateTime();
  split = true;
  left = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, leftsite, leftpath);
  right = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, rightsite, rightpath);
  right->setFocus(false);
  active = left;
  loadExtraKeyBinds();
  init(row, col);
}

void BrowseScreen::redraw() {
  if (reloadextrakeybinds) {
    loadExtraKeyBinds();
    reloadextrakeybinds = false;
  }
  vv->clear();
  ui->hideCursor();
  ui->setSplit(split);
  unsigned int splitcol = col / 2;
  if (split) {
    for (unsigned int i = 0; i < row; i++) {
      vv->putChar(i, splitcol, BOX_VLINE);
    }
  }
  if (!!right) {
    if (!split) {
      right->redraw(row, col, 0);
    }
    else {
      right->redraw(row, col - splitcol - 1, splitcol + 1);
    }
  }
  if (!!left) {
    if (!split) {
      left->redraw(row, col, 0);

    }
    else {
      left->redraw(row, splitcol, 0);
    }
  }

}

void BrowseScreen::update() {
  if (!!left) {
    left->update();
  }
  if (!isTop()) {
    return;
  }
  if (!!right) {
    right->update();
  }
}

void BrowseScreen::command(const std::string & command, const std::string & arg) {
  active->command(command, arg);
}

bool BrowseScreen::keyPressed(unsigned int ch) {
  BrowseScreenAction op = active->keyPressed(ch);
  switch (op.getOp()) {
    case BROWSESCREENACTION_CLOSE:
      clearCompareListMode();
      if (active->type() != BrowseScreenType::SELECTOR && split) {
        if (active == left) {
          if (right->type() == BrowseScreenType::SELECTOR) {
            ui->returnToLast();
            return true;
          }
          else {
            active = left = std::make_shared<BrowseScreenSelector>(ui, localkeybinds);
            ui->redraw();
            ui->setInfo();
            ui->setLegend();
            return true;
          }
        }
        else {
          if (left->type() == BrowseScreenType::SELECTOR) {
            ui->returnToLast();
            return true;
          }
          else {
            active = right = std::make_shared<BrowseScreenSelector>(ui, localkeybinds);
            ui->redraw();
            ui->setInfo();
            ui->setLegend();
            return true;
          }
        }
      }
      else {
        closeSide();
      }
      return true;
    case BROWSESCREENACTION_SITE: {
      goSite(active == left, op.getArg());
      return true;
    }
    case BROWSESCREENACTION_HOME:
      active = (active == left ? left : right) = std::make_shared<BrowseScreenLocal>(ui, localkeybinds);
      ui->redraw();
      ui->setLegend();
      ui->setInfo();
      return true;
    case BROWSESCREENACTION_NOOP:
      return keyPressedNoSubAction(ch);
    case BROWSESCREENACTION_CAUGHT:
      return true;
    case BROWSESCREENACTION_CHDIR:
      clearCompareListMode();
      return true;
  }
  return false;
}

bool BrowseScreen::keyPressedNoSubAction(unsigned int ch) {
  int scope = KEYSCOPE_ALL;
  if (split) {
    if (left->type() == BrowseScreenType::SITE && right->type() == BrowseScreenType::SITE) {
      scope = KEYSCOPE_SPLIT_SITE_SITE;
    }
    if ((left->type() == BrowseScreenType::LOCAL && right->type() == BrowseScreenType::SITE) ||
        (left->type() == BrowseScreenType::SITE && right->type() == BrowseScreenType::LOCAL))
    {
      scope = KEYSCOPE_SPLIT_SITE_LOCAL;
    }
  }
  KeyBinds* binds = &keybinds;
  if (active->type() == BrowseScreenType::SITE) {
    binds = &sitekeybinds;
  }
  else if (active->type() == BrowseScreenType::LOCAL) {
    binds = &localkeybinds;
  }
  int action = binds->getKeyAction(ch, scope);
  switch (action) {
    case KEYACTION_SPLIT:
      if (!split) {
        split = true;
        right = std::make_shared<BrowseScreenSelector>(ui, keybinds);
      }
      {
        switchSide();
      }
      return true;
    case KEYACTION_TRANSFER:
      if (split && left->type() != BrowseScreenType::SELECTOR && right->type() != BrowseScreenType::SELECTOR) {
        std::shared_ptr<BrowseScreenSub> other = active == left ? right : left;
        if (active->type() == BrowseScreenType::SITE) {
          std::shared_ptr<FileList> activefl = std::static_pointer_cast<BrowseScreenSite>(active)->fileList();
          std::list<UIFile *> files = std::static_pointer_cast<BrowseScreenSite>(active)->getUIFileList()->getSelectedFiles();
          if (activefl != NULL) {
            if (other->type() == BrowseScreenType::SITE) {
              std::shared_ptr<FileList> otherfl = std::static_pointer_cast<BrowseScreenSite>(other)->fileList();
              if (otherfl != NULL) {
                for (std::list<UIFile *>::const_iterator it = files.begin(); it != files.end(); it++) {
                  UIFile * f = *it;
                  if (!f || (!f->isDirectory() && !f->getSize())) {
                    continue;
                  }
                  unsigned int id = global->getEngine()->newTransferJobFXP(std::static_pointer_cast<BrowseScreenSite>(active)->siteName(),
                                                                           activefl,
                                                                           std::static_pointer_cast<BrowseScreenSite>(other)->siteName(),
                                                                           otherfl,
                                                                           f->getName());
                  ui->addTempLegendTransferJob(id);
                }
              }
            }
            else {
              std::shared_ptr<LocalFileList> otherfl = std::static_pointer_cast<BrowseScreenLocal>(other)->fileList();
              if (!!otherfl) {
                for (std::list<UIFile *>::const_iterator it = files.begin(); it != files.end(); it++) {
                  UIFile * f = *it;
                  if (!f || (!f->isDirectory() && !f->getSize())) {
                    continue;
                  }
                  unsigned int id = global->getEngine()->newTransferJobDownload(std::static_pointer_cast<BrowseScreenSite>(active)->siteName(),
                                                                                activefl,
                                                                                f->getName(),
                                                                                otherfl->getPath());
                  ui->addTempLegendTransferJob(id);
                }
              }
            }
          }
        }
        else if (other->type() == BrowseScreenType::SITE) {
          std::shared_ptr<LocalFileList> activefl = std::static_pointer_cast<BrowseScreenLocal>(active)->fileList();
          std::shared_ptr<FileList> otherfl = std::static_pointer_cast<BrowseScreenSite>(other)->fileList();
          std::list<UIFile *> files = std::static_pointer_cast<BrowseScreenLocal>(active)->getUIFileList()->getSelectedFiles();
          if (!!activefl && otherfl != NULL) {
            for (std::list<UIFile *>::const_iterator it = files.begin(); it != files.end(); it++) {
              UIFile * f = *it;
              if (!f || (!f->isDirectory() && !f->getSize())) {
                continue;
              }
              unsigned int id = global->getEngine()->newTransferJobUpload(activefl->getPath(),
                                                                          f->getName(),
                                                                          std::static_pointer_cast<BrowseScreenSite>(other)->siteName(),
                                                                          otherfl);
              ui->addTempLegendTransferJob(id);
            }
          }
        }
      }
      return true;
    case KEYACTION_COMPARE_UNIQUE:
      toggleCompareListMode(CompareMode::UNIQUE);
      return true;
    case KEYACTION_COMPARE_IDENTICAL:
      toggleCompareListMode(CompareMode::IDENTICAL);
      return true;
    case KEYACTION_MOVE:
      if (active->type() == BrowseScreenType::SITE) {
        std::shared_ptr<BrowseScreenSite> activesite = std::static_pointer_cast<BrowseScreenSite>(active);
        std::shared_ptr<BrowseScreenSub> other = active == left ? right : left;
        std::string dstpath;
        if (split && other->type() == BrowseScreenType::SITE) {
          std::shared_ptr<BrowseScreenSite> othersite = std::static_pointer_cast<BrowseScreenSite>(other);
          if (activesite->getSite() == othersite->getSite()) {
            dstpath = othersite->getUIFileList()->getPath().toString();
          }
        }
        activesite->initiateMove(dstpath);
      }
      else if (active->type() == BrowseScreenType::LOCAL) {
        std::shared_ptr<BrowseScreenSub> other = active == left ? right : left;
        std::string dstpath;
        if (split && other->type() == BrowseScreenType::LOCAL) {
          dstpath = other->getUIFileList()->getPath().toString();
        }
        active->initiateMove(dstpath);
      }
      return true;
    case KEYACTION_0:
      jumpSectionHotkey(0);
      return true;
    case KEYACTION_1:
      jumpSectionHotkey(1);
      return true;
    case KEYACTION_2:
      jumpSectionHotkey(2);
      return true;
    case KEYACTION_3:
      jumpSectionHotkey(3);
      return true;
    case KEYACTION_4:
      jumpSectionHotkey(4);
      return true;
    case KEYACTION_5:
      jumpSectionHotkey(5);
      return true;
    case KEYACTION_6:
      jumpSectionHotkey(6);
      return true;
    case KEYACTION_7:
      jumpSectionHotkey(7);
      return true;
    case KEYACTION_8:
      jumpSectionHotkey(8);
      return true;
    case KEYACTION_9:
      jumpSectionHotkey(9);
      return true;
    case KEYACTION_KEYBINDS:
      ui->goKeyBinds(binds);
      return true;
    case KEYACTION_EXTERNAL_SCRIPTS:
      ui->goExternalScripts(externalscripts.get(),
          "Here you can add external scripts that can be bound to hotkeys in the browse screen.\n"
          "The scripts will be executed with various args, depending on situation:\n"
          "- <api-key> browse-site <sitename> <path> <selecteditems>\n"
          "- <api-key> browse-site-site <sitename1> <path1> <selecteditems> <sitename2> <path2>\n"
          "- <api-key> browse-site-local <sitename> <path> <selecteditems> <localpath>\n"
          "- <api-key> browse-local <localpath> <selecteditems>\n"
          "- <api-key> browse-local-site <localpath> <selecteditems> <sitename> <path>");
      reloadextrakeybinds = true;
      return true;
  }
  if (action >= KEYACTION_EXTERNAL_SCRIPT_START && action < KEYACTION_EXTERNAL_SCRIPT_MAX) {
    std::vector<std::string> args;
    std::shared_ptr<BrowseScreenSub> other = active == left ? right : left;
    if (active->type() == BrowseScreenType::SITE) {
      std::shared_ptr<BrowseScreenSite> activesite = std::static_pointer_cast<BrowseScreenSite>(active);
      std::string site = activesite->getSite()->getName();
      Path path = active->getUIFileList()->getPath();
      std::list<std::pair<std::string, bool>> selecteditemswithtype = active->getUIFileList()->getSelectedNames();
      std::list<std::string> selecteditems;
      for (const std::pair<std::string, bool>& pair : selecteditemswithtype) {
        selecteditems.push_back(pair.first);
      }
      if (split && other->type() != BrowseScreenType::SELECTOR) {
        if (other->type() == BrowseScreenType::SITE) {
          std::shared_ptr<BrowseScreenSite> othersite = std::static_pointer_cast<BrowseScreenSite>(other);
          std::string othersitename = othersite->getSite()->getName();
          Path otherpath = othersite->getUIFileList()->getPath();
          args = {"browse-site-site", site, path.toString(), util::join(selecteditems, ","), othersitename, otherpath.toString()};
        }
        else if (other->type() == BrowseScreenType::LOCAL) {
          Path otherpath = other->getUIFileList()->getPath();
          args = {"browse-site-local", site, path.toString(), util::join(selecteditems, ","), otherpath.toString()};
        }
      }
      else {
        args = {"browse-site", site, path.toString(), util::join(selecteditems, ",")};
      }
    }
    else if (active->type() == BrowseScreenType::LOCAL) {
      Path path = active->getUIFileList()->getPath();
      std::list<std::pair<std::string, bool>> selecteditemswithtype = active->getUIFileList()->getSelectedNames();
      std::list<std::string> selecteditems;
      for (const std::pair<std::string, bool>& pair : selecteditemswithtype) {
        selecteditems.push_back(pair.first);
      }
      if (split && other->type() == BrowseScreenType::SITE) {
        std::shared_ptr<BrowseScreenSite> othersite = std::static_pointer_cast<BrowseScreenSite>(other);
        std::string othersitename = othersite->getSite()->getName();
        Path otherpath = othersite->getUIFileList()->getPath();
        args = {"browse-local-site", path.toString(), util::join(selecteditems, ","), othersitename, otherpath.toString()};
      }
      else {
        args = {"browse-local", path.toString(), util::join(selecteditems, ",")};
      }
    }
    else {
      return false;
    }
    int scriptid = action - KEYACTION_EXTERNAL_SCRIPT_START;
    std::shared_ptr<RawBuffer> rawbuf = externalscripts->execute(scriptid, args);
    return true;
  }
  return false;
}

std::string BrowseScreen::getLegendText() const {
  return active->getLegendText(KEYSCOPE_ALL);
  /*std::string extra = "";
  if (split && left->type() != BrowseScreenType::SELECTOR && right->type() != BrowseScreenType::SELECTOR) {
    extra += "Show [u]niques - Show [I]denticals - ";
    if (left->type() == BrowseScreenType::SITE || right->type() == BrowseScreenType::SITE) {
      extra += "[t]ransfer - ";
    }
  }
  return "[Tab] switch side - " + extra  + active->getLegendText();*/
}

std::string BrowseScreen::getInfoLabel() const {
  if (split) {
   if (left->type() == BrowseScreenType::SITE) {
     if (right->type() == BrowseScreenType::SITE) {
       return "BROWSING: " + std::static_pointer_cast<BrowseScreenSite>(left)->siteName() + " - " +
           std::static_pointer_cast<BrowseScreenSite>(right)->siteName();
     }
     return left->getInfoLabel();
   }
   if (right->type() == BrowseScreenType::SITE) {
     return right->getInfoLabel();
   }
  }
  return active->getInfoLabel();
}

std::string BrowseScreen::getInfoText() const {
  return active->getInfoText();
}

bool BrowseScreen::isInitialized() const {
  return !!active;
}

void BrowseScreen::suggestOtherRefresh(BrowseScreenSub* sub) {
  if (split && left->type() == BrowseScreenType::SITE && right->type() == BrowseScreenType::SITE) {
    if (std::static_pointer_cast<BrowseScreenSite>(left)->getSite() == std::static_pointer_cast<BrowseScreenSite>(right)->getSite()) {
      if (sub == left.get()) {
        right->refreshFileList();
      }
      else {
        left->refreshFileList();
      }
    }
  }
}

void BrowseScreen::switchSide() {
  initsplitupdate = false;
  bool leftfocused = active == left;
  left->setFocus(!leftfocused);
  right->setFocus(leftfocused);
  if (leftfocused) {
    left->update();
    active = right;
  }
  else {
    right->update();
    active = left;
  }
  ui->setInfo();
  if (left->type() != right->type()) {
    ui->setLegend();
  }
  ui->redraw();
}

void BrowseScreen::closeSide() {
  if (split) {
    initsplitupdate = false;
    split = false;
    if (active == left) {
      left = right;
    }
    else {
      right.reset();
    }
    active = left;
    if (active->type() == BrowseScreenType::SELECTOR) {
      ui->returnToLast();
      return;
    }
    active->setFocus(true);
    ui->redraw();
    ui->setInfo();
    ui->setLegend();
  }
  else {
    ui->returnToLast();
  }
}

void BrowseScreen::toggleCompareListMode(CompareMode mode) {
  if (split && left->type() != BrowseScreenType::SELECTOR && right->type() != BrowseScreenType::SELECTOR) {
    UIFileList * leftlist = left->getUIFileList();
    UIFileList * rightlist = right->getUIFileList();
    if (leftlist->getCompareListMode() == mode || rightlist->getCompareListMode() == mode) {
      leftlist->clearCompareListMode();
      rightlist->clearCompareListMode();
    }
    else {
      std::set<std::string> leftfiles;  // need to make copies of the lists here since setting
      std::set<std::string> rightfiles; // it on one side will affect the other side
      const std::vector <UIFile *> * sorteduniquelist = leftlist->getSortedList();
      for (std::vector<UIFile *>::const_iterator it = sorteduniquelist->begin(); it != sorteduniquelist->end(); it++) {
        if (*it != nullptr) {
          leftfiles.insert((*it)->getName());
        }
      }
      sorteduniquelist = rightlist->getSortedList();
      for (std::vector<UIFile *>::const_iterator it = sorteduniquelist->begin(); it != sorteduniquelist->end(); it++) {
        if (*it != nullptr) {
          rightfiles.insert((*it)->getName());
        }
      }
      leftlist->setCompareList(rightfiles, mode);
      rightlist->setCompareList(leftfiles, mode);
    }
  }
  ui->redraw();
  ui->setInfo();
}

void BrowseScreen::clearCompareListMode() {
  if (left && left->type() != BrowseScreenType::SELECTOR) {
    left->getUIFileList()->clearCompareListMode();
  }
  if (right && right->type() != BrowseScreenType::SELECTOR) {
    right->getUIFileList()->clearCompareListMode();
  }
}

void BrowseScreen::jumpSectionHotkey(int hotkey) {
  Section * section = global->getSectionManager()->getSection(hotkey);
  if (section == nullptr) {
    return;
  }
  if (active == left || left->type() != BrowseScreenType::SELECTOR) {
    BrowseScreenAction action = left->tryJumpSection(section->getName());
    if (action.getOp() == BROWSESCREENACTION_SITE) {
      goSite(true, action.getArg(), section->getName());
    }
  }
  if (split && (active == right || right->type() != BrowseScreenType::SELECTOR)) {
    BrowseScreenAction action = right->tryJumpSection(section->getName());
    if (action.getOp() == BROWSESCREENACTION_SITE) {
      goSite(false, action.getArg(), section->getName());
    }
  }
}

void BrowseScreen::goSite(bool leftside, const std::string& site, const std::string& section) {
  std::string targetsection = section;
  if (split) {
    std::shared_ptr<Site> siteptr = global->getSiteManager()->getSite(site);
    std::shared_ptr<BrowseScreenSub> other = leftside ? right : left;
    if (targetsection.empty()) {
      if (other->type() == BrowseScreenType::SITE) {
        const std::shared_ptr<BrowseScreenSite>& othersite = std::static_pointer_cast<BrowseScreenSite>(other);
        Path otherpath = othersite->getUIFileList()->getPath();
        if (otherpath == othersite->getLastJumpPath()) {
          targetsection = othersite->getLastJumpSection();
        }
        else {
          std::list<std::string> sections = othersite->getSite()->getSectionsForPath(otherpath);
          for (const std::string& sec : sections) {
            if (siteptr->hasSection(sec)) {
              targetsection = sec;
              break;
            }
          }
        }
      }
    }
  }
  bool updateactive = (active == left && leftside) || (active == right && !leftside);
  (leftside ? left : right) = std::make_shared<BrowseScreenSite>(ui, this, sitekeybinds, site, targetsection);
  if (updateactive) {
    active = leftside ? left : right;
  }
  ui->redraw();
  ui->setLegend();
  ui->setInfo();
}
