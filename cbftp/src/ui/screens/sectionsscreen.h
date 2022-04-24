#pragma once

#include <set>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;
class Section;

class SectionsScreen : public UIWindow {
public:
  SectionsScreen(Ui *);
  ~SectionsScreen();
  void initialize(unsigned int row, unsigned int col, bool selectsection = false, const std::list<std::string>& preselected = std::list<std::string>());
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  void command(const std::string & command, const std::string & arg) override;
private:
  enum class Mode {
    EDIT,
    SELECT
  };
  void addSectionTableHeader(unsigned int y, MenuSelectOption & mso);
  void addSectionDetails(unsigned int y, MenuSelectOption & mso, const Section & section);
  void addSectionTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
      const std::string& selected, const std::string & name, const std::string & skiplist, const std::string & hotkey, const std::string & numjobs,
      const std::string & numsites, const std::string & sites);
  int getCurrentScope() const;
  MenuSelectOption table;
  unsigned int currentviewspan;
  unsigned int ypos;
  bool temphighlightline;
  bool hascontents;
  unsigned int totallistsize;
  Mode mode;
  std::set<std::string> selected;
  bool togglestate;
};
