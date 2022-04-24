#include "scoreboardscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "../../globalcontext.h"
#include "../../scoreboard.h"
#include "../../engine.h"
#include "../../scoreboardelement.h"
#include "../../sitelogic.h"
#include "../../site.h"

ScoreBoardScreen::ScoreBoardScreen(Ui* ui) : UIWindow(ui, "ScoreBoardScreen"), table(*vv) {
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
}

ScoreBoardScreen::~ScoreBoardScreen() {

}

void ScoreBoardScreen::initialize(unsigned int row, unsigned int col) {
  engine = global->getEngine();
  scoreboard = engine->getScoreBoard();
  autoupdate = true;
  currentviewspan = 0;
  init(row, col);
}

void ScoreBoardScreen::redraw() {
  vv->clear();
  unsigned int y = 0;
  table.clear();
  std::shared_ptr<MenuSelectAdjustableLine> msal;
  std::shared_ptr<MenuSelectOptionTextButton> msotb;
  msal = table.addAdjustableLine();
  msotb = table.addTextButtonNoContent(y, 1, "filename", "FILE NAME");
  msal->addElement(msotb, 2, 0, RESIZE_CUTEND, true);
  msotb = table.addTextButtonNoContent(y, 2, "sites", "SITES");
  msal->addElement(msotb, 4, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 3, "potential", "POTENTIAL");
  msal->addElement(msotb, 1, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 4, "score", "SCORE");
  msal->addElement(msotb, 3, RESIZE_REMOVE);
  y++;

  std::vector<ScoreBoardElement *>::const_iterator it;
  if (currentviewspan < scoreboard->size()) {
    for (it = scoreboard->begin() + currentviewspan; it != scoreboard->end() && y < row; it++, y++) {
      std::string sites = (*it)->getSource()->getSite()->getName() + " -> " + (*it)->getDestination()->getSite()->getName();
      msal = table.addAdjustableLine();
      msotb = table.addTextButtonNoContent(y, 1, "filename", (*it)->fileName());
      msal->addElement(msotb, 2, 0, RESIZE_CUTEND, true);
      msotb = table.addTextButtonNoContent(y, 2, "sites", sites);
      msal->addElement(msotb, 4, RESIZE_REMOVE);
      msotb = table.addTextButtonNoContent(y, 3, "potential", std::to_string((*it)->getSource()->getPotential()));
      msal->addElement(msotb, 1, RESIZE_REMOVE);
      msotb = table.addTextButtonNoContent(y, 4, "score", std::to_string((*it)->getScore()));
      msal->addElement(msotb, 3, RESIZE_REMOVE);
    }
  }
  table.adjustLines(col - 3);
  bool highlight;
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    highlight = false;
    if (table.getSelectionPointer() == i) {
      //highlight = true; // later problem
    }
    if (re->isVisible()) {
      vv->putStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }

  printSlider(vv, row, 1, col - 1, scoreboard->size(), currentviewspan);
}

bool ScoreBoardScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  switch(action) {
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_UP:
      if (currentviewspan < 2) {
        currentviewspan = 0;
      }
      else {
        currentviewspan -= 2;
      }
      ui->update();
      return true;
    case KEYACTION_DOWN:
      if (row < scoreboard->size() + 1) {
        if (currentviewspan < scoreboard->size() + 1 - row) {
          currentviewspan += 2;
        }
        if (currentviewspan > scoreboard->size() + 1 - row) {
          currentviewspan = scoreboard->size() + 1 - row;
        }
      }
      if (row >= scoreboard->size() + 1) {
        currentviewspan = 0;
      }
      ui->update();
      return true;
    case KEYACTION_PREVIOUS_PAGE:
      if (currentviewspan < row * 0.5) {
        currentviewspan = 0;
      }
      else {
        currentviewspan -= row * 0.5;
      }
      ui->update();
      return true;
    case KEYACTION_NEXT_PAGE:
      if (row * 1.5 < scoreboard->size() + 1 && currentviewspan < scoreboard->size() + 1 - row * 1.5) {
        currentviewspan += row * 0.5;
      }
      else if (scoreboard->size() + 1 > row) {
        currentviewspan = scoreboard->size() + 1 - row;
      }
      ui->update();
      return true;
    case KEYACTION_TOP:
      currentviewspan = 0;
      ui->update();
      return true;
    case KEYACTION_BOTTOM:
      if (scoreboard->size() + 1 > row) {
        currentviewspan = scoreboard->size() + 1 - row;
      }
      ui->update();
      return true;
  }
  return false;
}

std::string ScoreBoardScreen::getLegendText() const {
  return keybinds.getLegendSummary();
}

std::string ScoreBoardScreen::getInfoLabel() const {
  return "SCOREBOARD";
}

std::string ScoreBoardScreen::getInfoText() const {
  std::string size = std::to_string(scoreboard->size());
  std::string max = std::to_string((int)scoreboard->getElementVector().size());
  return "Size: " + size + "  Max: " + max;
}
