#pragma once

#include <memory>
#include <string>

#include "legendprinter.h"
#include "menuselectoption.h"

class Race;
class Ui;

class LegendPrinterSpreadJob : public LegendPrinter {
public:
  LegendPrinterSpreadJob(Ui* ui, unsigned int id);
  bool print();
private:
  Ui* ui;
  std::shared_ptr<Race> race;
  MenuSelectOption mso;
  int jobfinishedprintcount;
};
