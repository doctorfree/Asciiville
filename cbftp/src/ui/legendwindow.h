#pragma once

#include <list>
#include <memory>
#include <string>

#include "uiwindow.h"

class LegendPrinter;

class LegendWindow : public UIWindow {
public:
  LegendWindow(Ui* ui, unsigned int row, unsigned int col);
  void redraw();
  void update();
  void setSplit(bool split);
  void setMainLegendPrinter(std::shared_ptr<LegendPrinter> printer);
  void addTempLegendPrinter(std::shared_ptr<LegendPrinter> printer);
  void clearTempLegendPrinters();
private:
  int latestid;
  int latestcount;
  int staticcount;
  std::string latesttext;
  bool split;
  std::shared_ptr<LegendPrinter> mainlegendprinter;
  std::list<std::shared_ptr<LegendPrinter> > templegendprinters;
};
