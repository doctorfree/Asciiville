#pragma once

#include <list>
#include <string>

enum class PairingJobType {
  RACE,
  DISTRIBUTE,
  DLONLY,
  ALL
};

enum class PairingAction {
  ALLOW,
  PREFER,
  SOFTLOCK,
  HARDLOCK,
  BLOCK
};


class TransferPairingItem {
public:
  TransferPairingItem(bool allsites, const std::list<std::string>& targetsites,
      PairingJobType type, const std::string& jobnamepattern, bool allsections,
      const std::list<std::string>& sections, PairingAction action, int slots);
  bool getAllSites() const;
  PairingJobType getJobType() const;
  std::string getJobNamePattern() const;
  bool getAllSections() const;
  PairingAction getPairingAction() const;
  int getSlots() const;
private:
  bool allsites;
  std::list<std::string> targetsites;
  PairingJobType type;
  std::string jobnamepattern;
  bool allsections;
  std::list<std::string> sections;
  PairingAction action;
  int slots;
};
