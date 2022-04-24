#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../site.h"

class MenuSelectOptionElement;
class Path;

class SiteSectionsScreen : public UIWindow {
public:
  SiteSectionsScreen(Ui *);
  ~SiteSectionsScreen();
  void initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
private:
  void addSectionTableHeader(unsigned int y, MenuSelectOption & mso);
  void addSectionDetails(unsigned int y, MenuSelectOption & mso, const std::string & section, const Path & path);
  void addSectionTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
      const std::string & name, const std::string & path);
  MenuSelectOption table;
  std::shared_ptr<Site> site;
  std::shared_ptr<Site> sitecopy;
  unsigned int currentviewspan;
  unsigned int ypos;
  bool temphighlightline;
  bool hascontents;
  unsigned int totallistsize;
};
