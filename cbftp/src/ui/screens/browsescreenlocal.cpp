#include "browsescreenlocal.h"



#include "../../core/tickpoke.h"
#include "../../localstorage.h"
#include "../../localfilelist.h"
#include "../../globalcontext.h"
#include "../../skiplist.h"
#include "../../util.h"
#include "../../filesystem.h"

#include "../ui.h"
#include "../menuselectoptiontextbutton.h"
#include "../resizableelement.h"
#include "../termint.h"
#include "../menuselectadjustableline.h"
#include "../misc.h"
#include "../keybinds.h"

#include "browsescreenaction.h"

#include <cassert>

BrowseScreenLocal::BrowseScreenLocal(Ui* ui, KeyBinds& keybinds,
    const Path& path) : BrowseScreenSub(keybinds),
    ui(ui), currentviewspan(0), focus(true), table(ui->getVirtualView()), spinnerpos(0), tickcount(0),
    resort(false), sortmethod(UIFileList::SortMethod::COMBINED), gotomode(false), gotomodefirst(false),
    gotomodeticker(0), filtermodeinput(false), filtermodeinputregex(false), gotopathinput(false),
    temphighlightline(false), softselecting(false), lastinfo(LastInfo::NONE),
    refreshfilelistafter(false), freespace(0), nameonly(false)
{
  vv = &ui->getVirtualView();
  BrowseScreenRequest request;
  request.type = BrowseScreenRequestType::FILELIST;
  request.path = path != "" ? path :global->getLocalStorage()->getDownloadPath();
  request.id = global->getLocalStorage()->requestLocalFileList(request.path);
  requests.push_back(request);
}

BrowseScreenLocal::~BrowseScreenLocal() {

}

BrowseScreenType BrowseScreenLocal::type() const {
  return BrowseScreenType::LOCAL;
}

void BrowseScreenLocal::redraw(unsigned int row, unsigned int col, unsigned int coloffset) {
  row = row - ((filtermodeinput || filtermodeinputregex || gotopathinput) ? 2 : 0);
  this->row = row;
  this->col = col;
  this->coloffset = coloffset;

  if (resort == true) {
    list.sortMethod(sortmethod);
    resort = false;
  }
  unsigned int position = list.currentCursorPosition();
  unsigned int listsize = list.size();
  adaptViewSpan(currentviewspan, row, position, listsize);

  table.reset();
  const std::vector<UIFile *> * uilist = list.getSortedList();
  for (unsigned int i = 0; i + currentviewspan < uilist->size() && i < row; i++) {
    unsigned int listi = i + currentviewspan;
    UIFile * uifile = (*uilist)[listi];
    bool selected = uifile == list.cursoredFile();
    bool isdir = uifile->isDirectory();
    bool allowed = global->getSkipList()->check((list.getPath() / uifile->getName()).toString(), isdir, false).action != SKIPLIST_DENY;
    std::string prepchar = " ";
    if (isdir) {
      if (allowed) {
        prepchar = "#";
      }
      else {
        prepchar = "S";
      }
    }
    else if (!allowed) {
      prepchar = "s";
    }
    else if (uifile->isLink()){
      prepchar = "L";
    }
    std::string owner = uifile->getOwner() + "/" + uifile->getGroup();
    addFileDetails(table, coloffset, i, uifile->getName(), prepchar, uifile->getSizeRepr(), uifile->getLastModified(), owner, true, selected, uifile, nameonly);
  }
  table.adjustLines(col - 3);
  table.checkPointer();
  std::shared_ptr<MenuSelectAdjustableLine> highlightline;
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    bool cursored = table.getSelectionPointer() == i;
    bool softselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isSoftSelected();
    bool hardselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isHardSelected();
    bool highlight = cursored || softselected || hardselected;
    if (re->isVisible()) {
      if ((cursored && softselected) || (cursored && hardselected) || (softselected && hardselected)) {
        printFlipped(ui, re);
      }
      else {
        vv->putStr(re->getRow(), re->getCol(), re->getLabelText(), highlight && focus);
      }
    }
    if (cursored && (temphighlightline ^ ui->getHighlightEntireLine())) {
      highlightline = table.getAdjustableLine(re);
    }
  }
  if (highlightline) {
    std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
    vv->highlightOn(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
  }
  printSlider(vv, row, coloffset + col - 1, listsize, currentviewspan);

  if (!list.isInitialized()) {
    vv->putStr(0, coloffset + 3, "Loading file list...");
  }
  else if (listsize == 0) {
    vv->putStr(0, coloffset + 3, "(empty directory)");
  }
  if (filtermodeinput || filtermodeinputregex || gotopathinput) {
    std::string oldtext = bottomlinetextfield.getData();
    bottomlinetextfield = MenuSelectOptionTextField("filter", row + 1, 1, "", oldtext,
        col - 20, 512, false);
    ui->showCursor();
  }
  update();
}

