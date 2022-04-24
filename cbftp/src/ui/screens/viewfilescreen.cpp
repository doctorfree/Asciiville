#include "viewfilescreen.h"

#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectoption.h"
#include "../resizableelement.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../termint.h"
#include "../misc.h"
#include "../externalfileviewing.h"

#include "../../transferstatus.h"
#include "../../globalcontext.h"
#include "../../sitelogicmanager.h"
#include "../../sitelogic.h"
#include "../../transfermanager.h"
#include "../../localstorage.h"
#include "../../localfilelist.h"
#include "../../filelist.h"
#include "../../file.h"

#include "../../core/types.h"
#include "../../downloadfiledata.h"

class CommandOwner;

namespace ViewFileState {

enum {
  TOO_LARGE_FOR_INTERNAL,
  NO_DISPLAY,
  CONNECTING,
  DOWNLOADING,
  LOADING_VIEWER,
  VIEWING_EXTERNAL,
  VIEWING_INTERNAL,
  DOWNLOAD_FAILED
};

}

namespace {

enum KeyScopes {
  KEYSCOPE_VIEWING_INTERNAL,
  KEYSCOPE_VIEWING_EXTERNAL
};

enum KeyAction {
  KEYACTION_ENCODING,
  KEYACTION_KILL,
  KEYACTION_KILL_ALL
};
}

ViewFileScreen::ViewFileScreen(Ui* ui) : UIWindow(ui, "ViewFileScreen") {
  keybinds.addScope(KEYSCOPE_VIEWING_INTERNAL, "During internal viewing");
  keybinds.addScope(KEYSCOPE_VIEWING_EXTERNAL, "During external viewing");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up", KEYSCOPE_VIEWING_INTERNAL);
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down", KEYSCOPE_VIEWING_INTERNAL);
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page", KEYSCOPE_VIEWING_INTERNAL);
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page", KEYSCOPE_VIEWING_INTERNAL);
  keybinds.addBind('e', KEYACTION_ENCODING, "Switch encoding", KEYSCOPE_VIEWING_INTERNAL);
  keybinds.addBind('k', KEYACTION_KILL, "Kill external viewer", KEYSCOPE_VIEWING_EXTERNAL);
}

ViewFileScreen::~ViewFileScreen() {

}

void ViewFileScreen::initialize() {
  rawcontents.clear();
  tmpdata.clear();
  x = 0;
  y = 0;
  ymax = 0;
  xmax = 0;
  externallyviewable = false;
  legendupdated = false;
  pid = 0;
  autoupdate = true;
  ts.reset();
}

void ViewFileScreen::initialize(unsigned int row, unsigned int col, const std::string & site, const std::string & file, const std::shared_ptr<FileList>& fl) {
  initialize();
  deleteafter = true;
  this->site = site;
  this->file = file;
  filelist = fl;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(site);
  size = filelist->getFile(file)->getSize();
  state = ViewFileState::CONNECTING;
  if (ui->getExternalFileViewing().isViewable(file)) {
    if (!ui->getExternalFileViewing().hasDisplay()) {
      state = ViewFileState::NO_DISPLAY;
    }
    externallyviewable = true;
  }
  else {
    if (size > MAXOPENSIZE) {
      state = ViewFileState::TOO_LARGE_FOR_INTERNAL;
    }
  }
  if (state == ViewFileState::CONNECTING) {
    requestid = sitelogic->requestDownloadFile(ui, filelist, file, !externallyviewable);
  }
  init(row, col);
}

void ViewFileScreen::initialize(unsigned int row, unsigned int col, const Path & dir, const std::string & file) {
  initialize();
  deleteafter = false;
  path = dir / file;
  this->file = file;
  size = global->getLocalStorage()->getFileSize(path);
  state = ViewFileState::LOADING_VIEWER;
  if (ui->getExternalFileViewing().isViewable(file)) {
    if (!ui->getExternalFileViewing().hasDisplay()) {
      state = ViewFileState::NO_DISPLAY;
    }
    externallyviewable = true;
  }
  else {
    if (size > MAXOPENSIZE) {
      state = ViewFileState::TOO_LARGE_FOR_INTERNAL;
    }
  }
  init(row, col);
}

