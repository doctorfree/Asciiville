#include "keybinds.h"

#include <cassert>

#include "termint.h"
#include "../util.h"

QueuedReplaceBind::QueuedReplaceBind(int keyaction, int scope, const std::set<unsigned int>& newkeys) : keyaction(keyaction), scope(scope), newkeys(newkeys) {

}

KeyRepr KeyBinds::getKeyRepr(unsigned int key) {
  KeyRepr repr;
  repr.wch = key;
  switch (repr.wch) {
    case 0:
      repr.repr = "Ctrl+Space";
      return repr;
    case 9:
      repr.repr = "Tab";
      return repr;
    case 10:
      repr.repr = "Enter";
      return repr;
    case 27:
      repr.repr = "Escape";
      return repr;
    case 32:
      repr.repr = "Space";
      return repr;
    case 258:
      repr.repr = "Down";
      return repr;
    case 259:
      repr.repr = "Up";
      return repr;
    case 260:
      repr.repr = "Left";
      return repr;
    case 261:
      repr.repr = "Right";
      return repr;
    case TERMINT_HOME:
      repr.repr = "Home";
      return repr;
    case 263:
      repr.repr = "Backspace";
      return repr;
    case 330:
      repr.repr = "Delete";
      return repr;
    case 331:
      repr.repr = "Insert";
      return repr;
    case TERMINT_SHIFT_DOWN:
      repr.repr = "Shift+Down";
      return repr;
    case TERMINT_SHIFT_UP:
      repr.repr = "Shift+Up";
      return repr;
    case 338:
      repr.repr = "Page Down";
      return repr;
    case 339:
      repr.repr = "Page Up";
      return repr;
    case 353:
      repr.repr = "Shift+Tab";
      return repr;
    case TERMINT_END:
      repr.repr = "End";
      return repr;
    case 383:
      repr.repr = "Shift+Delete";
      return repr;
    case TERMINT_SHIFT_LEFT:
      repr.repr = "Shift+Left";
      return repr;
    case TERMINT_SHIFT_RIGHT:
      repr.repr = "Shift+Right";
      return repr;
    case TERMINT_CTRL_DOWN:
      repr.repr = "Ctrl+Down";
      return repr;
    case TERMINT_CTRL_LEFT:
      repr.repr = "Ctrl+Left";
      return repr;
    case TERMINT_CTRL_SHIFT_LEFT:
      repr.repr = "Ctrl+Shift+Left";
      return repr;
    case TERMINT_CTRL_RIGHT:
      repr.repr = "Ctrl+Right";
      return repr;
    case TERMINT_CTRL_SHIFT_RIGHT:
      repr.repr = "Ctrl+Shift+Right";
      return repr;
    case TERMINT_CTRL_UP:
      repr.repr = "Ctrl+Up";
      return repr;
  }
  if (repr.wch >= 1 && repr.wch <= 26) {
    char ctrlkey = repr.wch + 96;
    repr.repr = std::string("Ctrl+") + ctrlkey;
  }
  if (repr.wch >= 265 && repr.wch <= 273) {
    char fkey = repr.wch - 216;
    repr.repr = std::string("F") + fkey;
  }
  if (repr.wch >= 274 && repr.wch <= 276) {
    char fkey = repr.wch - 226;
    repr.repr = std::string("F1") + fkey;
  }
  return repr;
}

KeyBinds::KeyBinds(const std::string& name) : name(name), allowkeybinds(true), alternatebutton(false) {
  addScope(KEYSCOPE_ALL, "Common");
}

KeyBinds::KeyBinds(const KeyBinds& other) : name(other.name), keydata(other.keydata),
    scopes(other.scopes), actions(other.actions), allowkeybinds(other.allowkeybinds)
{
  regenerate();
}

void KeyBinds::addBind(const std::list<int>& keys, int keyaction, const std::string& description, int scope) {
  ActionAndScope action(keyaction, scope);
  assert(actions.find(action) == actions.end());
  assert(scopes.find(scope) != scopes.end());
  actions.insert(action);
  KeyData keydat;
  keydat.keyaction = keyaction;
  keydat.description = description;
  keydat.originalkeys = std::set<unsigned int>(keys.begin(), keys.end());
  keydat.scope = scope;
  keydat.configuredkeys = keydat.originalkeys;
  keydata.push_back(keydat);
  for (int key : keys) {
    KeyAndScope token(key, scope);
    std::map<KeyAndScope, std::list<KeyData>::iterator>::const_iterator it = keybinds.find(token);
    assert(it == keybinds.end());
  }
  applyQueuedReplaceBinds();
  regenerate();
}

