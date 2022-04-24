#pragma once

#include "../menuselectoptiontextfield.h"
#include "../uiwindow.h"

class RawBuffer;

class EventLogScreen : public UIWindow {
public:
  EventLogScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  void fixCopyReadPos();
  bool readfromcopy;
  unsigned int copyreadpos;
  RawBuffer * rawbuf;
  std::string filtertext;
  bool filtermodeinput;
  bool filtermodeinputregex;
  MenuSelectOptionTextField filterfield;
};