void ViewFileScreen::redraw() {
  vv->clear();
  switch (state) {
    case ViewFileState::TOO_LARGE_FOR_INTERNAL:
      vv->putStr(1, 1, file + " is too large to download and open in the internal viewer.");
      vv->putStr(2, 1, "The maximum file size for internal viewing is set to " + std::to_string(MAXOPENSIZE) + " bytes.");
      break;
    case ViewFileState::NO_DISPLAY:
      vv->putStr(1, 1, file + " cannot be opened in an external viewer.");
      vv->putStr(2, 1, "The DISPLAY environment variable is not set.");
      break;
    case ViewFileState::DOWNLOAD_FAILED: {
      vv->putStr(1, 1, "Download of " + file + " from " + site + " failed.");
      int x = 3;
      if (ts) {
        for (const std::string& line : ts->getLogLines()) {
          vv->putStr(x++, 1, line);
        }
      }
      autoupdate = false;
      break;
    }
    case ViewFileState::CONNECTING: {
      void* data = sitelogic->getOngoingRequestData(requestid);
      if (data) {
        DownloadFileData* dlfdata = static_cast<DownloadFileData*>(data);
        ts = dlfdata->ts;
        if (!!ts) {
          state = ViewFileState::DOWNLOADING;
          ts->setAwaited(true);
          path = ts->getTargetPath() / file;
          expectbackendpush = true;
          redraw();
        }
      }
      else {
        if (sitelogic->requestReady(requestid)) {
          if (!sitelogic->requestStatus(requestid)) {
            state = ViewFileState::DOWNLOAD_FAILED;
            redraw();
            return;
          }
          DownloadFileData* dlfdata = sitelogic->getDownloadFileData(requestid);
          if (dlfdata->inmemory) {
            tmpdata = dlfdata->data;
          }
          sitelogic->finishRequest(requestid);
          loadViewer();
        }
        else {
          vv->putStr(1, 1, "Awaiting download slot...");
        }
      }
      break;
    }
    case ViewFileState::DOWNLOADING:
      if (sitelogic->requestReady(requestid)) {
        if (!sitelogic->requestStatus(requestid)) {
          state = ViewFileState::DOWNLOAD_FAILED;
          redraw();
          return;
        }
        DownloadFileData* dlfdata = sitelogic->getDownloadFileData(requestid);
        if (dlfdata->inmemory) {
          tmpdata = dlfdata->data;
        }
        sitelogic->finishRequest(requestid);
        loadViewer();
      }
      else if (ts->getState() == TRANSFERSTATUS_STATE_FAILED) {
        state = ViewFileState::DOWNLOAD_FAILED;
        redraw();
      }
      else {
        vv->putStr(1, 1, "Downloading from " + site + "...");
        printTransferInfo();
      }
      break;
    case ViewFileState::LOADING_VIEWER:
      loadViewer();
      break;
    case ViewFileState::VIEWING_EXTERNAL:
      viewExternal();
      break;
    case ViewFileState::VIEWING_INTERNAL:
      viewInternal();
      break;
  }
}

void ViewFileScreen::update() {
  if (pid) {
    if (!ui->getExternalFileViewing().stillViewing(pid)) {
      ui->returnToLast();
    }
    else if (!legendupdated) {
      legendupdated = true;
      ui->update();
      ui->setLegend();
    }
  }
  else {
    redraw();
    if ((state == ViewFileState::VIEWING_INTERNAL || state == ViewFileState::VIEWING_EXTERNAL)
        && !legendupdated)
    {
      legendupdated = true;
      ui->update();
      ui->setLegend();
    }
  }
}

