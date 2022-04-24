#include "menuselectoption.h"

#include <cassert>
#include <cstdlib>

#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptionnumarrow.h"
#include "menuselectoptioncheckbox.h"
#include "menuselectoptiontextbutton.h"
#include "menuselectoptiontextarrow.h"
#include "menuselectadjustableline.h"
#include "virtualview.h"

#define MAX_DIFF_LR 30
#define MAX_DIFF_UD 2

enum Direction {
  UP,
  DOWN,
  LEFT,
  RIGHT
};

MenuSelectOption::MenuSelectOption(VirtualView& vv, bool affectvvvertical) : vv(vv), affectvvvertical(affectvvvertical) {
  pointer = 0;
  lastpointer = 0;
}

MenuSelectOption::~MenuSelectOption() {

}

bool MenuSelectOption::navigate(int dir) {
  if (!options.size()) return false;
  int ccol = options[pointer]->getCol();
  int crow = options[pointer]->getRow();
  unsigned int closestelem;
  bool movefound = false;
  int closest = -1;
  for (unsigned int i = 0; i < options.size(); i++) {
    if (!options[i]->visible() || !options[i]->isSelectable()) {
      continue;
    }
    int row = options[i]->getRow();
    int col = options[i]->getCol();
    int distance = abs(row - crow) * 30 + abs(col - ccol);
    if ((((dir == DOWN && row > crow) || (dir == UP && row < crow)) &&
         abs(col - ccol) <= MAX_DIFF_LR) ||
        (((dir == LEFT && col < ccol) || (dir == RIGHT && col > ccol)) &&
         abs(row - crow) <= MAX_DIFF_UD))
    {
      if (distance < closest || closest == -1) {
        closest = distance;
        closestelem = i;
        movefound = true;
      }
    }
  }
  if (movefound) {
    const std::shared_ptr<MenuSelectOptionElement>& msoe = options[closestelem];
    if (affectvvvertical || static_cast<unsigned int>(crow) == msoe->getRow()) {
      vv.viewPosition(msoe->getRow(), msoe->getCol(), msoe->getTotalWidth());
    }
    lastpointer = pointer;
    pointer = closestelem;
    return true;
  }
  if ((dir == DOWN && leavedown) || (dir == UP && leaveup) ||
      (dir == LEFT && leaveleft) || (dir == RIGHT && leaveright))
    {
    lastpointer = pointer;
    focus = false;
    return true;
  }
  return false;
}

bool MenuSelectOption::goDown() {
  return navigate(DOWN);

}

bool MenuSelectOption::goUp() {
  return navigate(UP);
}

bool MenuSelectOption::goRight() {
  return navigate(RIGHT);
}

bool MenuSelectOption::goLeft() {
  return navigate(LEFT);
}

bool MenuSelectOption::goNext() {
  if (!options.size()) return false;
  unsigned int temppointer = pointer;
  while (pointer < options.size() - 1) {
    pointer++;
    if (options[pointer]->isSelectable()) {
      lastpointer = temppointer;
      return true;
    }
  }
  return false;
}

bool MenuSelectOption::goPrevious() {
  if (!options.size()) return false;
  unsigned int temppointer = pointer;
  while (pointer > 0) {
    pointer--;
    if (options[pointer]->isSelectable()) {
      lastpointer = temppointer;
      return true;
    }
  }
  return false;
}

std::shared_ptr<MenuSelectOptionTextField> MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret) {
  return addStringField(row, col, identifier, label, starttext, secret, 32, 32);
}

std::shared_ptr<MenuSelectOptionTextField> MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret, int maxlen) {
  return addStringField(row, col, identifier, label, starttext, secret, maxlen, maxlen);
}

std::shared_ptr<MenuSelectOptionTextField> MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret, int visiblelen, int maxlen) {
  std::shared_ptr<MenuSelectOptionTextField> msotf(std::make_shared<MenuSelectOptionTextField>(identifier, row, col, label, starttext, visiblelen, maxlen, secret));
  options.push_back(msotf);
  return msotf;
}

std::shared_ptr<MenuSelectOptionTextArrow> MenuSelectOption::addTextArrow(int row, int col, std::string identifier, std::string label) {
  std::shared_ptr<MenuSelectOptionTextArrow> msota(std::make_shared<MenuSelectOptionTextArrow>(identifier, row, col, label));
  options.push_back(msota);
  return msota;
}

