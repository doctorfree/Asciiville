#include "transferpairingitem.h"

TransferPairingItem::TransferPairingItem(bool allsites,
    const std::list<std::string>& targetsites, PairingJobType type,
    const std::string& jobnamepattern, bool allsections,
    const std::list<std::string>& sections, PairingAction action, int slots) :
    allsites(allsites), targetsites(targetsites), type(type),
    jobnamepattern(jobnamepattern), allsections(allsections), sections(sections),
    action(action), slots(slots)
{

}

bool TransferPairingItem::getAllSites() const {
  return allsites;
}

PairingJobType TransferPairingItem::getJobType() const {
  return type;
}

std::string TransferPairingItem::getJobNamePattern() const {
  return jobnamepattern;
}

bool TransferPairingItem::getAllSections() const {
  return allsections;
}

PairingAction TransferPairingItem::getPairingAction() const {
  return action;
}

int TransferPairingItem::getSlots() const {
  return slots;
}
