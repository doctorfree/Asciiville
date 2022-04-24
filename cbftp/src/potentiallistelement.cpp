#include "potentiallistelement.h"

#include "potentialelement.h"

PotentialListElement::PotentialListElement(int dnslots) {
  for (int i = 0; i < dnslots; i++) {
    slots.push_back(new PotentialElement());
  }
}

PotentialListElement::~PotentialListElement() {
  for (unsigned int i = 0; i < slots.size(); i++) {
    delete slots[i];
  }
}

bool PotentialListElement::update(const std::shared_ptr<SiteLogic> & dst, int dstup, int potential, const std::string & filename) {
  bool allslotsused = allSlotsUsedForSite(dst, dstup);
  PotentialElement * lowelem = NULL;
  int lowest = 0;
  bool initialized = false;
  for (unsigned int i = 0; i < slots.size(); i++) {
    if (allslotsused && slots[i]->getSite() != dst) continue;
    if (!initialized || slots[i]->getPotential() < lowest) {
      lowest = slots[i]->getPotential();
      lowelem = slots[i];
      if (!initialized) initialized = true;
    }
  }
  if (lowelem && lowest < potential) {
    lowelem->update(dst, dstup, potential, filename);
    return true;
  }
  return false;
}

std::vector<PotentialElement *> & PotentialListElement::getSlotsVector() {
  return slots;
}

void PotentialListElement::reset() {
  for (unsigned int i = 0; i < slots.size(); i++) {
    slots[i]->reset();
  }
}

void PotentialListElement::updateSlots(int dnslots) {
  while (dnslots < (int)slots.size()) {
    delete slots.back();
    slots.pop_back();
  }
  while (dnslots > (int)slots.size()) {
    slots.push_back(new PotentialElement());
  }
}

bool PotentialListElement::allSlotsUsedForSite(const std::shared_ptr<SiteLogic> & dst, int dstup) const {
  int sitematch = 0;
  for (unsigned int i = 0; i < slots.size(); i++) {
    if(slots[i]->getSite() == dst) {
      sitematch++;
    }
  }
  if (sitematch == dstup) return true;
  return false;
}
