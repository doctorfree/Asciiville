#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class FocusableArea;
class Ui;
class PreparedRace;
class SiteLogic;
class Site;
class Race;
class TransferJob;

class MainScreen : public UIWindow {
public:
  MainScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void command(const std::string &);
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
  std::string getInfoText() const;
private:
  void printTable(MenuSelectOption & table);
  void addPreparedRaceTableRow(unsigned int, MenuSelectOption &, unsigned int,
      bool, const std::string &, const std::string &, const std::string &, const std::string &);
  void addPreparedRaceTableHeader(unsigned int, MenuSelectOption &);
  void addPreparedRaceDetails(unsigned int, MenuSelectOption &, const std::shared_ptr<PreparedRace> &);
  void addSiteHeader(unsigned int y, MenuSelectOption & mso);
  void addSiteRow(unsigned int y, MenuSelectOption & mso, bool selectable,
      const std::string & site, const std::string & logins, const std::string & uploads,
      const std::string & downloads, const std::string & allowup,
      const std::string & allowdown, const std::string & disabled, const std::string & dayup,
      const std::string & daydn, const std::string & alup, const std::string & aldn,
      const std::string & prio);
  void addSiteDetails(unsigned int y, MenuSelectOption & mso, const std::shared_ptr<SiteLogic> & sl);
  bool keyUp() override;
  bool keyDown() override;
  void jumpSectionHotkey(int hotkey);
  int getCurrentScope() const;
  unsigned int currentviewspan;
  unsigned int sitestartrow;
  unsigned int sitepos;
  unsigned int totalsitessize;
  int currentraces;
  int currenttransferjobs;
  bool awaitingremovesitefromallspreadjobs;
  std::string removesite;
  std::string activeracestext;
  std::string activejobstext;
  std::string numsitestext;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string deletesite;
  MenuSelectOption msop;
  MenuSelectOption msosj;
  MenuSelectOption msotj;
  MenuSelectOption msos;
  bool gotomode;
  bool temphighlightline;
  std::shared_ptr<Race> abortrace;
  std::shared_ptr<Race> abortdeleteraceinc;
  std::shared_ptr<Race> abortdeleteraceall;
  std::shared_ptr<TransferJob> abortjob;
};
