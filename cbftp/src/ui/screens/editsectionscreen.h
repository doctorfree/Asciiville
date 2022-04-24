#pragma once

#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class MenuSelectOptionElement;
class Section;

class EditSectionScreen : public UIWindow {
  enum class Mode {
    ADD,
    EDIT
  };
public:
  EditSectionScreen(Ui *);
  ~EditSectionScreen();
  void initialize(unsigned int row, unsigned int col, const std::string & section = "");
  void redraw() override;
  bool keyPressed(unsigned int) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  void done();
  Section * section;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  Mode mode;
  std::string oldname;
};