void KeyBinds::addBind(int key, int keyaction, const std::string& description, int scope) {
  std::list<int> keys = {key};
  addBind(keys, keyaction, description, scope);
}

void KeyBinds::addUnboundBind(int keyaction, const std::string& description, int scope) {
  addBind(std::list<int>(), keyaction, description, scope);
}

bool KeyBinds::hasBind(int keyaction, int scope) const {
  ActionAndScope action(keyaction, scope);
  return actions.find(action) != actions.end();
}

void KeyBinds::addScope(int scope, const std::string& description) {
  scopes[scope] = description;
}

void KeyBinds::addCustomBind(int keyaction, int scope, int newkey) {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    if (it->scope == scope && it->keyaction == keyaction) {
      it->configuredkeys.insert(newkey);
      regenerate();
      return;
    }
  }
}

void KeyBinds::replaceBind(int keyaction, int scope, const std::set<unsigned int>& newkeys) {
  queuedreplacebinds.emplace_back(keyaction, scope, newkeys);
  applyQueuedReplaceBinds();
}

void KeyBinds::replaceBind(int keyaction, int scope, unsigned int newkey) {
  std::set<unsigned int> newkeys = {newkey};
  replaceBind(keyaction, scope, newkeys);
}

void KeyBinds::resetBind(int keyaction, int scope) {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    if (it->scope == scope && it->keyaction == keyaction) {
      if (it->configuredkeys == it->originalkeys) {
        return;
      }
      it->configuredkeys = it->originalkeys;
      regenerate();
      return;
    }
  }
}

void KeyBinds::unbind(int keyaction, int scope) {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    if (it->scope == scope && it->keyaction == keyaction) {
      it->configuredkeys.clear();
      regenerate();
      return;
    }
  }
}

void KeyBinds::removeBind(int keyaction, int scope) {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    if (it->scope == scope && it->keyaction == keyaction) {
      keydata.erase(it);
      regenerate();
      return;
    }
  }
}

void KeyBinds::setBindDescription(int keyaction, const std::string& description, int scope) {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    if (it->scope == scope && it->keyaction == keyaction) {
      it->description = description;
      regenerate();
      return;
    }
  }
}

void KeyBinds::resetAll() {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    it->configuredkeys = it->originalkeys;
  }
  regenerate();
}

int KeyBinds::getKeyAction(int key, int scope) const {
  if (key == 27) {
    return KEYACTION_BACK_CANCEL;
  }
  if (allowkeybinds && ((key == '?' && !alternatebutton) || (key == 276 && alternatebutton))) {
    return KEYACTION_KEYBINDS;
  }
  if (key == '1') {
    return KEYACTION_1;
  }
  if (key == '2') {
    return KEYACTION_2;
  }
  if (key == '3') {
    return KEYACTION_3;
  }
  if (key == '4') {
    return KEYACTION_4;
  }
  if (key == '5') {
    return KEYACTION_5;
  }
  if (key == '6') {
    return KEYACTION_6;
  }
  if (key == '7') {
    return KEYACTION_7;
  }
  if (key == '8') {
    return KEYACTION_8;
  }
  if (key == '9') {
    return KEYACTION_9;
  }
  KeyAndScope token(key, scope);
  auto it = keybinds.find(token);
  if (it == keybinds.end()) {
    if (token.second != KEYSCOPE_ALL) {
      token.second = KEYSCOPE_ALL;
      it = keybinds.find(token);
    }
  }
  if (it != keybinds.end()) {
    return it->second->keyaction;
  }
  return KEYACTION_NONE;
}

std::string KeyBinds::getName() const {
  return name;
}

std::list<KeyBinds::KeyData> KeyBinds::getBindsForScope(int scope) const {
  std::list<KeyBinds::KeyData> out;
  for (const KeyData& keydat : keydata) {
    if (keydat.scope == scope) {
      out.push_back(keydat);
    }
  }
  return out;
}

