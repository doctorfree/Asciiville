#pragma once

#include <memory>
#include <vector>

class MenuSelectOptionElement;

class MenuSelectOptionContainer {
private:
  std::vector<std::shared_ptr<MenuSelectOptionElement> > elements;
public:
  MenuSelectOptionContainer();
  ~MenuSelectOptionContainer();
  void addElement(std::shared_ptr<MenuSelectOptionElement>);
  std::shared_ptr<MenuSelectOptionElement> getOption(unsigned int) const;
};
