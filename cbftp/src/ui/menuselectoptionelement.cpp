#include "menuselectoptionelement.h"

void MenuSelectOptionElement::init(const std::string& identifier, int row, int col, const std::string& label) {
  this->identifier = identifier;
  this->row = row;
  this->col = col;
  this->label = label;
  active = false;
  shown = true;
  selectable = true;
  id = 0;
  origin = nullptr;
}

MenuSelectOptionElement::~MenuSelectOptionElement() {

}

void MenuSelectOptionElement::setPosition(int row, int col) {
  this->row = row;
  this->col = col;
}

std::string MenuSelectOptionElement::getLabelText() const {
  return label;
}

std::string MenuSelectOptionElement::getIdentifier() const {
  return identifier;
}

unsigned int MenuSelectOptionElement::getId() const {
  return id;
}

std::string MenuSelectOptionElement::getExtraData() const {
  return extradata;
}

unsigned int MenuSelectOptionElement::getTotalWidth() const {
  return getLabelText().length() + 1 + getContentText().length();
}

bool MenuSelectOptionElement::activate() {
  active = true;
  return true;
}

void MenuSelectOptionElement::deactivate() {
  active = false;
}

bool MenuSelectOptionElement::isActive() const {
  return active;
}

std::string MenuSelectOptionElement::getLegendText() const {
  return "";
}

unsigned int MenuSelectOptionElement::getCol() const {
  return col;
}

unsigned int MenuSelectOptionElement::getRow() const {
  return row;
}

int MenuSelectOptionElement::cursorPosition() const {
  return -1;
}

bool MenuSelectOptionElement::inputChar(int ch) {
  return false;
}

void MenuSelectOptionElement::hide() {
  shown = false;
}

void MenuSelectOptionElement::show() {
  shown = true;
}

bool MenuSelectOptionElement::visible() const {
  return shown;
}

bool MenuSelectOptionElement::isSelectable() const {
  return selectable;
}

void MenuSelectOptionElement::setSelectable(bool selectable) {
  this->selectable = selectable;
}

void MenuSelectOptionElement::setId(unsigned int id) {
  this->id = id;
}

void MenuSelectOptionElement::setExtraData(const std::string& data) {
  extradata = data;
}

void MenuSelectOptionElement::setLabel(const std::string& label) {
  this->label = label;
}

void * MenuSelectOptionElement::getOrigin() const {
  return origin;
}

void MenuSelectOptionElement::setOrigin(void* origin) {
  this->origin = origin;
}
