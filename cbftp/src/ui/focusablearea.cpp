#include "focusablearea.h"

#include "menuselectoptionelement.h"

#include <cstdlib>

FocusableArea::FocusableArea() {
  focus = false;
  leaveup = false;
  leavedown = false;
  leaveleft = false;
  leaveright = false;
}

FocusableArea::~FocusableArea() {

}

bool FocusableArea::isFocused() const {
  return focus;
}

void FocusableArea::enterFocusFrom(int direction) {
  focus = true;
}

void FocusableArea::defocus() {
  focus = false;
}

bool FocusableArea::goUp() {
  return false;
}

bool FocusableArea::goDown() {
  return false;
}

bool FocusableArea::goLeft() {
  return false;
}

bool FocusableArea::goRight() {
  return false;
}

bool FocusableArea::goNext() {
  return goDown();
}

bool FocusableArea::goPrevious() {
  return goUp();
}

void FocusableArea::makeLeavableUp(bool leaveup) {
  this->leaveup = leaveup;
}

void FocusableArea::makeLeavableDown(bool leavedown) {
  this->leavedown = leavedown;
}

void FocusableArea::makeLeavableLeft(bool leaveleft) {
  this->leaveleft = leaveleft;
}

void FocusableArea::makeLeavableRight(bool leaveright) {
  this->leaveright = leaveright;
}

void FocusableArea::makeLeavableUp() {
  makeLeavableUp(true);
}

void FocusableArea::makeLeavableDown() {
  makeLeavableDown(true);
}

void FocusableArea::makeLeavableLeft() {
  makeLeavableLeft(true);
}

void FocusableArea::makeLeavableRight() {
  makeLeavableRight(true);
}
unsigned int FocusableArea::getLastSelectionPointer() const {
  return 0;
}

bool FocusableArea::activateSelected() {
  return false;
}

std::shared_ptr<MenuSelectOptionElement> FocusableArea::getElement(unsigned int i) const {
  return std::shared_ptr<MenuSelectOptionElement>();
}
