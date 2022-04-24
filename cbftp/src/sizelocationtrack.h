#pragma once

#include <map>
#include <memory>

class SiteRace;

class SizeLocationTrack {
private:
  std::map<std::shared_ptr<SiteRace>, unsigned long long int> sitesizes;
  unsigned long long int estimatedsize;
  void recalculate();
public:
  SizeLocationTrack();
  unsigned long long int getEstimatedSize() const;
  int numSites() const;
  bool add(const std::shared_ptr<SiteRace>& site, unsigned long long int);
  void remove(const std::shared_ptr<SiteRace>& site);
};