bool ViewFileScreen::keyPressed(unsigned int ch) {
  int scope = getCurrentScope();
  int action = keybinds.getKeyAction(ch, scope);
  switch(action) {
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DOWN:
      if (goDown()) {
        goDown();
        ui->setInfo();
        ui->redraw();
        return true;
      }
      return false;
    case KEYACTION_UP:
      if (goUp()) {
        goUp();
        ui->setInfo();
        ui->redraw();
        return true;
      }
      return false;
    case KEYACTION_NEXT_PAGE:
      for (unsigned int i = 0; i < row / 2; i++) {
        goDown();
      }
      ui->setInfo();
      ui->redraw();
      return true;
    case KEYACTION_PREVIOUS_PAGE:
      for (unsigned int i = 0; i < row / 2; i++) {
        goUp();
      }
      ui->setInfo();
      ui->redraw();
      return true;
    case KEYACTION_KILL:
      if (pid) {
        ui->getExternalFileViewing().killViewer(pid);
      }
      return true;
    case KEYACTION_ENCODING:
      if (state == ViewFileState::VIEWING_INTERNAL) {
        if (encoding == encoding::ENCODING_CP437) {
          encoding = encoding::ENCODING_CP437_DOUBLE;
        }
        else if (encoding == encoding::ENCODING_CP437_DOUBLE) {
          encoding = encoding::ENCODING_ISO88591;
        }
        else if (encoding == encoding::ENCODING_ISO88591) {
          encoding = encoding::ENCODING_UTF8;
        }
        else {
          encoding = encoding::ENCODING_CP437;
        }
        translate();
        ui->redraw();
        ui->setInfo();
      }
      return true;
  }
  return false;
}

void ViewFileScreen::loadViewer() {
  if (externallyviewable) {
    if (!pid) {
      if (deleteafter) {
        pid = ui->getExternalFileViewing().viewThenDelete(path);
      }
      else {
        pid = ui->getExternalFileViewing().view(path);
      }
    }
    state = ViewFileState::VIEWING_EXTERNAL;
    viewExternal();
  }
  else {
    if (tmpdata.empty()) {
      tmpdata = global->getLocalStorage()->getFileContent(path);
    }
    if (deleteafter) {
      global->getLocalStorage()->requestDelete(path);
    }
    encoding = encoding::guessEncoding(tmpdata);
    unsigned int tmpdatalen = tmpdata.size();
    if (tmpdatalen > 0) {
      std::string current;
      for (unsigned int i = 0; i < tmpdatalen; i++) {
        if (tmpdata[i] == '\n') {
          rawcontents.push_back(current);
          current.clear();
        }
        else {
          current += tmpdata[i];
        }
      }
      if (current.length() > 0) {
        rawcontents.push_back(current);
      }
      translate();
    }
    autoupdate = false;
    state = ViewFileState::VIEWING_INTERNAL;
    viewInternal();
  }
}

void ViewFileScreen::viewExternal() {
  vv->putStr(1, 1, "Opening " + file + " with: " + ui->getExternalFileViewing().getViewApplication(file));
  vv->putStr(3, 1, "Press 'k' to kill this external viewer instance.");
}

void ViewFileScreen::viewInternal() {
  if (ymax <= row) {
    y = 0;
  }
  while (ymax > row && y + row > ymax) {
    --y;
  }
  ymax = rawcontents.size();
  for (unsigned int i = 0; i < ymax; i++) {
    if (translatedcontents[i].length() > xmax) {
      xmax = translatedcontents[i].length();
    }
  }
  for (unsigned int i = 0; i < row && i < ymax; i++) {
    std::basic_string<unsigned int> & line = translatedcontents[y + i];
    for (unsigned int j = 0; j < line.length() && j < col - 2; j++) {
      vv->putChar(i, j + 1, line[j]);
    }
  }

  printSlider(vv, row, col - 1, ymax, y);
}

std::string ViewFileScreen::getLegendText() const {
  return keybinds.getLegendSummary(getCurrentScope());
}

std::string ViewFileScreen::getInfoLabel() const {
  return "VIEW FILE: " + file;
}

