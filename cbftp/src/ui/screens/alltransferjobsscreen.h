#pragma once

#include <map>
#include <memory>
#include <string>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class Engine;
class TransferJob;

class AllTransferJobsScreen : public UIWindow {
public:
  AllTransferJobsScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw() override;
  void command(const std::string &, const std::string &) override;
  bool keyPressed(unsigned int) override;
  std::string getInfoLabel() const override;
  std::string getInfoText() const override;
  static void addJobTableHeader(unsigned int, MenuSelectOption &, const std::string &);
  static void addJobDetails(unsigned int, MenuSelectOption &, std::shared_ptr<TransferJob>);
private:
  static void addJobTableRow(unsigned int, MenuSelectOption &, unsigned int, bool, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &,
      const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);
  MenuSelectOption table;
  Engine * engine;
  bool hascontents;
  unsigned int currentviewspan;
  unsigned int ypos;
  bool temphighlightline;
  std::shared_ptr<TransferJob> abortjob;
};
