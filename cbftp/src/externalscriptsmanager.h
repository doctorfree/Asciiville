#pragma once

#include <map>
#include <string>

class ExternalScripts;

class ExternalScriptsManager {
public:
  void addExternalScripts(const std::string& name, ExternalScripts* externalscripts);
  ExternalScripts* getExternalScripts(const std::string& name);
  std::map<std::string, ExternalScripts*>::const_iterator begin() const;
  std::map<std::string, ExternalScripts*>::const_iterator end() const;
private:
  std::map<std::string, ExternalScripts*> externalscripts;
};
