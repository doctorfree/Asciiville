#pragma once

#include <string>

#include "uiwindow.h"

class InfoWindow : public UIWindow {
public:
  InfoWindow(Ui* ui, unsigned int row, unsigned int col);
  void redraw();
  void update();
  void setLabel(const std::basic_string<unsigned int>& label);
  void setText(const std::basic_string<unsigned int>& text);
  void setSplit(bool split);
private:
  std::basic_string<unsigned int> label;
  std::basic_string<unsigned int> text;
  bool split;
};