std::shared_ptr<MenuSelectOptionNumArrow> MenuSelectOption::addIntArrow(int row, int col, std::string identifier, std::string label, int startval, int min, int max) {
  std::shared_ptr<MenuSelectOptionNumArrow> msona(std::make_shared<MenuSelectOptionNumArrow>(identifier, row, col, label, startval, min, max));
  options.push_back(msona);
  return msona;
}

std::shared_ptr<MenuSelectOptionCheckBox> MenuSelectOption::addCheckBox(int row, int col, std::string identifier, std::string label, bool startval) {
  std::shared_ptr<MenuSelectOptionCheckBox> msocb(std::make_shared<MenuSelectOptionCheckBox>(identifier, row, col, label, startval));
  options.push_back(msocb);
  return msocb;
}

std::shared_ptr<MenuSelectOptionTextButton> MenuSelectOption::addTextButton(int row, int col, std::string identifier, std::string label) {
  std::shared_ptr<MenuSelectOptionTextButton> msotb(std::make_shared<MenuSelectOptionTextButton>(identifier, row, col, label, true));
  options.push_back(msotb);
  return msotb;
}

std::shared_ptr<MenuSelectOptionTextButton> MenuSelectOption::addTextButtonNoContent(int row, int col, std::string identifier, std::string label) {
  std::shared_ptr<MenuSelectOptionTextButton> msotb(std::make_shared<MenuSelectOptionTextButton>(identifier, row, col, label, false));
  options.push_back(msotb);
  return msotb;
}

std::shared_ptr<MenuSelectAdjustableLine> MenuSelectOption::addAdjustableLine() {
  std::shared_ptr<MenuSelectAdjustableLine> msal(std::make_shared<MenuSelectAdjustableLine>());
  adjustablelines.push_back(msal);
  return msal;
}

std::shared_ptr<MenuSelectAdjustableLine> MenuSelectOption::addAdjustableLineBefore(std::shared_ptr<MenuSelectAdjustableLine> before) {
  std::shared_ptr<MenuSelectAdjustableLine> msal(std::make_shared<MenuSelectAdjustableLine>());
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == before) {
      adjustablelines.insert(it, msal);
      return msal;
    }
  }
  adjustablelines.push_back(msal);
  return msal;
}

std::shared_ptr<MenuSelectOptionElement> MenuSelectOption::getElement(unsigned int i) const {
  if (i >= size()) {
    return std::shared_ptr<MenuSelectOptionElement>();
  }
  return options[i];
}

std::shared_ptr<MenuSelectOptionElement> MenuSelectOption::getElement(std::string identifier) const {
  std::vector<std::shared_ptr<MenuSelectOptionElement> >::const_iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    if ((*it)->getIdentifier() == identifier) {
      return *it;
    }
  }
  return std::shared_ptr<MenuSelectOptionElement>();
}

unsigned int MenuSelectOption::getLastSelectionPointer() const {
  return lastpointer;
}

unsigned int MenuSelectOption::getSelectionPointer() const {
  return pointer;
}

bool MenuSelectOption::activateSelected() {
  return getElement(pointer)->activate();
}

void MenuSelectOption::clear() {
  options.clear();
  adjustablelines.clear();
}

void MenuSelectOption::reset() {
  clear();
  pointer = 0;
  lastpointer = 0;
  focus = false;
}

void MenuSelectOption::enterFocusFrom(int dir) {
  focus = true;
  if (lastpointer) {
    pointer = lastpointer;
  }
  else if (dir == 2) {
    pointer = lastpointer = size() - 1;
  }
  checkPointer();
  if (size()) {
    const std::shared_ptr<MenuSelectOptionElement>& msoe = options[pointer];
    vv.viewPosition(msoe->getRow(), msoe->getCol(), msoe->getTotalWidth());
  }
}

unsigned int MenuSelectOption::size() const {
  return options.size();
}

unsigned int MenuSelectOption::linesSize() const {
  return adjustablelines.size();
}

