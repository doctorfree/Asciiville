#pragma once

#include <map>
#include <string>

class NumInputArrow {
public:
  NumInputArrow();
  NumInputArrow(int, int, int);
  int getValue() const;
  bool setValue(int);
  bool increase();
  bool decrease();
  std::string getVisual() const;
  void activate();
  void deactivate();
  void setSubstituteText(int value, const std::string & text);
private:
  int val;
  int min;
  int max;
  bool active;
  std::map<int, std::string> substituteTexts;
};
