#include "numinputarrow.h"

NumInputArrow::NumInputArrow() {

}

NumInputArrow::NumInputArrow(int startval, int min, int max) {
  val = startval;
  active = false;
  this->min = min;
  this->max = max;
}

bool NumInputArrow::decrease() {
  if (val > min) {
    val--;
    return true;
  }
  return false;

}

bool NumInputArrow::increase() {
  if (val < max) {
    val++;
    return true;
  }
  return false;
}

bool NumInputArrow::setValue(int newval) {
  if (newval >= min && newval <= max) {
    val = newval;
    return true;
  }
  return false;
}

int NumInputArrow::getValue() const {
  return val;
}

void NumInputArrow::activate() {
  active = true;
}

void NumInputArrow::deactivate() {
  active = false;
}
std::string NumInputArrow::getVisual() const {
  unsigned int maxlen = std::to_string(max).length();
  for (const std::pair<const int, std::string> & substitute : substituteTexts) {
    if (substitute.second.length() > maxlen) {
      maxlen = substitute.second.length();
    }
  }
  maxlen += 4;
  std::map<int, std::string>::const_iterator it = substituteTexts.find(val);
  std::string out = it != substituteTexts.end() ? it->second : std::to_string(val);
  if (active) {
    out = "< " + out + " >";
  }
  while (out.length() < maxlen) out += ' ';
  return out;
}

void NumInputArrow::setSubstituteText(int value, const std::string & text) {
  substituteTexts[value] = text;
}
