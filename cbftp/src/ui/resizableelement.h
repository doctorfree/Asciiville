#pragma once

#include "menuselectoptionelement.h"

#define RESIZE_REMOVE 1191
#define RESIZE_WITHDOTS 1192
#define RESIZE_WITHLAST3 1193
#define RESIZE_CUTEND 1194

class ResizableElement : public MenuSelectOptionElement {
public:
  ResizableElement() :
    rightaligned(false),
    shortspacing(false),
    visible(false),
    expandable(false) {
  }
  virtual ~ResizableElement() {
  }
  virtual unsigned int wantedWidth() const = 0;
  unsigned int highPriority() const {
    return highprio;
  }
  unsigned int lowPriority() const {
    return lowprio;
  }
  unsigned int resizeMethod() const {
    return resizemethod;
  }
  unsigned int getMaxWidth() const {
    return maxwidth;
  }
  virtual void setMaxWidth(unsigned int maxwidth) {
    this->maxwidth = maxwidth;
  }
  void setHighPriority(unsigned int prio) {
    this->highprio = prio;
  }
  void setLowPriority(unsigned int prio) {
    this->lowprio = prio;
  }
  void setResizeMethod(unsigned int resizemethod) {
    this->resizemethod = resizemethod;
  }
  bool isVisible() const {
    return visible;
  }
  void setVisible(bool visible) {
    this->visible = visible;
  }
  bool isExpandable() const {
    return expandable;
  }
  void setExpandable(bool expandable) {
    this->expandable = expandable;
  }
  void setShortSpacing() {
    shortspacing = true;
  }
  bool shortSpacing() const {
    return shortspacing;
  }
  void setRightAligned() {
    rightaligned = true;
  }
  bool rightAligned() const {
    return rightaligned;
  }
protected:
  unsigned int maxwidth;
  unsigned int resizemethod;
  bool rightaligned;
private:
  bool shortspacing;
  unsigned int highprio;
  unsigned int lowprio;
  bool visible;
  bool expandable;
};
