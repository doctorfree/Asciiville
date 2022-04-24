#include "legendprintertransferjob.h"

#include "../transferjob.h"
#include "../util.h"
#include "../globalcontext.h"
#include "../engine.h"

#include "menuselectadjustableline.h"
#include "menuselectoptiontextbutton.h"

#include "screens/transferjobstatusscreen.h"

#include "ui.h"

LegendPrinterTransferJob::LegendPrinterTransferJob(Ui * ui, unsigned int id) : ui(ui), mso(ui->getVirtualView()), jobfinishedprintcount(0) {
  transferjob = global->getEngine()->getTransferJob(id);
}

bool LegendPrinterTransferJob::print() {
  if (!transferjob) {
    return false;
  }
  mso.reset();
  std::string timespent = util::simpleTimeFormat(transferjob->timeSpent());
  bool running = transferjob->getStatus() == TRANSFERJOB_RUNNING;
  bool started = transferjob->getStatus() != TRANSFERJOB_QUEUED;
  std::string timeremaining = running ? util::simpleTimeFormat(transferjob->timeRemaining()) : "-";
  std::string route = TransferJobStatusScreen::getRoute(transferjob);
  std::string sizeprogress = util::parseSize(transferjob->sizeProgress()) +
                             " / " + util::parseSize(transferjob->totalSize());
  std::string filesprogress = std::to_string(transferjob->filesProgress()) + "/" +
                              std::to_string(transferjob->filesTotal());
  std::string speed = started ? util::parseSize(transferjob->getSpeed() * SIZEPOWER) + "/s" : "-";
  std::string progress = std::to_string(transferjob->getProgress()) + "%";
  std::string status;
  switch (transferjob->getStatus()) {
    case TRANSFERJOB_QUEUED:
      status = "queue";
      break;
    case TRANSFERJOB_RUNNING:
      status = progress;
      break;
    case TRANSFERJOB_DONE:
      status = "done";
      jobfinishedprintcount++;
      break;
    case TRANSFERJOB_ABORTED:
      status = "abor";
      jobfinishedprintcount++;
      break;
  }
  std::string type = "FXP";
  switch (transferjob->getType()) {
    case TRANSFERJOB_DOWNLOAD:
      type = "DL";
      break;
    case TRANSFERJOB_UPLOAD:
      type = "UL";
      break;
  }

  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(1, 4, "timespent", timespent);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButton(1, 4, "type", type);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButton(1, 4, "name", transferjob->getName());
  msal->addElement(msotb, 11, 0, RESIZE_CUTEND, true);

  msotb = mso.addTextButton(1, 4, "route", route);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "sizeprogress", sizeprogress);
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "filesprogress", filesprogress);
  msal->addElement(msotb, 1, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "remaining", timeremaining);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "speed", speed);
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(1, 4, "status", status);
  msal->addElement(msotb, 10, RESIZE_REMOVE);

  mso.adjustLines(col - 8);

  unsigned int highlightuntilpos = 4 + ((col - 8) * transferjob->getMilliProgress()) / 1000;
  if (transferjob->getMilliProgress() == 0) {
    highlightuntilpos = 0;
  }
  for (unsigned int i = 4; i < col - 4; i++) {
    bool highlight = i <= highlightuntilpos;
    ui->getRenderer().printChar(1, i, ' ', highlight, false, -1, Window::LEGEND);
  }
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(mso.getElement(i));
    if (!re->isVisible()) {
      continue;
    }
    std::string text = re->getLabelText();
    for (unsigned int i = 0; i < text.length(); i++) {
      bool highlight = i + re->getCol() <= highlightuntilpos;
      ui->getRenderer().printChar(re->getRow(), re->getCol() + i, text[i], highlight, false, -1, Window::LEGEND);
    }
  }
  if (jobfinishedprintcount >= 20) {
    return false;
  }
  return true;
}
