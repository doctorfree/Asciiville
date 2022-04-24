#include "menuselectoptioncheckbox.h"

MenuSelectOptionCheckBox::MenuSelectOptionCheckBox(std::string identifier, int row, int col, std::string label, bool startval) {
  this->value = startval;
  init(identifier, row, col, label);
}

std::string MenuSelectOptionCheckBox::getContentText() const {
  if (value) {
    return "[X]";
  }
  return "[ ]";
}

bool MenuSelectOptionCheckBox::activate() {
  if (value) {
    value = false;
  }
  else {
    value = true;
  }
  return false;
}

void MenuSelectOptionCheckBox::setValue(bool value) {
  this->value = value;
}

bool MenuSelectOptionCheckBox::getData() const {
  return value;
}

unsigned int MenuSelectOptionCheckBox::wantedWidth() const {
  return 3;
}
