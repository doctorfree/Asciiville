#pragma once

#include <memory>
#include <string>

class SiteLogic;

class PotentialElement {
  private:
    std::shared_ptr<SiteLogic> site;
    int potential;
    int dstupslots;
    std::string filename;
  public:
    PotentialElement();
    const std::shared_ptr<SiteLogic> & getSite() const;
    int getDestinationSiteUploadSlots() const;
    int getPotential() const;
    std::string getFileName() const;
    void reset();
    void update(const std::shared_ptr<SiteLogic> &, int, int, const std::string &);
};
