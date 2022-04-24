#include "proxyoptionsscreen.h"

#include "../ui.h"
#include "../menuselectoptiontextarrow.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextbutton.h"

#include "../../globalcontext.h"
#include "../../proxy.h"
#include "../../proxymanager.h"
#include "../../sitemanager.h"
#include "../../eventlog.h"

ProxyOptionsScreen::ProxyOptionsScreen(Ui* ui) : UIWindow(ui, "ProxyOptionsScreen"), mso(*vv), msop(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind(KEY_DC, KEYACTION_DELETE, "Delete proxy");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

ProxyOptionsScreen::~ProxyOptionsScreen() {

}

void ProxyOptionsScreen::initialize(unsigned int row, unsigned int col) {
  active = false;
  defaultset = false;
  defaultdataset = false;
  deleteproxy = "";
  editproxy = "";
  pm = global->getProxyManager();
  unsigned int y = 1;
  unsigned int x = 1;
  mso.reset();
  msop.reset();
  useproxy = mso.addTextArrow(y++, x, "useproxy", "Default proxy:");
  dataproxy = mso.addTextArrow(y++, x, "dataproxy", "Default data proxy:");
  focusedarea = &mso;
  mso.makeLeavableDown();
  msop.makeLeavableUp();
  mso.enterFocusFrom(0);
  init(row, col);
}

void ProxyOptionsScreen::redraw() {
  vv->clear();
  if (editproxy != "" && pm->getProxy(editproxy) == nullptr) {
    global->getSiteManager()->proxyRemoved(editproxy);
    editproxy = "";
  }
  std::string prevselect = useproxy->getDataText();
  std::string prevdataselect = dataproxy->getDataText();
  useproxy->clear();
  dataproxy->clear();
  useproxy->addOption("None", 0);
  dataproxy->addOption("None", 0);
  unsigned int y = 7;
  unsigned int x = 1;
  msop.clear();
  msop.addTextButtonNoContent(3, x, "add", "Add proxy");
  vv->putStr(5, x, "Name");
  vv->putStr(5, x + 10, "Address");
  vv->putStr(5, x + 30, "Port");
  vv->putStr(5, x + 37, "Auth");
  vv->putStr(5, x + 48, "Resolve");
  for(std::vector<Proxy *>::const_iterator it = pm->begin(); it != pm->end(); it++) {
    std::string name = (*it)->getName();
    useproxy->addOption(name, 1);
    dataproxy->addOption(name, 1);
    msop.addTextButton(y++, x, name, name);
  }
  msop.checkPointer();
  if (!defaultset) {
    if (pm->getDefaultProxy() != nullptr) {
      useproxy->setOptionText(pm->getDefaultProxy()->getName());
    }
    defaultset = true;
  }
  else {
    useproxy->setOptionText(prevselect);
  }
  if (!defaultdataset) {
    if (pm->getDefaultDataProxy() != nullptr) {
      dataproxy->setOptionText(pm->getDefaultDataProxy()->getName());
    }
    defaultdataset = true;
  }
  else {
    dataproxy->setOptionText(prevdataselect);
  }
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  for (unsigned int i = 0; i < msop.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = msop.getElement(i);
    highlight = false;
    if (msop.isFocused() && msop.getSelectionPointer() == i) {
      highlight = true;
    }
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight, 9);
    if (msoe->getLabelText() == msoe->getIdentifier()) {
      Proxy * proxy = pm->getProxy(msoe->getLabelText());
      if (proxy != NULL) {
        vv->putStr(msoe->getRow(), msoe->getCol() + 10, proxy->getAddr(), false, 19);
        vv->putStr(msoe->getRow(), msoe->getCol() + 30, proxy->getPort(), false, 5);
        vv->putStr(msoe->getRow(), msoe->getCol() + 37, proxy->getAuthMethodText());
        vv->putStr(msoe->getRow(), msoe->getCol() + 48, proxy->getResolveHosts() ? "[X]" : "[ ]");
      }
    }
  }
}

void ProxyOptionsScreen::command(const std::string & command) {
  if (command == "yes") {
    if (deleteproxy != "") {
      pm->removeProxy(deleteproxy);
      deleteproxy = "";
      ui->redraw();
      return;
    }
  }
}

bool ProxyOptionsScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  std::shared_ptr<MenuSelectOptionElement> selected;
  switch(action) {
    case KEYACTION_UP:
      if (focusedarea->goUp()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
          focusedarea->enterFocusFrom(2);
        }
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &msop;
          focusedarea->enterFocusFrom(0);
        }
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_ENTER:
      selected = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (selected->getIdentifier() == "add") {
        ui->goAddProxy();
        return true;
      }
      activation = focusedarea->activateSelected();
      if (!activation) {
        if (focusedarea == &msop) {
          ui->goEditProxy(selected->getLabelText());
          return true;
        }
        ui->update();
        return true;
      }
      active = true;
      activeelement = selected;
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_DONE:
      for(unsigned int i = 0; i < mso.size(); i++) {
        std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "useproxy") {
          pm->setDefaultProxy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getDataText());
        }
        else if (identifier == "dataproxy") {
          pm->setDefaultDataProxy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getDataText());
        }
      }
      ui->returnToLast();
      return true;
    case KEYACTION_DELETE:
      selected = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (focusedarea == &msop && selected->getLabelText() == selected->getIdentifier()) {
        editproxy = selected->getIdentifier();
        deleteproxy = focusedarea->getElement(focusedarea->getSelectionPointer())->getLabelText();
        ui->goConfirmation("Do you really want to delete " + editproxy);
      }
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string ProxyOptionsScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string ProxyOptionsScreen::getInfoLabel() const {
  return "PROXY OPTIONS";
}

std::string ProxyOptionsScreen::getInfoText() const {
  return "Proxies added: " + std::to_string(pm->size());
}
