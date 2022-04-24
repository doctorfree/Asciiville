#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoptiontextfield.h"
#include "../commandhistory.h"

class SiteLogic;
class RawBuffer;

class RawDataScreen : public UIWindow {
public:
  RawDataScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string, int);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
  static void printRawBufferLines(VirtualView* vv, RawBuffer *, unsigned int, unsigned int, unsigned int);
  static void printRawBufferLines(VirtualView* vv, RawBuffer *, unsigned int, unsigned int, unsigned int, bool, unsigned int);
private:
  static bool skipCodePrint(const std::string &);
  void fixCopyReadPos();
  bool rawcommandmode;
  bool rawcommandswitch;
  int threads;
  bool readfromcopy;
  unsigned int copyreadpos;
  MenuSelectOptionTextField rawcommandfield;
  std::string sitename;
  int connid;
  std::shared_ptr<SiteLogic> sitelogic;
  RawBuffer * rawbuf;
  CommandHistory history;
  std::string filtertext;
  bool filtermodeinput;
  bool filtermodeinputregex;
  MenuSelectOptionTextField filterfield;
};
