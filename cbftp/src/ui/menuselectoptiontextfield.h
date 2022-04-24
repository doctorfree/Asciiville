#pragma once

#include <string>

#include "textinputfield.h"
#include "menuselectoptionelement.h"
#include "resizableelement.h"

class MenuSelectOptionTextField : public ResizableElement {
private:
  TextInputField textfield;
  std::string extralegend;
public:
  MenuSelectOptionTextField();
  MenuSelectOptionTextField(const std::string& identifier, int row, int col,
      const std::string& label, const std::string& starttext, int visiblelen,
      int maxlen, bool secret);
  std::string getContentText() const override;
  bool activate() override;
  bool inputChar(int ch) override;
  int cursorPosition() const override;
  std::string getData() const;
  void clear();
  void setText(const std::string& text);
  std::string getLegendText() const override;
  void setExtraLegendText(const std::string& extra);
  unsigned int wantedWidth() const override;
  void setMaxWidth(unsigned int maxwidth) override;
};
