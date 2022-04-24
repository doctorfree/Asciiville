#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "textarrow.h"
#include "resizableelement.h"

class MenuSelectOptionTextArrow : public ResizableElement {
private:
  TextArrow arrow;
public:
  MenuSelectOptionTextArrow(std::string, int, int, std::string);
  std::string getContentText() const override;
  bool inputChar(int ch) override;
  bool activate() override;
  void deactivate() override;
  bool isActive() const override;
  int getData() const;
  std::string getDataText() const;
  std::string getLegendText() const override;
  void addOption(std::string, int);
  bool setOption(int);
  bool setOptionText(std::string);
  void clear();
  unsigned int wantedWidth() const override;
};