std::map<int, std::string>::const_iterator KeyBinds::scopesBegin() const {
  return scopes.begin();
}

std::map<int, std::string>::const_iterator KeyBinds::scopesEnd() const {
  return scopes.end();
}

std::string KeyBinds::getLegendSummary(int scope) const {
  auto it = legendsummaries.find(scope);
  if (it != legendsummaries.end()) {
    return it->second;
  }
  return "";
}

void KeyBinds::generateLegendSummaries() {
  std::string allkeybindstext = "[" + (alternatebutton ? getKeyRepr(276).repr : "?") + "] All keybinds";
  for (const std::pair<const int, std::string>& scope : scopes) {
    std::list<KeyBinds::KeyData> binds = getBindsForScope(scope.first);
    std::string summary;
    if (allowkeybinds && scope.first != KEYSCOPE_ALL) {
      summary += allkeybindstext;
    }
    for (const KeyBinds::KeyData& keydata : binds) {
      summary += summary.empty() ? "" : " - ";
      std::string keystr;
      for (unsigned int nkey : keydata.configuredkeys) {
        KeyRepr repr = getKeyRepr(nkey);
        std::string append = keydata.description;
        std::string key = repr.repr.empty() ? std::string() + static_cast<char>(repr.wch) : repr.repr;
        keystr += (keystr.empty() ? "" : "/") + key;
      }
      if (scope.first == KEYSCOPE_ALL && keydata.keyaction == KEYACTION_BACK_CANCEL && keystr.find("Esc") == std::string::npos) {
        keystr = "Esc/" + keystr;
      }
      std::string append = keydata.description;
      size_t pos = util::toLower(append).find(util::toLower(keystr));
      if (pos != std::string::npos) {
        append = append.substr(0, pos) + "[" + keystr + "]" + append.substr(pos + keystr.length());
      }
      else {
        append = "[" + keystr + "] " + append;
      }
      summary += append;
    }
    legendsummaries[scope.first] = summary;
  }
  std::string all = legendsummaries[KEYSCOPE_ALL];
  if (all.empty()) {
    return;
  }
  for (std::map<int, std::string>::iterator it = legendsummaries.begin(); it != legendsummaries.end(); ++it) {
    if (it->first != KEYSCOPE_ALL) {
      it->second += (!it->second.empty() ? " - " : "") + all;
    }
  }
  if (allowkeybinds) {
    legendsummaries[KEYSCOPE_ALL] = allkeybindstext + " - " + legendsummaries[KEYSCOPE_ALL];
  }
}

std::list<KeyBinds::KeyData>::const_iterator KeyBinds::begin() const {
  return keydata.begin();
}

std::list<KeyBinds::KeyData>::const_iterator KeyBinds::end() const {
  return keydata.end();
}

bool KeyBinds::hasExtraScopes() const {
  return scopes.size() > 1;
}

void KeyBinds::disallowKeybinds() {
  allowkeybinds = false;
  generateLegendSummaries();
}

void KeyBinds::useAlternateKeybindsButton() {
  alternatebutton = true;
  generateLegendSummaries();
}

void KeyBinds::generateIndex() {
  keybinds.clear();
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    for (int key : it->configuredkeys) {
      KeyAndScope token(key, it->scope);
      keybinds[token] = it;
    }
  }
}

void KeyBinds::regenerate() {
  generateIndex();
  generateLegendSummaries();
}

void KeyBinds::applyQueuedReplaceBinds() {
  bool regen = false;
  for (std::list<QueuedReplaceBind>::const_iterator it = queuedreplacebinds.begin(); it != queuedreplacebinds.end();) {
    bool found = false;
    for (std::list<KeyData>::iterator it2 = keydata.begin(); it2 != keydata.end(); ++it2) {
      if (it2->scope == it->scope && it2->keyaction == it->keyaction) {
        it2->configuredkeys = it->newkeys;
        regen = true;
        found = true;
        it = queuedreplacebinds.erase(it);
        break;
      }
    }
    if (!found) {
      ++it;
    }
  }
  if (regen) {
    regenerate();
  }
}

