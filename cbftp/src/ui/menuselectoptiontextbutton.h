#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "resizableelement.h"

class MenuSelectOptionTextButton : public ResizableElement {
private:
  std::string text;
  std::string legendtext;
public:
  MenuSelectOptionTextButton(std::string, int, int, std::string);
  MenuSelectOptionTextButton(std::string, int, int, std::string, bool);
  std::string getContentText() const override;
  std::string getLabelText() const override;
  bool isActivated() const;
  bool activate() override;
  unsigned int wantedWidth() const override;
  std::string getLegendText() const override;
  void setLegendText(const std::string& text);
  virtual unsigned int getTotalWidth() const override;
};
