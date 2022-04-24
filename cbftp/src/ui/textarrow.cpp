#include "textarrow.h"

TextArrow::TextArrow() {
  active = false;
  currentpos = 0;
  maxlen = 0;
}

bool TextArrow::previous() {
  if (currentpos > 0) {
    currentpos--;
    return true;
  }
  return false;

}

bool TextArrow::next() {
  if (currentpos < options.size() - 1) {
    currentpos++;
    return true;
  }
  return false;
}

bool TextArrow::setOption(int newval) {
  for (unsigned int i = 0; i < options.size(); i++) {
    if (options[i].first == newval) {
      currentpos = i;
      return true;
    }
  }
  return false;
}

bool TextArrow::setOptionText(std::string newtext) {
  for (unsigned int i = 0; i < options.size(); i++) {
    if (options[i].second == newtext) {
      currentpos = i;
      return true;
    }
  }
  return false;
}

int TextArrow::getOption() const {
  if (options.size() == 0) {
    return -1;
  }
  return options[currentpos].first;
}

std::string TextArrow::getOptionText() const {
  if (options.size() == 0) {
    return "";
  }
  return options[currentpos].second;
}

void TextArrow::activate() {
  active = true;
}

void TextArrow::deactivate() {
  active = false;
}

bool TextArrow::isActive() const {
  return active;
}

std::string TextArrow::getVisual() const {
  std::string out = getOptionText();
  if (active) {
    out = "< " + out + " >";
  }
  while (out.length() < maxlen + 4) out += ' ';
  return out;
}

void TextArrow::addOption(std::string text, int id) {
  options.push_back(std::pair<int, std::string>(id, text));
  if (text.length() > maxlen) {
    maxlen = text.length();
  }
}

void TextArrow::clear() {
  options.clear();
  maxlen = 0;
  currentpos = 0;
}
