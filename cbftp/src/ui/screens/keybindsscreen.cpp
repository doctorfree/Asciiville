#include "keybindsscreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../engine.h"
#include "../../race.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptiontextarrow.h"
#include "../menuselectoptionelement.h"
#include "../focusablearea.h"
#include "../keybinds.h"

namespace {
enum KeyActions {
  KEYACTION_RESET_ALL,
  KEYACTION_ADD_KEY
};

}
KeyBindsScreen::KeyBindsScreen(Ui* ui) : UIWindow(ui, "KeyBindsScreen"), mso(*vv) {
  keybinds.addBind('d', KEYACTION_DONE, "Done, save changes");
  keybinds.addBind(10, KEYACTION_ENTER, "New key");
  keybinds.addBind({'+', 'A'}, KEYACTION_ADD_KEY, "Add extra key");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind('r', KEYACTION_RESET, "Reset to default");
  keybinds.addBind('R', KEYACTION_RESET_ALL, "Reset all to default");
  keybinds.addBind(KEY_DC, KEYACTION_DELETE, "Unbind");
  keybinds.disallowKeybinds();
}

KeyBindsScreen::~KeyBindsScreen() {

}

void KeyBindsScreen::initialize(unsigned int row, unsigned int col, KeyBinds* keybinds) {
  active = false;
  realkeybinds = keybinds;
  tempkeybinds = std::make_shared<KeyBinds>(*realkeybinds);
  mso.reset();
  repopulate();
  mso.enterFocusFrom(0);
  init(row, col);
}

void KeyBindsScreen::repopulate() {
  actionandscope.clear();
  unsigned int y = 1;
  mso.clear();
  std::map<int, std::string>::const_iterator it;
  for (it = tempkeybinds->scopesBegin(); it != tempkeybinds->scopesEnd(); ++it) {
    int scope = it->first;
    std::list<KeyBinds::KeyData> binds = tempkeybinds->getBindsForScope(scope);
    if (tempkeybinds->hasExtraScopes()) {
      std::shared_ptr<MenuSelectOptionTextButton> header = mso.addTextButtonNoContent(y++, 1, it->second, it->second + ":");
      header->setSelectable(false);
    }
    for (const KeyBinds::KeyData& keydata : binds) {
      actionandscope.emplace_back(keydata.keyaction, keydata.scope);
      std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
      std::shared_ptr<MenuSelectOptionTextButton> desc = mso.addTextButtonNoContent(y, 1, keydata.description, keydata.description);
      desc->setSelectable(false);
      msal->addElement(desc, 1, RESIZE_WITHDOTS);
      std::string keytext;
      std::string origkeytext;
      unsigned int setwch = 0;
      if (keydata.configuredkeys.empty()) {
        keytext = "None";
      }
      else {
        for (unsigned int key : keydata.configuredkeys) {
          KeyRepr repr = KeyBinds::getKeyRepr(key);
          std::string keyrepr = repr.repr.empty() ? std::string() + static_cast<char>(repr.wch) : repr.repr;
          keytext += (keytext.empty() ? "" : ",") + keyrepr;
          if (repr.repr.empty() && setwch == 0) {
            setwch = repr.wch;
          }
        }
      }
      for (unsigned int key : keydata.originalkeys) {
        KeyRepr repr = KeyBinds::getKeyRepr(key);
        std::string keyrepr = repr.repr.empty() ? std::string() + static_cast<char>(repr.wch) : repr.repr;
        origkeytext += (origkeytext.empty() ? "" : ",") + keyrepr;
      }
      if (keydata.originalkeys != keydata.configuredkeys && !origkeytext.empty()) {
        keytext += " (Default: " + origkeytext + ")";
      }
      std::shared_ptr<MenuSelectOptionTextButton> boundkey = mso.addTextButtonNoContent(y++, 1, keydata.description + "-bind", keytext);
      boundkey->setId(setwch);
      boundkey->setOrigin(static_cast<void*>(&actionandscope.back()));
      boundkey->setLegendText("[Esc] Cancel - [Any] set key bind");
      msal->addElement(boundkey, 2, RESIZE_REMOVE, true);
    }
    y++;
  }
}
void KeyBindsScreen::redraw() {
  vv->clear();
  mso.adjustLines(col - 3);
  bool highlight;

  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    std::string text = msoe->isActive() ? "<Press " + std::string(addextrakey ? "additional" : "new") + " key>" : msoe->getLabelText();
    if (!text.empty()) {
      int add = 0;
      if (text[0] == ' ') {
        vv->putChar(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getId(), highlight);
        ++add;
      }
      vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1 + add, text, highlight);
    }
    else {
      vv->putChar(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getId(), highlight);
    }
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
  }
}

bool KeyBindsScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch != 27 && activeelement->getOrigin()) {
      std::pair<int, int> actionscope = *static_cast<std::pair<int, int>*>(activeelement->getOrigin());
      if (addextrakey) {
        tempkeybinds->addCustomBind(actionscope.first, actionscope.second, ch);
      }
      else {
        tempkeybinds->replaceBind(actionscope.first, actionscope.second, ch);
      }
    }
    activeelement->deactivate();
    active = false;
    repopulate();
    ui->update();
    ui->setLegend();
    return true;
  }
  int action = keybinds.getKeyAction(ch);
  switch(action) {
    case KEYACTION_UP:
      if (mso.goUp()) {
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_DOWN:
      if (mso.goDown()) {
        ui->update();
        return true;
      }
      return false;
    case KEYACTION_ENTER:
      mso.getElement(mso.getSelectionPointer())->activate();
      addextrakey = false;
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_ADD_KEY:
      mso.getElement(mso.getSelectionPointer())->activate();
      addextrakey = true;
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_DONE:
      realkeybinds->resetAll();
      for (std::list<KeyBinds::KeyData>::const_iterator it = tempkeybinds->begin(); it != tempkeybinds->end(); ++it) {
        if (it->originalkeys != it->configuredkeys) {
          realkeybinds->replaceBind(it->keyaction, it->scope, it->configuredkeys);
        }
      }
      ui->returnToLast();
      return true;
    case KEYACTION_RESET: {
      void* origin = mso.getElement(mso.getSelectionPointer())->getOrigin();
      if (origin) {
        std::pair<int, int> actionscope = *static_cast<std::pair<int, int>*>(origin);
        tempkeybinds->resetBind(actionscope.first, actionscope.second);
        repopulate();
        ui->update();
      }
      return true;
    }
    case KEYACTION_DELETE: {
      void* origin = mso.getElement(mso.getSelectionPointer())->getOrigin();
      if (origin) {
        std::pair<int, int> actionscope = *static_cast<std::pair<int, int>*>(origin);
        tempkeybinds->unbind(actionscope.first, actionscope.second);
        repopulate();
        ui->update();
      }
      return true;
    }
    case KEYACTION_RESET_ALL:
      tempkeybinds->resetAll();
      repopulate();
      ui->update();
      return true;
  }
  return false;
}

std::string KeyBindsScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string KeyBindsScreen::getInfoLabel() const {
  return "KEYBINDS FOR: " + tempkeybinds->getName();
}
