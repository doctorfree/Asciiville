#include "virtualviewelement.h"

#include "misc.h"

VirtualViewElement::VirtualViewElement() :
  c(0), newc(32), highlight(false), newhighlight(false), bold(false),
  newbold(false), cleariteration(0), redrawiteration(0), newcleariteration(0),
  newredrawiteration(0), color(encodeColorRepresentation()), newcolor(encodeColorRepresentation())
{
}

unsigned int VirtualViewElement::getChar() const {
  return newc;
}

bool VirtualViewElement::getHighlight() const {
  return newhighlight;
}

bool VirtualViewElement::getBold() const {
  return newbold;
}

int VirtualViewElement::getColor() const {
  return newcolor;
}

int VirtualViewElement::getClearIteration() const {
  return newcleariteration;
}

int VirtualViewElement::getRedrawIteration() const {
  return newredrawiteration;
}

bool VirtualViewElement::isModified() const {
  return c != newc || highlight != newhighlight || bold != newbold || redrawiteration != newredrawiteration || color != newcolor;
}

bool VirtualViewElement::isClear() const {
  return newc == 32 && newhighlight == false;
}

void VirtualViewElement::set(unsigned int c, bool highlight, bool bold, int color, int cleariteration, int redrawiteration) {
  newc = c;
  newhighlight = highlight;
  newbold = bold;
  newcolor = color;
  newcleariteration = cleariteration;
  newredrawiteration = redrawiteration;
}

void VirtualViewElement::render() {
  c = newc;
  highlight = newhighlight;
  bold = newbold;
  color = newcolor;
  cleariteration = newcleariteration;
  redrawiteration = newredrawiteration;
}

void VirtualViewElement::setHighlightOn(int cleariteration, int redrawiteration) {
  newhighlight = true;
  newcleariteration = cleariteration;
  newredrawiteration = redrawiteration;
}

void VirtualViewElement::setHighlightOff(int cleariteration, int redrawiteration) {
  newhighlight = false;
  newcleariteration = cleariteration;
  newredrawiteration = redrawiteration;
}

void VirtualViewElement::setBoldOn(int cleariteration, int redrawiteration) {
  newbold = true;
  newcleariteration = cleariteration;
  newredrawiteration = redrawiteration;
}

void VirtualViewElement::setBoldOff(int cleariteration, int redrawiteration) {
  newbold = false;
  newcleariteration = cleariteration;
  newredrawiteration = redrawiteration;
}

void VirtualViewElement::setColor(int color, int cleariteration, int redrawiteration) {
  newcolor = color;
  newcleariteration = cleariteration;
  newredrawiteration = redrawiteration;
}

void VirtualViewElement::clear() {
  newc = 32;
  newhighlight = false;
  newbold = false;
  newcolor = encodeColorRepresentation();
}

