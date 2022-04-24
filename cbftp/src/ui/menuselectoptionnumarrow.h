#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "numinputarrow.h"

class MenuSelectOptionNumArrow : public MenuSelectOptionElement {
private:
  NumInputArrow arrow;
public:
  MenuSelectOptionNumArrow(std::string, int, int, std::string, int, int, int);
  std::string getContentText() const override;
  bool inputChar(int ch) override;
  bool activate() override;
  void deactivate() override;
  int getData() const;
  void setData(int value);
  std::string getLegendText() const override;
  void setSubstituteText(int value, const std::string & text);
};
