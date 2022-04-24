#include "menuselectoptiontextbutton.h"

MenuSelectOptionTextButton::MenuSelectOptionTextButton(std::string identifier, int row, int col, std::string text) {
  this->text = text;
  init(identifier, row, col, text);
}
MenuSelectOptionTextButton::MenuSelectOptionTextButton(std::string identifier, int row, int col, std::string text, bool content) {
  if (content) {
    this->text = text;
  }
  else {
    this->text = "";
  }
  init(identifier, row, col, text);
}

std::string MenuSelectOptionTextButton::getContentText() const {
  return text;
}

std::string MenuSelectOptionTextButton::getLabelText() const {
  if (label.length() <= maxwidth) {
    std::string alignextra;
    if (rightaligned) {
      while (maxwidth > label.length() + alignextra.length()) {
        alignextra += " ";
      }
    }
    return alignextra + label;
  }
  else {
    switch (resizemethod) {
      case RESIZE_CUTEND:
        return label.substr(0, maxwidth);
      case RESIZE_WITHDOTS:
      {
        std::string dots = "...";
        if (maxwidth > 3) {
          return label.substr(0, maxwidth - dots.length()) + dots;
        }
        else {
          return dots.substr(0, maxwidth);
        }
      }
      case RESIZE_WITHLAST3:
        if (maxwidth > 3) {
          return label.substr(0, maxwidth - 4) + "~" + label.substr(label.length() - 3);
        }
        else {
          return label.substr(label.length() - maxwidth);
        }
    }
    return label;
  }
}

bool MenuSelectOptionTextButton::isActivated() const {
  return active;
}

bool MenuSelectOptionTextButton::activate() {
  active = true;
  return false;
}

unsigned int MenuSelectOptionTextButton::wantedWidth() const {
  return label.length();
}

std::string MenuSelectOptionTextButton::getLegendText() const {
  return legendtext;
}

void MenuSelectOptionTextButton::setLegendText(const std::string& text) {
  legendtext = text;
}

unsigned int MenuSelectOptionTextButton::getTotalWidth() const {
  return getLabelText().length() + (text.empty() ? 0 : 1 + getContentText().length());
}