void MenuSelectOption::adjustLines(unsigned int linesize) {
  if (!adjustablelines.size()) {
    return;
  }
  unsigned int elementcount = adjustablelines[0]->size();
  if (!elementcount) {
    return;
  }
  std::vector<unsigned int> maxwantedwidths;
  std::vector<unsigned int> maxwidths;
  std::vector<unsigned int> averagewantedwidths;
  maxwantedwidths.resize(elementcount); // int is initialized to 0
  maxwidths.resize(elementcount);
  averagewantedwidths.resize(elementcount);
  int shortspaces = 0;
  for (std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    for (unsigned int i = 0; i < elementcount; i++) {
      std::shared_ptr<ResizableElement> re = (*it)->getElement(i);
      unsigned int wantedwidth = re->wantedWidth();
      if (wantedwidth > maxwantedwidths[i]) {
        maxwantedwidths[i] = wantedwidth;
      }
      averagewantedwidths[i] += wantedwidth;
      if (it == adjustablelines.begin() && re->shortSpacing() && i + 1 != elementcount) {
        shortspaces++;
      }
    }
  }
  unsigned int totalwantedwidth = (maxwantedwidths.size() - 1) * RESIZE_SPACING -
      shortspaces * (RESIZE_SPACING - RESIZE_SPACING_SHORT);
  for (unsigned int i = 0; i < maxwantedwidths.size(); i++) {
    totalwantedwidth += maxwantedwidths[i];
    maxwidths[i] = maxwantedwidths[i];
    averagewantedwidths[i] = (averagewantedwidths[i] * 0.9) / adjustablelines.size();
  }
  while (totalwantedwidth != linesize) {
    if (totalwantedwidth < linesize) {
      bool expanded = false;
      for (unsigned int i = 0; i < elementcount; i++) {
        std::shared_ptr<ResizableElement> elem = adjustablelines[0]->getElement(i);
        if (elem->isExpandable()) {
          unsigned int expansion = linesize - totalwantedwidth;
          maxwidths[i] += expansion;
          totalwantedwidth += expansion;
          expanded = true;
          break;
        }
      }
      if (!expanded) {
        break;
      }
    }
    else if (totalwantedwidth > linesize) {
      int leastimportant = -1;
      int leastimportantprio = 0;
      bool leastimportantpartialremove = false;
      bool leastimportanthighprio = false;
      for (unsigned int i = 0; i < elementcount; i++) {
        if (!adjustablelines[0]->getElement(i)->isVisible()) {
          continue;
        }
        std::shared_ptr<ResizableElement> re = adjustablelines[0]->getElement(i);
        int prio = re->lowPriority();
        int highprio = re->highPriority();
        bool partialremove = false;
        bool athighprio = false;
        if (prio != highprio) {
          partialremove = true;
          if (maxwidths[i] <= averagewantedwidths[i]) {
            athighprio = true;
            prio = highprio;
            partialremove = false;
          }
        }
        if (prio < leastimportantprio || leastimportant < 0) {
          leastimportantprio = prio;
          leastimportant = i;
          leastimportantpartialremove = partialremove;
          leastimportanthighprio = athighprio;
        }
      }
      std::shared_ptr<ResizableElement> leastimportantelem = adjustablelines[0]->getElement(leastimportant);
      int spacing = leastimportantelem->shortSpacing() ? RESIZE_SPACING_SHORT : RESIZE_SPACING;
      unsigned int maxsaving = maxwantedwidths[leastimportant] + spacing;
      unsigned int resizemethod = leastimportantelem->resizeMethod();
      switch (resizemethod) {
        case RESIZE_REMOVE:
          leastimportantelem->setVisible(false);
          totalwantedwidth -= maxsaving;
          break;
        case RESIZE_WITHDOTS:
        case RESIZE_CUTEND:
        case RESIZE_WITHLAST3: {
          int maxwantedwidth = (leastimportanthighprio ? averagewantedwidths : maxwantedwidths)[leastimportant];
          if (leastimportantpartialremove || totalwantedwidth - maxwantedwidth < linesize) {
            int reduction = leastimportantpartialremove ?
                maxwantedwidth - averagewantedwidths[leastimportant] :
                totalwantedwidth - linesize;
            maxwidths[leastimportant] = maxwantedwidth - reduction;
            totalwantedwidth -= reduction;
          }
          else {
            leastimportantelem->setVisible(false);
            totalwantedwidth -= maxsaving;
          }
          break;
        }
      }
    }
  }
  int startpos = adjustablelines[0]->getElement(0)->getCol();
  for (std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    int elementpos = startpos;
    for (unsigned int i = 0; i < elementcount; i++) {
      std::shared_ptr<ResizableElement> elem = (*it)->getElement(i);
      if (adjustablelines[0]->getElement(i)->isVisible()) {
        elem->setMaxWidth(maxwidths[i]);
        elem->setPosition(elem->getRow(), elementpos);
        int spacing = elem->shortSpacing() ? RESIZE_SPACING_SHORT : RESIZE_SPACING;
        elementpos += maxwidths[i] + spacing;
      }
      else {
        elem->setVisible(false);
      }
    }
  }
}

