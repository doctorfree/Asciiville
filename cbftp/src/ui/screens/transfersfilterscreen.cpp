#include "transfersfilterscreen.h"

#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptioncheckbox.h"

#include "../../commandowner.h"
#include "../../globalcontext.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../engine.h"
#include "../../race.h"
#include "../../transferjob.h"
#include "../../util.h"

namespace {

void fillPreselectionList(const std::string & preselectstr, std::list<std::shared_ptr<Site> > * list) {
  std::list<std::string> preselectlist = util::trim(util::split(preselectstr, ","));
  for (std::list<std::string>::const_iterator it = preselectlist.begin(); it != preselectlist.end(); it++) {
    std::shared_ptr<Site> site = global->getSiteManager()->getSite(*it);
    list->push_back(site);
  }
}

}
TransfersFilterScreen::TransfersFilterScreen(Ui* ui) : UIWindow(ui, "TransfersFilterScreen"), mso(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('f', KEYACTION_FILTER, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
  keybinds.addBind('r', KEYACTION_RESET, "Reset");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
}

TransfersFilterScreen::~TransfersFilterScreen() {

}

void TransfersFilterScreen::initialize(unsigned int row, unsigned int col, const TransferFilteringParameters & tfp) {
  mso.reset();
  selectedspreadjobs = tfp.spreadjobsfilter;
  selectedtransferjobs = tfp.transferjobsfilter;
  active = false;
  int y = 1;
  mso.addCheckBox(y++, 1, "jobfilter", "Enable job filtering:", tfp.usejobfilter);
  mso.addStringField(y++, 1, "spreadjobs", "Spread jobs:", getSpreadJobsText(), false, 60, 512);
  mso.addStringField(y++, 1, "transferjobs", "Transfer jobs:", getTransferJobsText(), false, 60, 512);
  y++;
  mso.addCheckBox(y++, 1, "sitesfilter", "Enable sites filtering:", tfp.usesitefilter);
  mso.addStringField(y++, 1, "source", "Source:", util::join(tfp.sourcesitefilters, ","), false, 60, 2048);
  mso.addStringField(y++, 1, "destination", "Destination:", util::join(tfp.targetsitefilters, ","), false, 60, 2048);
  mso.addStringField(y++, 1, "anydirection", "Any direction:", util::join(tfp.anydirectionsitefilters, ","), false, 60, 2048);
  y++;
  mso.addCheckBox(y++, 1, "filenamefilter", "Enable file name filtering:", tfp.usefilenamefilter);
  mso.addStringField(y++, 1, "filename", "File name:", tfp.filenamefilter, false, 60, 512);
  y++;
  mso.addCheckBox(y++, 1, "statusfilter", "Enable transfer status filtering:", tfp.usestatusfilter);
  mso.addCheckBox(y++, 1, "statusinprogress", "In progress:", tfp.showstatusinprogress);
  mso.addCheckBox(y++, 1, "statusdone", "Done:", tfp.showstatusdone);
  mso.addCheckBox(y++, 1, "statusfail", "Failed:", tfp.showstatusfail);
  mso.addCheckBox(y++, 1, "statusdupe", "Dupe:", tfp.showstatusdupe);
  mso.enterFocusFrom(0);
  init(row, col);
}

void TransfersFilterScreen::redraw() {
  vv->clear();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
      if (active && msoe->cursorPosition() >= 0) {
        ui->showCursor();
        vv->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
      }
      else {
        ui->hideCursor();
      }
    }
    vv->putStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    vv->putStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void TransfersFilterScreen::command(const std::string & command, const std::string & arg) {
  if (command == "returnselectitems") {
    std::list<std::string> items = util::trim(util::split(arg, ","));
    std::string identifier = activeelement->getIdentifier();
    if (identifier == "spreadjobs") {
      selectedspreadjobs.clear();
      for (std::list<std::string>::const_iterator it = items.begin(); it != items.end(); it++) {
        selectedspreadjobs.push_back(global->getEngine()->getRace(std::stoi(*it))->getName());
      }
      std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("spreadjobs"))->setText(getSpreadJobsText());
    }
    else if (identifier == "transferjobs") {
      selectedtransferjobs.clear();
      for (std::list<std::string>::const_iterator it = items.begin(); it != items.end(); it++) {
        selectedtransferjobs.push_back(global->getEngine()->getTransferJob(std::stoi(*it))->getName());
      }
      std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("transferjobs"))->setText(getTransferJobsText());
    }
    else {
      std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->setText(arg);
    }
    if (!items.empty()) {
      if (identifier == "spreadjobs" || identifier == "transferjobs") {
        std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("jobfilter"))->setValue(true);
      }
      else if (identifier == "source" || identifier == "destination" || identifier == "anydirection") {
        std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("sitesfilter"))->setValue(true);
      }
    }
    ui->redraw();
  }
}

