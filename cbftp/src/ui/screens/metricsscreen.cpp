#include "metricsscreen.h"

#include "../../globalcontext.h"
#include "../../loadmonitor.h"
#include "../../util.h"

#include "../ui.h"
#include "../braillegraph.h"

namespace {

void drawAllIn(VirtualView* vv, const std::list<BrailleGraph::GraphChar>& list, unsigned int startrow) {
  for (const BrailleGraph::GraphChar& ch : list) {
    vv->putChar(ch.row + startrow, ch.col, ch.ch);
  }
}

void drawGraph(VirtualView* vv, const std::unique_ptr<BrailleGraph>& graph, unsigned int startrow = 0) {
  drawAllIn(vv, graph->getStaticBorderChars(), startrow);
  drawAllIn(vv, graph->getChangingBorderChars(), startrow);
  drawAllIn(vv, graph->getGraphChars(), startrow);
}

}

MetricsScreen::MetricsScreen(Ui* ui) : UIWindow(ui, "MetricsScreen") {
  allowimplicitgokeybinds = false;
  autoupdate = true;
}

MetricsScreen::~MetricsScreen() {

}

void MetricsScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  unsigned int graphheight = row / 5;
  cpuall = std::unique_ptr<BrailleGraph>(new BrailleGraph(graphheight, col, "CPU load total", "%", 0, 100));
  cpuworker = std::unique_ptr<BrailleGraph>(new BrailleGraph(graphheight, col, "CPU load worker", "%", 0, 100));
  workqueuesize = std::unique_ptr<BrailleGraph>(new BrailleGraph(graphheight, col, "Worker queue size", "", 0, 100, true));
  perflevel = std::unique_ptr<BrailleGraph>(new BrailleGraph(graphheight, col, "Performance level", "", 1, 9));
  filelistrefreshrate = std::unique_ptr<BrailleGraph>(new BrailleGraph(graphheight, col, "Total file list refresh rate", "/s", 0, 100, true));
  cpuall->setData(global->getLoadMonitor()->getCpuUsageAllHistory());
  cpuworker->setData(global->getLoadMonitor()->getCpuUsageWorkerHistory());
  workqueuesize->setData(global->getLoadMonitor()->getWorkQueueSizeHistory());
  perflevel->setData(global->getLoadMonitor()->getPerformanceLevelHistory());
  filelistrefreshrate->setData(global->getLoadMonitor()->getFileListRefreshRateHistory());
  init(row, col);
}

void MetricsScreen::redraw() {
  unsigned int graphheight = row / 5;
  cpuall->resize(graphheight, col, true);
  cpuworker->resize(graphheight, col, true);
  workqueuesize->resize(graphheight, col, true);
  perflevel->resize(graphheight, col, true);
  filelistrefreshrate->resize(graphheight, col, true);
  update();
}

void MetricsScreen::update() {
  vv->clear();
  ui->hideCursor();
  cpuall->addNewData(global->getLoadMonitor()->getUnseenCpuUsageAllHistory());
  cpuworker->addNewData(global->getLoadMonitor()->getUnseenCpuUsageWorkerHistory());
  workqueuesize->addNewData(global->getLoadMonitor()->getUnseenWorkQueueSizeHistory());
  perflevel->addNewData(global->getLoadMonitor()->getUnseenPerformanceLevelHistory());
  filelistrefreshrate->addNewData(global->getLoadMonitor()->getUnseenFileListRefreshRateHistory());
  drawGraph(vv, cpuall);
  drawGraph(vv, cpuworker, cpuall->rows());
  drawGraph(vv, workqueuesize, cpuall->rows() + cpuworker->rows());
  drawGraph(vv, perflevel, cpuall->rows() + cpuworker->rows() + workqueuesize->rows());
  drawGraph(vv, filelistrefreshrate, cpuall->rows() + cpuworker->rows() + workqueuesize->rows() + perflevel->rows());
}

bool MetricsScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case ' ':
    case 'c':
    case 10:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string MetricsScreen::getLegendText() const {
  return "[Esc/Space/Enter/c] Return";
}

std::string MetricsScreen::getInfoLabel() const {
  return "METRICS";
}

std::string MetricsScreen::getInfoText() const {
  return "History for the latest " + std::to_string(global->getLoadMonitor()->getHistoryLengthSeconds()) + " seconds";
}