void MenuSelectOption::checkPointer() {
  if (pointer >= size()) {
    pointer = size() - 1;
  }
  if (size() == 0) {
    pointer = 0;
  }
  else {
    while ((!options[pointer]->visible() || !options[pointer]->isSelectable()) && pointer > 0) pointer--;
    while ((!options[pointer]->visible() || !options[pointer]->isSelectable()) && pointer < size() - 1) pointer++;
  }
  lastpointer = pointer;
}

std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator MenuSelectOption::linesBegin() {
  return adjustablelines.begin();
}

std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator MenuSelectOption::linesEnd() {
  return adjustablelines.end();
}

std::shared_ptr<MenuSelectAdjustableLine> MenuSelectOption::getAdjustableLine(std::shared_ptr<MenuSelectOptionElement> msoe) const {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::const_iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    for (unsigned int i = 0; i < (*it)->size(); i++) {
      if ((*it)->getElement(i) == msoe) {
        return *it;
      }
    }
  }
  return std::shared_ptr<MenuSelectAdjustableLine>();
}

std::shared_ptr<MenuSelectAdjustableLine> MenuSelectOption::getAdjustableLine(unsigned int lineindex) const {
  assert(lineindex < adjustablelines.size());
  return adjustablelines[lineindex];
}

std::shared_ptr<MenuSelectAdjustableLine> MenuSelectOption::getAdjustableLineOnRow(unsigned int row) const {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::const_iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (!(*it)->empty() && (*it)->getElement(0)->getRow() == row) {
      return *it;
    }
  }
  return std::shared_ptr<MenuSelectAdjustableLine>();
}

void MenuSelectOption::removeAdjustableLine(std::shared_ptr<MenuSelectAdjustableLine> msal) {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == msal) {
      for (unsigned int i = 0; i < (*it)->size(); i++) {
        removeElement(msal->getElement(i));
      }
      adjustablelines.erase(it);
      return;
    }
  }
}

void MenuSelectOption::removeElement(std::shared_ptr<MenuSelectOptionElement> msoe) {
  std::vector<std::shared_ptr<MenuSelectOptionElement> >::iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    if (*it == msoe) {
      options.erase(it);
      return;
    }
  }
}

void MenuSelectOption::setPointer(std::shared_ptr<MenuSelectOptionElement> set) {
  for (unsigned int i = 0; i < options.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = options[i];
    if (msoe == set) {
      pointer = i;
      return;
    }
  }
}

bool MenuSelectOption::swapLineWithNext(std::shared_ptr<MenuSelectAdjustableLine> msal) {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == msal) {
      if (it + 1 != adjustablelines.end()) {
        std::shared_ptr<MenuSelectAdjustableLine> swap = *(it + 1);
        *(it + 1) = msal;
        *it = swap;
        return true;
      }
      return false;
    }
  }
  return false;
}

bool MenuSelectOption::swapLineWithPrevious(std::shared_ptr<MenuSelectAdjustableLine> msal) {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == msal) {
      if (it != adjustablelines.begin()) {
        std::shared_ptr<MenuSelectAdjustableLine> swap = *(it - 1);
        *(it - 1) = msal;
        *it = swap;
        return true;
      }
      return false;
    }
  }
  return false;
}

int MenuSelectOption::getLineIndex(std::shared_ptr<MenuSelectAdjustableLine> msal) {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it;
  int index = 0;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == msal) {
      return index;
    }
    index++;
  }
  return -1;
}