bool TransfersFilterScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      if (activeelement->getIdentifier() == "filename" && !std::static_pointer_cast<MenuSelectOptionTextField>(activeelement)->getData().empty()) {
        std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("filenamefilter"))->setValue(true);
      }
      active = false;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  switch (action) {
    case KEYACTION_UP: {
      bool moved = mso.goUp();
      ui->update();
      return moved;
    }
    case KEYACTION_DOWN: {
      bool moved = mso.goDown();
      ui->update();
      return moved;
    }
    case KEYACTION_DONE:
    case KEYACTION_FILTER:
    {
      TransferFilteringParameters tfp;
      tfp.usejobfilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("jobfilter"))->getData();
      tfp.spreadjobsfilter = selectedspreadjobs;
      tfp.transferjobsfilter = selectedtransferjobs;
      tfp.usesitefilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("sitesfilter"))->getData();
      tfp.sourcesitefilters = util::trim(util::split(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("source"))->getData(), ","));
      tfp.targetsitefilters = util::trim(util::split(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("destination"))->getData(), ","));
      tfp.anydirectionsitefilters = util::trim(util::split(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("anydirection"))->getData(), ","));
      tfp.usefilenamefilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("filenamefilter"))->getData();
      tfp.filenamefilter = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("filename"))->getData();
      tfp.usestatusfilter = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusfilter"))->getData();
      tfp.showstatusinprogress = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusinprogress"))->getData();
      tfp.showstatusdone = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusdone"))->getData();
      tfp.showstatusfail= std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusfail"))->getData();
      tfp.showstatusdupe = std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusdupe"))->getData();
      ui->returnTransferFilters(tfp);
      return true;
    }
    case KEYACTION_ENTER:
    {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      if (msoe->getIdentifier() == "spreadjobs") {
        activeelement = msoe;
        ui->goSelectSpreadJobs();
        return true;
      }
      if (msoe->getIdentifier() == "transferjobs") {
        activeelement = msoe;
        ui->goSelectTransferJobs();
        return true;
      }
      if (msoe->getIdentifier() == "source" || msoe->getIdentifier() == "destination" || msoe->getIdentifier() == "anydirection") {
        std::string preselectstr = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
        std::list<std::shared_ptr<Site> > preselected;
        fillPreselectionList(preselectstr, &preselected);
        activeelement = msoe;
        std::string headerword = msoe->getIdentifier() + " ";
        if (headerword == "anydirection") {
          headerword = "";
        }
        ui->goSelectSites("Show these " + headerword + "sites in transfers", preselected, std::list<std::shared_ptr<Site> >());
        return true;
      }
      bool activation = msoe->activate();
      if (!activation) {
        std::string identifier = msoe->getIdentifier();
        if ((identifier == "statusinprogress" || identifier == "statusdone" || identifier == "statusdupe" || identifier == "statusfail") &&
            std::static_pointer_cast<MenuSelectOptionCheckBox>(msoe)->getData())
        {
          std::static_pointer_cast<MenuSelectOptionCheckBox>(mso.getElement("statusfilter"))->setValue(true);
        }
        ui->update();
        return true;
      }
      active = true;
      activeelement = msoe;
      ui->setLegend();
      ui->update();
      return true;
    }
    case KEYACTION_RESET:
      initialize(row, col, TransferFilteringParameters());
      ui->redraw();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string TransfersFilterScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string TransfersFilterScreen::getInfoLabel() const {
  return "TRANSFERS FILTERING";
}

std::string TransfersFilterScreen::getSpreadJobsText() const {
  size_t size = selectedspreadjobs.size();
  std::string text;
  if (size == 1) {
    text = selectedspreadjobs.front();
  }
  else if (size > 1) {
    text = std::to_string((int)size) + " spread jobs selected...";
  }
  return text;
}

std::string TransfersFilterScreen::getTransferJobsText() const {
  size_t size = selectedtransferjobs.size();
  std::string text;
  if (size == 1) {
    text = selectedtransferjobs.front();
  }
  else if (size > 1) {
    text = std::to_string((int)size) + " transfer jobs selected...";
  }
  return text;
}
