#pragma once

#include <map>
#include <memory>
#include <string>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Engine;
class Race;

class AllRacesScreen : public UIWindow {
public:
  AllRacesScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw() override;
  void command(const std::string &, const std::string &) override;
  bool keyPressed(unsigned int) override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  static void addRaceTableHeader(unsigned int, MenuSelectOption &, const std::string &);
  static void addRaceDetails(unsigned int, MenuSelectOption &, std::shared_ptr<Race>);
private:
  static void addRaceTableRow(unsigned int, MenuSelectOption &, unsigned int, bool, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);
  MenuSelectOption table;
  Engine * engine;
  bool hascontents;
  unsigned int currentviewspan;
  unsigned int ypos;
  bool temphighlightline;
  std::shared_ptr<Race> abortrace;
  std::shared_ptr<Race> abortdeleteraceinc;
  std::shared_ptr<Race> abortdeleteraceall;
};
