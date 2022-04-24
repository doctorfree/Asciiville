#include "externalscriptsmanager.h"

void ExternalScriptsManager::addExternalScripts(const std::string& name, ExternalScripts* externalscripts) {
  this->externalscripts.insert(std::make_pair(name, externalscripts));
}

ExternalScripts* ExternalScriptsManager::getExternalScripts(const std::string& name) {
  std::map<std::string, ExternalScripts*>::const_iterator it = externalscripts.find(name);
  if (it != externalscripts.end()) {
    return it->second;
  }
  return nullptr;
}

std::map<std::string, ExternalScripts*>::const_iterator ExternalScriptsManager::begin() const {
  return externalscripts.begin();
}

std::map<std::string, ExternalScripts*>::const_iterator ExternalScriptsManager::end() const {
  return externalscripts.end();
}