std::string ViewFileScreen::getInfoText() const {
  if (state == ViewFileState::VIEWING_INTERNAL) {
    std::string enc;
    switch (encoding) {
      case encoding::ENCODING_CP437:
        enc = "CP437";
        break;
      case encoding::ENCODING_CP437_DOUBLE:
        enc = "Double CP437";
        break;
      case encoding::ENCODING_ISO88591:
        enc = "ISO-8859-1";
        break;
      case encoding::ENCODING_UTF8:
        enc = "UTF-8";
        break;
    }
    unsigned int end = ymax < y + row ? ymax : y + row;
    return "Line " + std::to_string(y) + "-" +
        std::to_string(end) + "/" + std::to_string(ymax) + "  Encoding: " + enc;
  }
  else {
    return "";
  }
}

void ViewFileScreen::translate() {
  translatedcontents.clear();
  for (unsigned int i = 0; i < rawcontents.size(); i++) {
    std::basic_string<unsigned int> current;
    if (encoding == encoding::ENCODING_CP437_DOUBLE) {
      current = encoding::doublecp437toUnicode(rawcontents[i]);
    }
    else if (encoding == encoding::ENCODING_CP437) {
      current = encoding::cp437toUnicode(rawcontents[i]);
    }
    else if (encoding == encoding::ENCODING_ISO88591) {
      current = encoding::toUnicode(rawcontents[i]);
    }
    else {
      current = encoding::utf8toUnicode(rawcontents[i]);
    }
    translatedcontents.push_back(current);
  }
}

bool ViewFileScreen::goDown() {
  if (y + row < ymax) {
    y++;
    return true;
  }
  return false;
}

bool ViewFileScreen::goUp() {
  if (y > 0) {
    y--;
    return true;
  }
  return false;
}

void ViewFileScreen::printTransferInfo() {
  TransferDetails td = TransfersScreen::formatTransferDetails(ts);
  unsigned int y = 3;
  MenuSelectOption table(*vv);
  std::shared_ptr<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;
  msotb = table.addTextButtonNoContent(y, 1, "transferred", "TRANSFERRED");
  msal->addElement(msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 3, "filename", "FILENAME");
  msal->addElement(msotb, 2, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 6, "remaining", "LEFT");
  msal->addElement(msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 7, "speed", "SPEED");
  msal->addElement(msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 8, "progress", "DONE");
  msal->addElement(msotb, 7, RESIZE_REMOVE);
  y++;
  msal = table.addAdjustableLine();
  msotb = table.addTextButtonNoContent(y, 1, "transferred", td.transferred);
  msal->addElement(msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 10, "filename", ts->getFile());
  msal->addElement(msotb, 2, RESIZE_WITHLAST3);
  msotb = table.addTextButtonNoContent(y, 60, "remaining", td.timeremaining);
  msal->addElement(msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 40, "speed", td.speed);
  msal->addElement(msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 50, "progress", td.progress);
  msal->addElement(msotb, 7, RESIZE_REMOVE);
  table.adjustLines(col - 3);
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    if (re->isVisible()) {
      if (re->getIdentifier() == "transferred") {
        std::string labeltext = re->getLabelText();
        bool highlight = table.getLineIndex(table.getAdjustableLine(re)) == 1;
        int charswithhighlight = highlight ? labeltext.length() * ts->getProgress() / 100 : 0;
        vv->putStr(re->getRow(), re->getCol(), labeltext.substr(0, charswithhighlight), true);
        vv->putStr(re->getRow(), re->getCol() + charswithhighlight, labeltext.substr(charswithhighlight));
      }
      else {
        vv->putStr(re->getRow(), re->getCol(), re->getLabelText());
      }
    }
  }
}

int ViewFileScreen::getCurrentScope() const {
  if (state == ViewFileState::VIEWING_EXTERNAL) {
    return KEYSCOPE_VIEWING_EXTERNAL;
  }
  if (state == ViewFileState::VIEWING_INTERNAL) {
    return KEYSCOPE_VIEWING_INTERNAL;
  }
  return KEYSCOPE_ALL;
}
