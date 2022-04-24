#pragma once

#include <string>
#include <list>
#include <utility>

class PreparedRace {
public:
  PreparedRace(unsigned int id, const std::string& release, const std::string& section, const std::list<std::pair<std::string, bool>>& sites, bool reset, int ttl);
  unsigned int getId() const;
  const std::string& getName() const;
  const std::string& getSection() const;
  bool getReset() const;
  const std::list<std::pair<std::string, bool>>& getSites() const;
  int getRemainingTime() const;
  void tick();
private:
  unsigned int id;
  const std::string name;
  const std::string section;
  const std::list<std::pair<std::string, bool>> sites;
  bool reset;
  int ttl;
};
