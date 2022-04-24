#pragma once

#include <string>

#include "legendprinter.h"

class Ui;

class LegendPrinterKeybinds : public LegendPrinter {
public:
  LegendPrinterKeybinds(Ui* ui);
  bool print();
  void setText(const std::string& text);
private:
  Ui* ui;
  std::string text;
  unsigned int offset;
  int staticcount;
};
