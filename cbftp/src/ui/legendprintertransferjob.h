#pragma once

#include <memory>
#include <string>

#include "legendprinter.h"
#include "menuselectoption.h"

class TransferJob;
class Ui;

class LegendPrinterTransferJob : public LegendPrinter {
public:
  LegendPrinterTransferJob(Ui* ui, unsigned int id);
  bool print();
private:
  Ui* ui;
  std::shared_ptr<TransferJob> transferjob;
  MenuSelectOption mso;
  int jobfinishedprintcount;
};
