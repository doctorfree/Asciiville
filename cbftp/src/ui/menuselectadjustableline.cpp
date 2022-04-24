#include "menuselectadjustableline.h"

#include <unistd.h>

#include "resizableelement.h"

void MenuSelectAdjustableLine::addElement(std::shared_ptr<ResizableElement> re, unsigned int prio) {
  addElement(re, prio, RESIZE_REMOVE);
}

void MenuSelectAdjustableLine::addElement(std::shared_ptr<ResizableElement> re, unsigned int prio, unsigned int resizemethod) {
  addElement(re, prio, resizemethod, false);
}

void MenuSelectAdjustableLine::addElement(std::shared_ptr<ResizableElement> re, unsigned int prio, unsigned int resizemethod, bool expandable) {
  addElement(re, prio, prio, resizemethod, expandable);
}

void MenuSelectAdjustableLine::addElement(std::shared_ptr<ResizableElement> re, unsigned int highprio, unsigned int lowprio, unsigned int resizemethod, bool expandable) {
  re->setHighPriority(highprio);
  re->setLowPriority(lowprio);
  re->setResizeMethod(resizemethod);
  re->setVisible(true);
  re->setExpandable(expandable);
  elements.push_back(re);
}

std::shared_ptr<ResizableElement> MenuSelectAdjustableLine::getElement(unsigned int pos) const {
  if (!elements.size() || elements.size() - 1 < pos) {
    return std::shared_ptr<ResizableElement>();
  }
  return elements[pos];
}

std::pair<unsigned int, unsigned int> MenuSelectAdjustableLine::getMinMaxCol(bool usecontent) const {
  if (elements.empty()) {
    return std::pair<unsigned int, unsigned int>(0, 0);
  }
  unsigned int min = elements[0]->getCol();
  unsigned int maxelempos = elements.size() - 1;
  std::shared_ptr<ResizableElement> maxelem = elements[0];
  while (maxelempos > 0) {
    const std::shared_ptr<ResizableElement>& tmpmaxelem = elements[maxelempos--];
    if (tmpmaxelem->isVisible()) {
      maxelem = tmpmaxelem;
      break;
    }
  }
  unsigned int max = maxelem->getCol() + (usecontent ? maxelem->getContentText().length() : maxelem->getLabelText().length()) - 1;
  return std::pair<unsigned int, unsigned int>(min, max);
}

unsigned int MenuSelectAdjustableLine::getRow() const {
  if (elements.empty()) {
    return 0;
  }
  return elements[0]->getRow();
}

unsigned int MenuSelectAdjustableLine::size() const {
  return elements.size();
}

bool MenuSelectAdjustableLine::empty() const {
  return elements.empty();
}
