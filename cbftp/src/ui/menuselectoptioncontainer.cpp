#include "menuselectoptioncontainer.h"

#include <cstdlib>

#include "menuselectoptionelement.h"

MenuSelectOptionContainer::MenuSelectOptionContainer() {

}

MenuSelectOptionContainer::~MenuSelectOptionContainer() {

}

void MenuSelectOptionContainer::addElement(std::shared_ptr<MenuSelectOptionElement> msoe) {
  elements.push_back(msoe);
}

std::shared_ptr<MenuSelectOptionElement> MenuSelectOptionContainer::getOption(unsigned int id) const {
  if (elements.size() > 0 && elements.size() - 1 >= id) {
    return elements[id];
  }
  return std::shared_ptr<MenuSelectOptionElement>();
}