void BrowseScreenLocal::update() {
  if (handleReadyRequests()) {
    return;
  }
  if (table.size()) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(table.getLastSelectionPointer()));
    bool softselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isSoftSelected();
    bool hardselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isHardSelected();
    std::shared_ptr<MenuSelectAdjustableLine> highlightline = table.getAdjustableLine(re);
    std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
    vv->highlightOff(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
    if (softselected && hardselected) {
      printFlipped(ui, re);
    }
    else {
      vv->putStr(re->getRow(), re->getCol(), re->getLabelText(), softselected || hardselected);
    }
    re = std::static_pointer_cast<ResizableElement>(table.getElement(table.getSelectionPointer()));
    bool selected = re->getOrigin() && (static_cast<UIFile *>(re->getOrigin())->isSoftSelected() ||
                                        static_cast<UIFile *>(re->getOrigin())->isHardSelected());
    highlightline = table.getAdjustableLine(re);
    minmaxcol = highlightline->getMinMaxCol();
    if (temphighlightline ^ ui->getHighlightEntireLine()) {
      vv->highlightOn(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
    }
    else {
      vv->highlightOff(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
    }
    if (selected && focus) {
      printFlipped(ui, re);
    }
    else {
      vv->putStr(re->getRow(), re->getCol(), re->getLabelText(), focus);
    }
  }
  if (filtermodeinput || filtermodeinputregex || gotopathinput) {
    std::string pretag = filtermodeinput ? "[Filter(s)]: " : (filtermodeinputregex ? "[Regex filter]: " : "[Go to path]: ");
    vv->putStr(bottomlinetextfield.getRow(), coloffset + bottomlinetextfield.getCol(), pretag + bottomlinetextfield.getContentText());
    vv->moveCursor(bottomlinetextfield.getRow(), coloffset + bottomlinetextfield.getCol() + pretag.length() + bottomlinetextfield.cursorPosition());
  }
}

bool BrowseScreenLocal::handleReadyRequests() {
  bool handled = false;
  while (!requests.empty() && global->getLocalStorage()->requestReady(requests.front().id)) {
    handled = true;
    const BrowseScreenRequest & request = requests.front();
    switch (request.type) {
      case BrowseScreenRequestType::PATH_INFO: {
        LocalPathInfo pathinfo = global->getLocalStorage()->getPathInfo(request.id);
        std::string dirs = std::to_string(pathinfo.getNumDirs());
        std::string files = std::to_string(pathinfo.getNumFiles());
        std::string size = util::parseSize(pathinfo.getSize());
        int maxdepth = pathinfo.getDepth();
        std::string targettext = "these " + std::to_string(static_cast<int>(request.files.size())) + " items";
        if (request.files.size() == 1) {
          targettext = request.files.front().first;
        }
        std::string confirmation = "Do you really want to delete " + targettext + "?";
        if (maxdepth > 0) {
          std::string contain = request.files.size() == 1 ? " It contains " : " They contain ";
          std::string depthpre = request.files.size() == 1 ? "is " : "are ";
          confirmation += contain + size + " in " + files + "f/" + dirs + "d and " + depthpre;
        }
        if (maxdepth == 1) {
          confirmation += "1 level deep.";
        }
        else if (maxdepth > 1) {
          confirmation += std::to_string(maxdepth) + " levels deep.";
        }
        if (pathinfo.getNumDirs() >= 10 || pathinfo.getNumFiles() >= 100 || pathinfo.getSize() >= 100000000000 || pathinfo.getDepth() > 2) {
          ui->goStrongConfirmation(confirmation);
        }
        else {
          ui->goConfirmation(confirmation);
        }
        break;
      }
      case BrowseScreenRequestType::FILELIST:
        gotoPath(request.id, request.path);
        requests.pop_front();
        break;
      case BrowseScreenRequestType::DELETE: {
        bool success = global->getLocalStorage()->getDeleteResult(request.id);
        if (success) {
          lastinfo = LastInfo::DELETE_SUCCESS;
        }
        else {
          lastinfo = LastInfo::DELETE_FAILED;
        }
        lastinfotarget = request.files.front().first;
        tickcount = 0;
        refreshfilelistafter = true;
        requests.pop_front();
        break;
      }
      case BrowseScreenRequestType::MKDIR: {
        util::Result res = global->getLocalStorage()->getMakeDirResult(request.id);
        if (res.success) {
          lastinfo = LastInfo::MKDIR_SUCCESS;
          lastinfotarget = request.files.front().first;
        }
        else {
          lastinfo = LastInfo::MKDIR_FAILED;
          lastinfotarget = res.error;
        }
        tickcount = 0;
        refreshfilelistafter = true;
        requests.pop_front();
        break;
      }
      case BrowseScreenRequestType::MOVE: {
        util::Result res = global->getLocalStorage()->getMoveResult(request.id);
        if (res.success) {
          lastinfo = LastInfo::MOVE_SUCCESS;
          lastinfotarget = request.files.front().first;
        }
        else {
          lastinfo = LastInfo::MOVE_FAILED;
          lastinfotarget = res.error;
        }
        tickcount = 0;
        refreshfilelistafter = true;
        requests.pop_front();
        break;
      }
      default:
        assert(false);
        break;
    }
  }
  if (handled) {
    if (requests.empty() && refreshfilelistafter) {
      refreshfilelistafter = false;
      refreshFilelist();
    }
    ui->redraw();
    ui->setInfo();
  }
  return handled;
}

void BrowseScreenLocal::command(const std::string & command, const std::string & arg) {
  if (!requests.empty() && requests.front().type == BrowseScreenRequestType::PATH_INFO) {
    const BrowseScreenRequest & pathinforequest = requests.front();
    if (command == "yes") {
      for (const auto & file : pathinforequest.files) {
        BrowseScreenRequest delrequest;
        delrequest.path = pathinforequest.path;
        std::list<std::pair<std::string, bool>> files;
        files.push_back(file);
        delrequest.files = files;
        delrequest.type = BrowseScreenRequestType::DELETE;
        delrequest.id = global->getLocalStorage()->requestDelete(delrequest.path / file.first, true);
        requests.push_back(delrequest);
      }
    }
    requests.pop_front();
  }
  else if (command == "makedir") {
    BrowseScreenRequest request;
    request.id = global->getLocalStorage()->requestMakeDirectory(list.getPath(), arg);
    request.type = BrowseScreenRequestType::MKDIR;
    request.path = list.getPath();
    request.files = { std::make_pair(arg, true) };
    requests.push_back(request);
  }
  else if (command == "move") {
    Path dstpath = arg;
    for (const std::pair<std::string, bool>& item : list.getSelectedNames()) {
      BrowseScreenRequest request;
      request.id = global->getLocalStorage()->requestMove(list.getPath() / item.first, dstpath.isAbsolute() ? dstpath : list.getPath() / dstpath);
      request.type = BrowseScreenRequestType::MOVE;
      request.path = list.getPath();
      request.files = { item };
      requests.push_back(request);
    }
  }
  ui->setInfo();
  ui->redraw();
}

BrowseScreenAction BrowseScreenLocal::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (temphighlightline) {
    temphighlightline = false;
    ui->redraw();
    if (action == KEYACTION_HIGHLIGHT_LINE) {
      return BrowseScreenAction();
    }
  }
  bool update = false;
  bool success = false;
  unsigned int pagerows = (unsigned int) row * 0.6;
  if (filtermodeinput || filtermodeinputregex || gotopathinput) {
    if (bottomlinetextfield.inputChar(ch)) {
      ui->update();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
    else if (ch == 10) {
      ui->hideCursor();
      std::string text = bottomlinetextfield.getData();
      if (text.length()) {
        if (filtermodeinput) {
          list.setWildcardFilters(util::trim(util::split(text)));
        }
        else if (filtermodeinputregex) {
          try {
            std::regex regexfilter = util::regexParse(text);
            list.setRegexFilter(regexfilter, text);
          }
          catch (std::regex_error& e) {
            ui->goInfo("Invalid regular expression.");
            return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
          }
        }
        else if (gotopathinput) {
          BrowseScreenRequest request;
          request.type = BrowseScreenRequestType::FILELIST;
          request.path = text;
          request.id = global->getLocalStorage()->requestLocalFileList(request.path);
          requests.push_back(request);
          bottomlinetextfield.clear();
          ui->setInfo();
        }
        clearSoftSelects();
        resort = true;
      }
      filtermodeinput = false;
      filtermodeinputregex = false;
      gotopathinput = false;
      ui->redraw();
      ui->setLegend();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
    else if (ch == 27) {
      if (!bottomlinetextfield.getData().empty()) {
        bottomlinetextfield.clear();
        ui->update();
      }
      else {
        filtermodeinput = false;
        filtermodeinputregex = false;
        gotopathinput = false;
        ui->hideCursor();
        ui->redraw();
        ui->setLegend();
      }
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
    else if (ch == '\t') {
      filtermodeinput = !filtermodeinput;
      filtermodeinputregex = !filtermodeinputregex;
      ui->update();
      ui->setLegend();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
  }
  if (gotomode) {
    if (gotomodefirst) {
      gotomodefirst = false;
    }
    if (ch >= 32 && ch <= 126) {
      gotomodeticker = 0;
      gotomodestring += toupper(ch);
      unsigned int gotomodelength = gotomodestring.length();
      const std::vector<UIFile *> * sortedlist = list.getSortedList();
      for (unsigned int i = 0; i < sortedlist->size(); i++) {
        if ((*sortedlist)[i] == NULL) continue;
        std::string name = (*sortedlist)[i]->getName();
        if (name.length() >= gotomodelength) {
          std::string substr = name.substr(0, gotomodelength);
          for (unsigned int j = 0; j < gotomodelength; j++) {
            substr[j] = toupper(substr[j]);
          }
          if (substr == gotomodestring) {
            list.setCursorPosition(i);
            clearSoftSelects();
            break;
          }
        }
      }
      ui->redraw();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
    else {
      disableGotoMode();
    }
    if (ch == 27) {
      return BrowseScreenAction();
    }
  }
  switch (action) {
    case KEYACTION_BACK_CANCEL:
      if (list.clearSelected()) {
        ui->redraw();
        break;
      }
      ui->returnToLast();
      break;
    case KEYACTION_CLOSE:
      return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
    case KEYACTION_WIPE:
    case KEYACTION_DELETE: {
      std::list<std::pair<std::string, bool> > filenames = list.getSelectedNames();
      if (filenames.empty()) {
        break;
      }
      BrowseScreenRequest request;
      request.path = filelist->getPath();
      request.files = filenames;
      request.type = BrowseScreenRequestType::PATH_INFO;
      std::list<Path> paths;
      for (std::list<std::pair<std::string, bool> >::const_iterator it = filenames.begin(); it != filenames.end(); it++) {
        paths.emplace_back(request.path / it->first);
      }
      request.id = global->getLocalStorage()->requestPathInfo(paths);
      requests.push_back(request);
      ui->setInfo();
      break;
    }
    case KEYACTION_QUICK_JUMP:
      gotomode = true;
      gotomodefirst = true;
      gotomodeticker = 0;
      gotomodestring = "";
      global->getTickPoke()->startPoke(this, "BrowseScreenLocal", 50, 0);
      ui->update();
      ui->setLegend();
      break;
    case KEYACTION_FILTER:
      if (list.hasWildcardFilters() || list.hasRegexFilter()) {
        resort = true;
        list.unsetFilters();
      }
      else {
        filtermodeinput = true;
      }
      ui->redraw();
      ui->setLegend();
      break;
    case KEYACTION_FILTER_REGEX:
      if (list.hasWildcardFilters() || list.hasRegexFilter()) {
        resort = true;
        list.unsetFilters();
      }
      else {
        filtermodeinputregex = true;
      }
      ui->redraw();
      ui->setLegend();
      break;
    case KEYACTION_GOTO_PATH:
      gotopathinput = true;
      bottomlinetextfield.clear();
      ui->redraw();
      ui->setLegend();
      break;
    case KEYACTION_LEFT:
    case KEYACTION_RETURN:
    {
      //go up one directory level, or return if at top already
      const Path & oldpath = list.getPath();
      if (oldpath == "/") {
        return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
      }
      BrowseScreenRequest request;
      request.type = BrowseScreenRequestType::FILELIST;
      request.path = oldpath.dirName();
      request.id = global->getLocalStorage()->requestLocalFileList(request.path);
      requests.push_back(request);
      ui->setInfo();
      ui->redraw();
      break;
    }
    case KEYACTION_REFRESH:
    {
      refreshFilelist();
      ui->setInfo();
      break;
    }
    case KEYACTION_DOWN:
      if (!keyDown()) {
        ui->update();
      }
      break;
    case KEYACTION_UP:
      //go up and highlight previous item (if not at top already)
      clearSoftSelects();
      update = list.goPrevious();
      if (list.currentCursorPosition() < currentviewspan) {
        ui->redraw();
      }
      else if (update) {
        table.goUp();
        ui->update();
      }
      break;
    case KEYACTION_NEXT_PAGE:
      clearSoftSelects();
      for (unsigned int i = 0; i < pagerows; i++) {
        success = list.goNext();
        if (!success) {
          break;
        }
        else if (!update) {
          update = true;
        }
        table.goDown();
      }
      if (update) {
        ui->redraw();
      }
      break;
    case KEYACTION_PREVIOUS_PAGE:
      clearSoftSelects();
      for (unsigned int i = 0; i < pagerows; i++) {
        success = list.goPrevious();
        if (!success) {
          break;
        }
        else if (!update) {
          update = true;
        }
        table.goUp();
      }
      if (update) {
        ui->redraw();
      }
      break;
    case KEYACTION_RIGHT:
    case KEYACTION_ENTER:
    {
      UIFile * cursoredfile = list.cursoredFile();
      if (cursoredfile == NULL) {
        break;
      }
      bool isdir = cursoredfile->isDirectory();
      bool islink = cursoredfile->isLink();
      if (isdir || islink) {
        BrowseScreenRequest request;
        request.type = BrowseScreenRequestType::FILELIST;
        const Path & oldpath = list.getPath();
        if (islink) {
          Path target = cursoredfile->getLinkTarget();
          if (target.isAbsolute()) {
            request.path = target;
          }
          else {
            request.path = oldpath / target;
          }
        }
        else {
          request.path = oldpath / cursoredfile->getName();
        }
        request.id = global->getLocalStorage()->requestLocalFileList(request.path);
        requests.push_back(request);
        ui->setInfo();
        ui->redraw();
      }
      else {
        viewCursored();
      }
      break;
    }
    case KEYACTION_TOP:
      clearSoftSelects();
      while (list.goPrevious()) {
        table.goUp();
        if (!update) {
          update = true;
        }
      }
      if (update) {
        ui->redraw();
      }
      break;
    case KEYACTION_BOTTOM:
      clearSoftSelects();
      while (list.goNext()) {
        table.goDown();
        if (!update) {
          update = true;
        }
      }
      if (update) {
        ui->redraw();
      }
      break;
    case KEYACTION_SORT:
      sortmethod = static_cast<UIFileList::SortMethod>((static_cast<int>(sortmethod) + 1) % 9);
      resort = true;
      lastinfo = LastInfo::CHANGED_SORT;
      lastinfotarget = list.getSortMethod(sortmethod);
      tickcount = 0;
      ui->redraw();
      ui->setInfo();
      break;
    case KEYACTION_SORT_DEFAULT:
      sortmethod = UIFileList::SortMethod::COMBINED;
      resort = true;
      lastinfo = LastInfo::CHANGED_SORT;
      lastinfotarget = list.getSortMethod(sortmethod);
      tickcount = 0;
      ui->redraw();
      ui->setInfo();
      break;
    case KEYACTION_VIEW_FILE:
      viewCursored();
      break;
    case KEYACTION_SELECT_ALL:
    {
      const std::vector<UIFile *> * uilist = list.getSortedList();
      for (unsigned int i = 0; i < uilist->size(); i++) {
        (*uilist)[i]->softSelect();
      }
      softselecting = true;
      ui->redraw();
      break;
    }
    case KEYACTION_SOFT_SELECT_UP:
      if (list.cursoredFile() != NULL) {
        UIFile * lastfile = list.cursoredFile();
        if (!list.goPrevious()) {
          break;
        }
        UIFile * file = list.cursoredFile();
        if (file->isSoftSelected()) {
          file->unSoftSelect();
        }
        else {
          lastfile->softSelect();
          softselecting = true;
        }
        table.goUp();
        ui->redraw();
      }
      break;
    case KEYACTION_SOFT_SELECT_DOWN:
      if (list.cursoredFile() != NULL) {
        UIFile * lastfile = list.cursoredFile();
        if (!list.goNext()) {
          break;
        }
        UIFile * file = list.cursoredFile();
        if (file->isSoftSelected()) {
          file->unSoftSelect();
        }
        else {
          lastfile->softSelect();
          softselecting = true;
        }
        table.goDown();
        ui->redraw();
      }
      break;
    case KEYACTION_HARD_SELECT:
      if (softselecting) {
        list.hardFlipSoftSelected();
        softselecting = false;
        if (list.cursoredFile()->isHardSelected()) {
          list.cursoredFile()->unHardSelect();
        }
        else {
          list.cursoredFile()->hardSelect();
        }
        if (!keyDown()) {
          ui->redraw();
        }
      }
      else if (list.cursoredFile() != NULL) {
        if (list.cursoredFile()->isHardSelected()) {
          list.cursoredFile()->unHardSelect();
        }
        else {
          list.cursoredFile()->hardSelect();
        }
        if (!keyDown()) {
          ui->update();
        }
      }
      break;
    case KEYACTION_HIGHLIGHT_LINE:
      if (list.cursoredFile() == NULL) {
        break;
      }
      temphighlightline = true;
      ui->redraw();
      break;
    case KEYACTION_INFO:
      if (list.cursoredFile() != nullptr) {
        ui->goFileInfo(list.cursoredFile());
      }
      break;
    case KEYACTION_TOGGLE_SHOW_NAMES_ONLY:
      nameonly = !nameonly;
      ui->redraw();
      break;
    case KEYACTION_MKDIR:
      ui->goMakeDir("", list);
      break;
  }
  return BrowseScreenAction();
}

std::string BrowseScreenLocal::getLegendText(int scope) const {
  if (gotomode) {
    return "[Any] Go to first matching entry name - [Esc] Cancel";
  }
  if (filtermodeinput) {
    return "[Any] Enter space separated filters. Valid operators are !, *, ?. Must match all negative filters and at least one positive if given. Case insensitive. - [Esc] Cancel";
  }
  if (filtermodeinputregex) {
    return "[Any] Enter regex input - [Tab] switch mode - [Esc] Cancel";
  }
  if (gotopathinput) {
    return "[Any] Enter path to go to - [Esc] Cancel";
  }
  return keybinds.getLegendSummary(scope);
}

std::string BrowseScreenLocal::getInfoLabel() const {
  return "LOCAL BROWSING";
}

std::string BrowseScreenLocal::getInfoText() const {
  std::string text = list.getPath().toString();
  if (!requests.empty()) {
    const BrowseScreenRequest & request = requests.front();
    std::string target = targetName(request.files);
    if (requests.size() > 1) {
      target += " (+" + std::to_string(requests.size() - 1) + " more)";
    }
    switch (request.type) {
      case BrowseScreenRequestType::DELETE:
        text = "Deleting " + target + "  ";
        break;
      case BrowseScreenRequestType::PATH_INFO:
        text = "Summarizing " + target + "  ";
        break;
      case BrowseScreenRequestType::FILELIST:
        text = "Getting list for " + request.path.toString() + "  ";
        break;
      case BrowseScreenRequestType::MKDIR:
        text = "Creating directory " + target + "  ";
        break;
      case BrowseScreenRequestType::MOVE:
        text = "Moving/renaming " + target + "  ";
        break;
      default:
        assert(false);
        break;
    }
    switch(spinnerpos++ % 4) {
      case 0:
        text += "|";
        break;
      case 1:
        text += "/";
        break;
      case 2:
        text += "-";
        break;
      case 3:
        text += "\\";
        break;
    }
    return text;
  }
  if (tickcount++ < 8 && lastinfo != LastInfo::NONE) {
    switch (lastinfo) {
      case LastInfo::CHANGED_SORT:
        text = "Sort method: ";
        break;
      case LastInfo::CWD_FAILED:
        text = "CWD failed: ";
        break;
      case LastInfo::DELETE_SUCCESS:
        text = "Delete successful: ";
        break;
      case LastInfo::DELETE_FAILED:
        text = "Delete failed: ";
        break;
      case LastInfo::MKDIR_SUCCESS:
        text = "Dir creation successful: ";
        break;
      case LastInfo::MKDIR_FAILED:
        text = "Dir creation failed: ";
        break;
      case LastInfo::MOVE_SUCCESS:
        text = "Move/rename successful: ";
        break;
      case LastInfo::MOVE_FAILED:
        text = "Move/rename failed: ";
        break;
      case LastInfo::NONE:
      default:
        break;
    }
    return text + lastinfotarget;
  }
  if (list.hasWildcardFilters() || list.hasRegexFilter() || list.getCompareListMode() != CompareMode::NONE) {
    if (list.hasWildcardFilters()) {
      text += "  FILTER: " + util::join(list.getWildcardFilters());
    }
    else if (list.hasRegexFilter()) {
      text += "  REGEX FILTER: " + list.getRegexFilterString();
    }
    if (list.getCompareListMode() == CompareMode::UNIQUE) {
      text += "  UNIQUES";
    }
    else if (list.getCompareListMode() == CompareMode::IDENTICAL) {
      text += "  IDENTICALS";
    }
    text += "  " + std::to_string(list.filteredSizeFiles()) + "/" + std::to_string(list.sizeFiles()) + "f " +
        std::to_string(list.filteredSizeDirs()) + "/" + std::to_string(list.sizeDirs()) + "d";
    text += std::string("  ") + util::parseSize(list.getFilteredTotalSize()) + "/" +
        util::parseSize(list.getTotalSize());
  }
  else {
    text += "  " + std::to_string(list.sizeFiles()) + "f " + std::to_string(list.sizeDirs()) + "d";
    text += std::string("  ") + util::parseSize(list.getTotalSize());
  }
  if (list.isInitialized()) {
    text += "  Free: " + util::parseSize(freespace);
  }
  return text;
}

void BrowseScreenLocal::setFocus(bool focus) {
  this->focus = focus;
  if (!focus) {
    disableGotoMode();
    filtermodeinput = false;
    filtermodeinputregex = false;
    gotopathinput = false;
  }
}

void BrowseScreenLocal::refreshFilelist() {
  BrowseScreenRequest request;
  request.type = BrowseScreenRequestType::FILELIST;
  request.path = list.getPath();
  request.id = global->getLocalStorage()->requestLocalFileList(request.path);
  requests.push_back(request);
}

void BrowseScreenLocal::tick(int) {
  if (gotomode && !gotomodefirst) {
    if (gotomodeticker++ >= 20) {
      disableGotoMode();
    }
  }
}

UIFile * BrowseScreenLocal::selectedFile() const {
  return list.cursoredFile();
}

void BrowseScreenLocal::disableGotoMode() {
  if (gotomode) {
    gotomode = false;
    global->getTickPoke()->stopPoke(this, 0);
    ui->setLegend();
  }
}

void BrowseScreenLocal::gotoPath(int requestid, const Path & path) {
  std::shared_ptr<LocalFileList> filelist = global->getLocalStorage()->getLocalFileList(requestid);
  if (!filelist) {
    lastinfo = LastInfo::CWD_FAILED;
    lastinfotarget = path.toString();
    tickcount = 0;
    return;
  }
  this->filelist = filelist;
  if (list.cursoredFile() != NULL) {
    selectionhistory.push_front(std::pair<Path, std::string>(list.getPath(), list.cursoredFile()->getName()));
  }
  else {
    currentviewspan = 0;
  }
  FileSystem::SpaceInfo info = FileSystem::getSpaceInfo(filelist->getPath());
  freespace = info.avail;
  unsigned int position = 0;
  bool hasregexfilter = false;
  std::list<std::string> wildcardfilters;
  std::regex regexfilter;
  std::string regexfilterstr;
  std::set<std::string> comparefiles;
  CompareMode comparemode = CompareMode::NONE;
  if (list.getPath() == filelist->getPath()) {
    position = list.currentCursorPosition();
    hasregexfilter = list.hasRegexFilter();
    if (hasregexfilter) {
      regexfilter = list.getRegexFilter();
      regexfilterstr = list.getRegexFilterString();
    }
    else {
      wildcardfilters = list.getWildcardFilters();
    }
    comparemode = list.getCompareListMode();
    comparefiles = list.getCompareList();
  }
  list.parse(filelist);
  if (hasregexfilter) {
    list.setRegexFilter(regexfilter, regexfilterstr);
  }
  else if (wildcardfilters.size()) {
    list.setWildcardFilters(wildcardfilters);
  }
  if (comparefiles.size()) {
    list.setCompareList(comparefiles, comparemode);
  }
  list.sortMethod(sortmethod);
  for (std::list<std::pair<Path, std::string> >::iterator it = selectionhistory.begin(); it != selectionhistory.end(); it++) {
    if (it->first == filelist->getPath()) {
      bool selected = list.selectFileName(it->second);
      selectionhistory.erase(it);
      if (selected) {
        return;
      }
      break;
    }
  }
  if (position) {
    list.setCursorPosition(position);
  }
}

std::shared_ptr<LocalFileList> BrowseScreenLocal::fileList() const {
  return filelist;
}

void BrowseScreenLocal::clearSoftSelects() {
  if (softselecting) {
    list.clearSoftSelected();
    ui->redraw();
    softselecting = false;
  }
}

UIFileList * BrowseScreenLocal::getUIFileList() {
  return &list;
}

void BrowseScreenLocal::viewCursored() {
  //view selected file, do nothing if a directory is selected
  if (list.cursoredFile() != NULL) {
    if (list.cursoredFile()->isDirectory()) {
      ui->goInfo("Cannot use the file viewer on a directory.");
    }
    else {
      ui->goViewFile(filelist->getPath(), list.cursoredFile()->getName());
    }
  }
}

bool BrowseScreenLocal::keyDown() {
  clearSoftSelects();
  //go down and highlight next item (if not at bottom already)
  bool update = list.goNext();
  if (list.currentCursorPosition() >= currentviewspan + row) {
    ui->redraw();
    return true;
  }
  else if (update) {
    table.goDown();
  }
  return false;
}

void BrowseScreenLocal::initiateMove(const std::string& dstpath) {
  std::list<std::pair<std::string, bool>> items = list.getSelectedNames();
  if (!items.empty()) {
    ui->goMove(targetName(items), list.getPath(), dstpath, items.front().first);
  }
}

