#pragma once

#include <memory>
#include <utility>
#include <vector>

#define RESIZE_SPACING 2
#define RESIZE_SPACING_SHORT 1

class ResizableElement;

class MenuSelectAdjustableLine {
public:
  void addElement(std::shared_ptr<ResizableElement>, unsigned int);
  void addElement(std::shared_ptr<ResizableElement>, unsigned int, unsigned int);
  void addElement(std::shared_ptr<ResizableElement>, unsigned int, unsigned int, bool);
  void addElement(std::shared_ptr<ResizableElement>, unsigned int, unsigned int, unsigned int, bool);
  std::shared_ptr<ResizableElement> getElement(unsigned int) const;
  std::pair<unsigned int, unsigned int> getMinMaxCol(bool usecontent = false) const;
  unsigned int getRow() const;
  unsigned int size() const;
  bool empty() const;
private:
  std::vector<std::shared_ptr<ResizableElement> > elements;
};
