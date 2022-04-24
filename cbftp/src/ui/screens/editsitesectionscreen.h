#pragma once

#include <list>
#include <memory>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Site;
class MenuSelectOptionElement;
class Path;

class EditSiteSectionScreen : public UIWindow {
  enum class Mode {
    ADD,
    EDIT
  };
public:
  EditSiteSectionScreen(Ui *);
  ~EditSiteSectionScreen();
  void initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const Path & path);
  void initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const std::string & section);
  void redraw() override;
  bool keyPressed(unsigned int) override;
  void command(const std::string & command, const std::string & arg) override;
  std::string getLegendText() const override;
  std::string getInfoLabel() const override;
private:
  void initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const std::string & section, const Path & path);
  bool setSections();
  std::shared_ptr<Site> modsite;
  bool active;
  std::shared_ptr<MenuSelectOptionElement> activeelement;
  MenuSelectOption mso;
  Mode mode;
  std::string oldsection;
  bool exists;
};
