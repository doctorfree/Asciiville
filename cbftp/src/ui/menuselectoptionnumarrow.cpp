#include "menuselectoptionnumarrow.h"

#include "ncurseswrap.h"

MenuSelectOptionNumArrow::MenuSelectOptionNumArrow(std::string identifier, int row, int col, std::string label, int value, int min, int max) {
  arrow = NumInputArrow(value, min, max);
  init(identifier, row, col, label);
}

std::string MenuSelectOptionNumArrow::getContentText() const {
  return arrow.getVisual();
}

bool MenuSelectOptionNumArrow::inputChar(int ch) {
  switch(ch) {
    case KEY_DOWN:
    case KEY_LEFT:
      arrow.decrease();
      return true;
    case KEY_UP:
    case KEY_RIGHT:
      arrow.increase();
      return true;
  }
  return false;
}

bool MenuSelectOptionNumArrow::activate() {
  arrow.activate();
  return true;
}

void MenuSelectOptionNumArrow::deactivate() {
  arrow.deactivate();
}

int MenuSelectOptionNumArrow::getData() const {
  return arrow.getValue();
}

void MenuSelectOptionNumArrow::setData(int value) {
  arrow.setValue(value);
}

std::string MenuSelectOptionNumArrow::getLegendText() const {
  return "[Enter] Finish editing - [Left] Decrease value - [Right] Increase value";
}

void MenuSelectOptionNumArrow::setSubstituteText(int value, const std::string & text) {
  arrow.setSubstituteText(value, text);
}
