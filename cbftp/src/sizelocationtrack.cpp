#include "sizelocationtrack.h"

SizeLocationTrack::SizeLocationTrack() {
  estimatedsize = 0;
}

unsigned long long int SizeLocationTrack::getEstimatedSize() const {
  return estimatedsize;
}

int SizeLocationTrack::numSites() const {
  return sitesizes.size();
}

bool SizeLocationTrack::add(const std::shared_ptr<SiteRace> & site, unsigned long long int size) {
  std::map<std::shared_ptr<SiteRace>, unsigned long long int>::iterator it;
  it = sitesizes.find(site);
  if (it == sitesizes.end() || it->second != size) {
    sitesizes[site] = size;
    recalculate();
    return true;
  }
  return false;
}

void SizeLocationTrack::remove(const std::shared_ptr<SiteRace> & site) {
  std::map<std::shared_ptr<SiteRace>, unsigned long long int>::iterator it;
  it = sitesizes.find(site);
  if (it != sitesizes.end()) {
    sitesizes.erase(it);
    recalculate();
  }
}

void SizeLocationTrack::recalculate() {
  std::map<std::shared_ptr<SiteRace>, unsigned long long int>::iterator it;
  std::map<unsigned long long int, int> commonsizes;
  std::map<unsigned long long int, int>::iterator it2;
  unsigned long long int size;
  for (it = sitesizes.begin(); it != sitesizes.end(); it++) {
    size = it->second;
    if (!size) {
      continue;
    }
    it2 = commonsizes.find(size);
    if (it2 != commonsizes.end()) {
      commonsizes[size] = it2->second + 1;
    }
    else {
      commonsizes[size] = 1;
    }
  }
  unsigned long long int mostcommon = 0;
  int mostcommoncount = 0;
  unsigned long long int largest = 0;
  int largestcount = 1;
  for (it2 = commonsizes.begin(); it2 != commonsizes.end(); it2++) {
    if (it2->second > mostcommoncount) {
      mostcommon = it2->first;
      mostcommoncount = it2->second;
    }
    if (it2->first > largest) {
      largest = it2->first;
      largestcount = 1;
    }
    else if (it2->first == largest) {
      largestcount++;
    }
  }
  if (largestcount >= 2 || largestcount == mostcommoncount ||
      (mostcommon == 0 && mostcommoncount + 1 < (int)commonsizes.size())) {
    estimatedsize = largest;
  }
  else {
    estimatedsize = mostcommon;
  }
}
