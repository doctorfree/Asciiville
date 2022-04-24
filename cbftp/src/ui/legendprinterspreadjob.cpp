#include "legendprinterspreadjob.h"

#include "../race.h"
#include "../util.h"
#include "../globalcontext.h"
#include "../engine.h"

#include "menuselectadjustableline.h"
#include "menuselectoptiontextbutton.h"

#include "ui.h"

LegendPrinterSpreadJob::LegendPrinterSpreadJob(Ui * ui, unsigned int id) : ui(ui), mso(ui->getVirtualView()), jobfinishedprintcount(0) {
  race = global->getEngine()->getRace(id);
}

bool LegendPrinterSpreadJob::print() {
  if (!race) {
    return false;
  }
  mso.reset();
  std::string done = std::to_string(race->numSitesDone()) + "/" + std::to_string(race->numSites());
  std::string timespent = util::simpleTimeFormat(race->getTimeSpent());
  std::string status;
  std::string worst = std::to_string(race->getWorstCompletionPercentage()) + "%";
  std::string avg = std::to_string(race->getAverageCompletionPercentage()) + "%";
  std::string best = std::to_string(race->getBestCompletionPercentage()) + "%";
  std::string size = util::parseSize(race->estimatedTotalSize());
  switch (race->getStatus()) {
    case RaceStatus::RUNNING:
      status = "running";
      break;
    case RaceStatus::DONE:
      status = "done";
      jobfinishedprintcount++;
      break;
    case RaceStatus::ABORTED:
      status = "aborted";
      jobfinishedprintcount++;
      break;
    case RaceStatus::TIMEOUT:
      status = "timeout";
      jobfinishedprintcount++;
      break;
  }

  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(1, 4, "timespent", timespent);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "section", race->getSection());
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButton(1, 4, "release", race->getName());
  msal->addElement(msotb, 12, 1, RESIZE_CUTEND, true);

  msotb = mso.addTextButtonNoContent(1, 4, "size", size);
  msal->addElement(msotb, 10, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "worst", worst);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "avg", avg);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "best", best);
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "status", status);
  msal->addElement(msotb, 11, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "done", done);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "sites", race->getSiteListText());
  msal->addElement(msotb, 0, RESIZE_WITHDOTS);

  mso.adjustLines(col - 8);

  for (unsigned int i = 4; i < col - 4; i++) {
    ui->getRenderer().printChar(1, i, ' ', false, false, -1, Window::LEGEND);
  }
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(mso.getElement(i));
    if (!re->isVisible()) {
      continue;
    }
    ui->getRenderer().printStr(re->getRow(), re->getCol(), re->getLabelText(), false, false, -1, -1, false, Window::LEGEND);
  }
  if (jobfinishedprintcount >= 20) {
    return false;
  }
  return true;
}
