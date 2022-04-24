#pragma once

#include <memory>
#include <string>
#include <vector>

class SiteLogic;
class PotentialElement;

class PotentialListElement {
  private:
    std::vector<PotentialElement *> slots;
    bool allSlotsUsedForSite(const std::shared_ptr<SiteLogic> &, int) const;
  public:
    PotentialListElement(int);
    ~PotentialListElement();
    bool update(const std::shared_ptr<SiteLogic> &, int, int, const std::string &);
    void reset();
    std::vector<PotentialElement *> & getSlotsVector();
    void updateSlots(int);

};
