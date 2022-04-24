#include "potentialtracker.h"

#include "core/tickpoke.h"
#include "globalcontext.h"
#include "potentialelement.h"
#include "potentiallistelement.h"

PotentialTracker::PotentialTracker(int dnslots) : maxpotentialcalculated(true) {
  for (int i = 0; i < POTENTIALITY_SLICES; i++) {
    potentiallist.push_back(new PotentialListElement(dnslots));
  }
  for (int i = 0; i < dnslots; i++) {
    top.push_back(new PotentialElement());
  }
  global->getTickPoke()->startPoke(this, "PotentialTracker", POTENTIALITY_LIFESPAN / POTENTIALITY_SLICES, 0);
}

PotentialTracker::~PotentialTracker() {
  global->getTickPoke()->stopPoke(this, 0);
  for (std::list<PotentialListElement *>::iterator it = potentiallist.begin(); it != potentiallist.end(); it++) {
    delete *it;
  }
  for (std::list<PotentialElement *>::iterator it = top.begin(); it != top.end(); it++) {
    delete *it;
  }
}

int PotentialTracker::getMaxAvailablePotential() {
  if (top.empty()) {
    return 0;
  }
  if (!maxpotentialcalculated) {
    calculateMaxAvailablePotential();
  }
  return top.front()->getPotential();
}

void PotentialTracker::calculateMaxAvailablePotential() {
  std::list<PotentialElement *>::iterator ittop;
  std::list<PotentialElement *>::iterator ittop2;
  std::list<PotentialListElement *>::iterator itple;
  std::vector<PotentialElement *>::iterator itpe;
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    (*ittop)->reset();
  }
  for (itple = potentiallist.begin(); itple != potentiallist.end(); itple++) {
    std::vector<PotentialElement *> & pelist = (*itple)->getSlotsVector();
    for (itpe = pelist.begin(); itpe != pelist.end(); itpe++) {
      PotentialElement * mpe;
      std::list<PotentialElement *>::iterator itrpe = top.begin();
      if (!!(*itpe)->getSite() && allTopSlotsUsedForSite(*itpe)) {
        itrpe = findFirstOfSite((*itpe)->getSite());
      }
      mpe = *itrpe;
      bool inserted = false;
      bool duplicate = false;
      bool lowscore = false;
      for (ittop = top.begin(); ittop != top.end(); ittop++) {
        if ((*itpe)->getPotential() < (*ittop)->getPotential()) {
          if (ittop == top.begin()) {
            lowscore = true;
            break;
          }
          for (ittop2 = top.begin(); ittop2 != top.end(); ittop2++) {
            if ((*itpe)->getSite() == (*ittop2)->getSite() && (*itpe)->getFileName().compare((*ittop2)->getFileName()) == 0) {
              duplicate = true;
              break;
            }
          }
          if (duplicate) break;
          top.insert(ittop, mpe);
          top.erase(itrpe);
          mpe->update((*itpe)->getSite(), (*itpe)->getDestinationSiteUploadSlots(), (*itpe)->getPotential(), (*itpe)->getFileName());
          inserted = true;
          break;
        }
      }
      if (!lowscore && !duplicate && !inserted && (*itpe)->getPotential() >= top.back()->getPotential()) {
        top.erase(itrpe);
        top.push_back(mpe);
        mpe->update((*itpe)->getSite(), (*itpe)->getDestinationSiteUploadSlots(), (*itpe)->getPotential(), (*itpe)->getFileName());
      }
    }
  }
  maxpotentialcalculated = true;
}

void PotentialTracker::pushPotential(int score, const std::string & file, const std::shared_ptr<SiteLogic> & dst, int dstup) {
  if (potentiallist.front()->update(dst, dstup, score, file)) {
    maxpotentialcalculated = false;
  }
}

void PotentialTracker::tick(int message) {
  potentiallist.back()->reset();
  potentiallist.push_front(potentiallist.back());
  potentiallist.pop_back();
  maxpotentialcalculated = false;
}

std::list<PotentialElement *>::iterator PotentialTracker::findFirstOfSite(const std::shared_ptr<SiteLogic> & st) {
  std::list<PotentialElement *>::iterator ittop;
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    if ((*ittop)->getSite() == st) return ittop;
  }
  return top.end();
}

void PotentialTracker::updateSlots(int dnslots) {
  for (std::list<PotentialListElement *>::iterator it = potentiallist.begin(); it != potentiallist.end(); it++) {
    (*it)->updateSlots(dnslots);
  }
  while ((int)top.size() > dnslots) {
    delete top.back();
    top.pop_back();
  }
  while ((int)top.size() < dnslots) {
    top.push_back(new PotentialElement());
  }
  maxpotentialcalculated = false;
}

bool PotentialTracker::allTopSlotsUsedForSite(PotentialElement * pe) const {
  int dstupslots = pe->getDestinationSiteUploadSlots();
  int sitematch = 0;
  std::list<PotentialElement *>::const_iterator ittop;
  for (ittop = top.begin(); ittop != top.end(); ittop++) {
    if((*ittop)->getSite() == pe->getSite()) {
      sitematch++;
    }
  }
  if (sitematch == dstupslots) return true;
  return false;
}
