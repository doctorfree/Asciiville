#pragma once

#include <memory>

#include "../../path.h"

#include "../uiwindow.h"
#include "../menuselectoptiontextfield.h"
#include "../commandhistory.h"

class RawBuffer;
class SiteLogic;

class RawCommandScreen : public UIWindow {
public:
  RawCommandScreen(Ui * ui);
  void initialize(unsigned int, unsigned int, const std::string &, const Path &, const std::string &);
  void initialize(unsigned int, unsigned int, const std::string &);
  void initialize(unsigned int, unsigned int, RawBuffer * rawbuffer, const std::string & label, const std::string & infotext);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  int getCurrentScope() const;
  void fixCopyReadPos();
  bool readfromcopy;
  unsigned int copyreadpos;
  MenuSelectOptionTextField rawcommandfield;
  std::string label;
  std::string selection;
  Path path;
  bool hasselection;
  std::shared_ptr<SiteLogic> sitelogic;
  RawBuffer * rawbuf;
  CommandHistory history;
  bool allowinput;
};
