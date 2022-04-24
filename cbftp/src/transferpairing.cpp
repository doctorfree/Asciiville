#include "transferpairing.h"

std::list<TransferPairingItem>::const_iterator TransferPairing::begin() const {
  return entries.begin();
}

std::list<TransferPairingItem>::const_iterator TransferPairing::end() const {
  return entries.end();
}

std::list<PairingAction> TransferPairing::check(int slots, PairingJobType type,
      const std::string& targetsite, const std::string& jobname,
      const std::string& section) const
{
  std::list<PairingAction> usedslots;
  return usedslots;
}

void TransferPairing::add(bool allsites, const std::list<std::string>& targetsites,
      PairingJobType type, const std::string& jobnamepattern, bool allsections,
      const std::list<std::string>& sections, PairingAction action, int slots)
{
  entries.emplace_back(allsites, targetsites, type, jobnamepattern,
                       allsections, sections, action, slots);
}

void TransferPairing::clear() {
  entries.clear();
}
