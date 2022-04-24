#include "legendwindow.h"

#include "termint.h"
#include "ui.h"
#include "legendprinterkeybinds.h"
#include "../eventlog.h"
#include "../globalcontext.h"

LegendWindow::LegendWindow(Ui* ui, unsigned int row, unsigned int col) : UIWindow(ui, "LegendWindow") {
  split = false;
  latestid = 0;
  latestcount = 8;
  init(row, col);
}

void LegendWindow::redraw() {
  ui->getRenderer().erase(Window::LEGEND);
  if (!!mainlegendprinter) {
    mainlegendprinter->setColumns(col);
  }
  std::list<std::shared_ptr<LegendPrinter> >::iterator it;
  for (it = templegendprinters.begin(); it != templegendprinters.end(); it++) {
    if (!!(*it)) {
      (*it)->setColumns(col);
    }
  }
  latestcount = 8;
  staticcount = 0;
  ui->getRenderer().printChar(0, 1, BOX_CORNER_TL, false, false, -1, Window::LEGEND);
  ui->getRenderer().printChar(1, 0, BOX_HLINE, false, false, -1, Window::LEGEND);
  ui->getRenderer().printChar(1, 1, BOX_CORNER_BR, false, false, -1, Window::LEGEND);
  ui->getRenderer().printChar(1, col - 1, BOX_HLINE, false, false, -1, Window::LEGEND);
  ui->getRenderer().printChar(1, col - 2, BOX_CORNER_BL, false, false, -1, Window::LEGEND);
  ui->getRenderer().printChar(0, col - 2, BOX_CORNER_TR, false, false, -1, Window::LEGEND);
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->getRenderer().printChar(0, i, BOX_HLINE, false, false, -1, Window::LEGEND);
  }
  if (split) {
    ui->getRenderer().printChar(0, col / 2, BOX_HLINE_TOP, false, false, -1, Window::LEGEND);
  }
  update();
}

void LegendWindow::update() {
  if (global->getEventLog()->getLatestId() != latestid) {
    latestid = global->getEventLog()->getLatestId();
    if (!templegendprinters.empty()) {
      return;
    }
    latestcount = 0;
    latesttext = global->getEventLog()->getLatest();
    for (unsigned int printpos = 4; printpos < col - 4; printpos++) {
      ui->getRenderer().printChar(1, printpos, ' ', false, false, -1, Window::LEGEND);
    }
    ui->getRenderer().printStr(1, 4, "EVENT: " + latesttext, false, false, -1, col - 4 - 4, false, Window::LEGEND);
    return;
  }
  if (latestcount < 8) { // 2 seconds
    latestcount++;
    return;
  }
  if (!templegendprinters.empty() && !!templegendprinters.front()) {
    if (!templegendprinters.front()->print()) {
      templegendprinters.pop_front();
    }
  }
  else if (!!mainlegendprinter) {
    mainlegendprinter->print();
  }
}

void LegendWindow::setSplit(bool split) {
  this->split = split;
}

void LegendWindow::setMainLegendPrinter(std::shared_ptr<LegendPrinter> printer) {
  mainlegendprinter = printer;
}

void LegendWindow::addTempLegendPrinter(std::shared_ptr<LegendPrinter> printer) {
  templegendprinters.push_back(printer);
}

void LegendWindow::clearTempLegendPrinters() {
  templegendprinters.clear();
}
