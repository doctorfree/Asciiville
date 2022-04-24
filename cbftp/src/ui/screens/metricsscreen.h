#pragma once

#include <memory>

#include "../uiwindow.h"

class BrailleGraph;

class MetricsScreen : public UIWindow {
public:
  MetricsScreen(Ui* ui);
  ~MetricsScreen();
  void initialize(unsigned int row, unsigned int col);
  void redraw();
  void update();
  bool keyPressed(unsigned int ch);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  std::unique_ptr<BrailleGraph> cpuall;
  std::unique_ptr<BrailleGraph> cpuworker;
  std::unique_ptr<BrailleGraph> workqueuesize;
  std::unique_ptr<BrailleGraph> perflevel;
  std::unique_ptr<BrailleGraph> filelistrefreshrate;
};
