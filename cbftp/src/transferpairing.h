#pragma once

#include <list>
#include <string>

#include "transferpairingitem.h"

class TransferPairing {
public:
  std::list<TransferPairingItem>::const_iterator begin() const;
  std::list<TransferPairingItem>::const_iterator end() const;

  std::list<PairingAction> check(int slots, PairingJobType type,
      const std::string& targetsite, const std::string& jobname,
      const std::string& section) const;
  void add(bool allsites, const std::list<std::string>& targetsites,
      PairingJobType type, const std::string& jobnamepattern, bool allsections,
      const std::list<std::string>& sections, PairingAction action, int slots);
  void clear();
private:
  std::list<TransferPairingItem> entries;
};
