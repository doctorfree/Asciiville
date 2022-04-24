#include "virtualview.h"

#include "renderer.h"
#include "termint.h"
#include "misc.h"

#define HORIZONTAL_STEP 4
#define VERTICAL_STEP 2

VirtualView::VirtualView(Renderer& renderer) : renderer(renderer), currentviewrow(0), currentviewcol(0), redrawall(true),
                              currentcleariteration(0), currentredrawiteration(0), maxrenderedrow(0), maxrenderedcol(0)
{
}

void VirtualView::putStr(unsigned int row, unsigned int col, const std::string& str, bool highlight, int maxlen, bool rightalign) {
  int len = str.length();
  size_t offset = rightalign ? (maxlen > len ? maxlen - len : 0) : 0;
  expandIfNeeded(row, col + offset + len - 1);
  int color = encodeColorRepresentation();
  bool bold = false;
  int writepos = 0;
  for (size_t i = 0; i < str.length() && (maxlen < 0 || static_cast<int>(i) < maxlen); ++i) {
    if (len - i > 3 && str[i] == '%') {
      if (str[i+1] == 'C' && str[i+2] == '(') {
        if (str[i+3] == ')') {
          color = encodeColorRepresentation();
          i += 3;
          continue;
        }
        else if (len - i > 4 && str[i+4] == ')') {
          int arg = atoi(reinterpret_cast<const char*>(str.data() + i + 3));
          color = encodeColorRepresentation(arg);
          i += 4;
          continue;
        }
        else if (len - i > 6 && str[i+4] == ',' && str[i+6] == ')') {
          int arg1 = atoi(reinterpret_cast<const char*>(str.data() + i + 3));
          int arg2 = atoi(reinterpret_cast<const char*>(str.data() + i + 5));
          color = encodeColorRepresentation(arg1, arg2);
          i += 6;
          continue;
        }
      }
      else if (str[i+1] == 'B' && str[i+2] == '(' && str[i+3] == ')') {
        bold ^= 1;
        i += 3;
        continue;
      }
    }
    VirtualViewElement& vve = virtualchars[row][col + writepos + offset];
    vve.set(str[i], highlight, bold, color, currentcleariteration, currentredrawiteration);
    if (col + writepos > maxrenderedcol) {
      maxrenderedcol = col + writepos;
    }
    if (vve.isModified()) {
      modifiedchars.emplace_back(row, col + writepos + offset);
    }
    writepos++;
  }
  if (row > maxrenderedrow) {
    maxrenderedrow = row;
  }
}

void VirtualView::putStr(unsigned int row, unsigned int col, const std::basic_string<unsigned int>& str, bool highlight, int maxlen, bool rightalign) {
  int len = str.length();
  size_t offset = rightalign ? (maxlen > len ? maxlen - len : 0) : 0;
  expandIfNeeded(row, col + offset + len - 1);
  int color = encodeColorRepresentation();
  bool bold = false;
  int writepos = 0;
  for (size_t i = 0; i < static_cast<unsigned int>(len) && (maxlen < 0 || static_cast<int>(i) < maxlen); ++i) {
    if (len - i > 3 && str[i] == '%') {
      if (str[i+1] == 'C' && str[i+2] == '(') {
        if (str[i+3] == ')') {
          color = encodeColorRepresentation();
          i += 3;
          continue;
        }
        else if (len - i > 4 && str[i+4] == ')') {
          int arg = atoi(reinterpret_cast<const char*>(str.data() + i + 3));
          color = encodeColorRepresentation(arg);
          i += 4;
          continue;
        }
        else if (len - i > 6 && str[i+4] == ',' && str[i+6] == ')') {
          int arg1 = atoi(reinterpret_cast<const char*>(str.data() + i + 3));
          int arg2 = atoi(reinterpret_cast<const char*>(str.data() + i + 5));
          color = encodeColorRepresentation(arg1, arg2);
          i += 6;
          continue;
        }
      }
      else if (str[i+1] == 'B' && str[i+2] == '(' && str[i+3] == ')') {
        bold ^= 1;
        i += 3;
        continue;
      }
    }
    VirtualViewElement& vve = virtualchars[row][col + writepos + offset];
    vve.set(str[i], highlight, bold, color, currentcleariteration, currentredrawiteration);
    if (col + writepos > maxrenderedcol) {
      maxrenderedcol = col + writepos;
    }
    if (vve.isModified()) {
      modifiedchars.emplace_back(row, col + writepos + offset);
    }
    writepos++;
  }
  if (row > maxrenderedrow) {
    maxrenderedrow = row;
  }
}

void VirtualView::putChar(unsigned int row, unsigned int col, int c, bool highlight) {
  expandIfNeeded(row, col);
  VirtualViewElement& vve = virtualchars[row][col];
  vve.set(c, highlight, false, -1, currentcleariteration, currentredrawiteration);
  if (vve.isModified()) {
    modifiedchars.emplace_back(row, col);
  }
  if (col > maxrenderedcol) {
    maxrenderedcol = col;
  }
  if (row > maxrenderedrow) {
    maxrenderedrow = row;
  }
}

void VirtualView::highlightOn(unsigned int row, unsigned int col, unsigned int len) {
  expandIfNeeded(row, col + len - 1);
  for (size_t i = 0; i < len; ++i) {
    VirtualViewElement& vve = virtualchars[row][col + i];
    if (vve.getClearIteration() != currentcleariteration) {
      vve.clear();
    }
    vve.setHighlightOn(currentcleariteration, currentredrawiteration);
    if (vve.isModified()) {
      modifiedchars.emplace_back(row, col + i);
    }
  }
  if (col > maxrenderedcol) {
    maxrenderedcol = col;
  }
  if (row > maxrenderedrow) {
    maxrenderedrow = row;
  }
}

void VirtualView::highlightOff(unsigned int row, unsigned int col, unsigned int len) {
  expandIfNeeded(row, col + len - 1);
  for (size_t i = 0; i < len; ++i) {
    VirtualViewElement& vve = virtualchars[row][col + i];
    if (vve.getClearIteration() != currentcleariteration) {
      vve.clear();
    }
    vve.setHighlightOff(currentcleariteration, currentredrawiteration);
    if (vve.isModified()) {
      modifiedchars.emplace_back(row, col + i);
    }
  }
  if (col > maxrenderedcol) {
    maxrenderedcol = col;
  }
  if (row > maxrenderedrow) {
    maxrenderedrow = row;
  }
}

void VirtualView::setColor(unsigned int row, unsigned int col, int fgcolor, int bgcolor, unsigned int len) {
  expandIfNeeded(row, col + len - 1);
  for (size_t i = 0; i < len; ++i) {
    VirtualViewElement& vve = virtualchars[row][col + i];
    if (vve.getClearIteration() != currentcleariteration) {
      vve.clear();
    }
    vve.setColor(encodeColorRepresentation(fgcolor, bgcolor), currentcleariteration, currentredrawiteration);
    if (vve.isModified()) {
      modifiedchars.emplace_back(row, col + i);
    }
  }
  if (col > maxrenderedcol) {
    maxrenderedcol = col;
  }
  if (row > maxrenderedrow) {
    maxrenderedrow = row;
  }
}

void VirtualView::setBold(unsigned int row, unsigned int col, bool bold, unsigned int len) {
  expandIfNeeded(row, col + len - 1);
  for (size_t i = 0; i < len; ++i) {
    VirtualViewElement& vve = virtualchars[row][col + i];
    if (vve.getClearIteration() != currentcleariteration) {
      vve.clear();
    }
    if (bold) {
      vve.setBoldOn(currentcleariteration, currentredrawiteration);
    }
    else {
      vve.setBoldOff(currentcleariteration, currentredrawiteration);
    }
    if (vve.isModified()) {
      modifiedchars.emplace_back(row, col + i);
    }
  }
  if (col > maxrenderedcol) {
    maxrenderedcol = col;
  }
  if (row > maxrenderedrow) {
    maxrenderedrow = row;
  }
}

bool VirtualView::tryFocus(unsigned int row, unsigned int col, unsigned int len) {
  return true;
}

void VirtualView::resize(unsigned int row, unsigned int col) {
  realchars.resize(row);
  for (auto& cols : realchars) {
    cols.resize(col);
  }
  realrows = row;
  realcols = col;
  if (virtualchars.empty()) {
    virtualResize(row, col);
  }
  unsigned int actualrealrows = getActualRealRows();
  unsigned int actualrealcols = getActualRealCols();
  if (currentviewrow > 0 && currentviewrow + actualrealrows > maxrenderedrow + 1) {
    int newcurrentviewrow = static_cast<int>(maxrenderedrow + 1) - actualrealrows;
    if (newcurrentviewrow < 0) {
      newcurrentviewrow = 0;
    }
    currentviewrow = newcurrentviewrow;
  }
  if (currentviewcol > 0 && currentviewcol + actualrealcols > maxrenderedcol + 1) {
    int newcurrentviewcol = static_cast<int>(maxrenderedcol + 1) - actualrealcols;
    if (newcurrentviewcol < 0) {
      newcurrentviewcol = 0;
    }
    currentviewcol = newcurrentviewcol;
  }
  setRedrawAll();
}

void VirtualView::render() {
  if (redrawall) {
    renderer.erase();
  }
  unsigned int actualrealrows = getActualRealRows();
  unsigned int actualrealcols = getActualRealCols();
  for (std::set<std::pair<unsigned int, unsigned int>>::iterator it = renderedchars.begin(); it != renderedchars.end(); ++it) {
    unsigned int realrow = it->first;
    unsigned int realcol = it->second;
    if (realrow >= actualrealrows || realcol >= actualrealcols) {
      continue;
    }
    int virtrow = realrow + currentviewrow;
    int virtcol = realcol + currentviewcol;
    VirtualViewElement& vvve = virtualchars[virtrow][virtcol];
    VirtualViewElement& rvve = realchars[realrow][realcol];
    bool clear = vvve.getClearIteration() != currentcleariteration;
    if (clear) {
      rvve.clear();
      vvve.clear();
      if (rvve.isModified()) {
        rvve.render();
        vvve.render();
        renderer.printChar(realrow, realcol, rvve.getChar(), rvve.getHighlight(), rvve.getBold(), rvve.getColor());
      }
    }
    if (redrawall) {
      rvve.render();
      vvve.render();
      renderer.printChar(realrow, realcol, rvve.getChar(), rvve.getHighlight(), rvve.getBold(), rvve.getColor());
    }
  }
  for (const std::pair<unsigned int, unsigned int>& chr : modifiedchars) {
    unsigned int virtrow = chr.first;
    unsigned int virtcol = chr.second;
    if ((virtrow < currentviewrow || virtrow >= currentviewrow + actualrealrows) ||
        (virtcol < currentviewcol || virtcol >= currentviewcol + actualrealcols))
    {
      continue;
    }
    unsigned int realrow = virtrow - currentviewrow;
    unsigned int realcol = virtcol - currentviewcol;
    renderedchars.insert(std::pair<unsigned int, unsigned int>(realrow, realcol));
    VirtualViewElement& vvve = virtualchars[virtrow][virtcol];
    VirtualViewElement& rvve = realchars[realrow][realcol];
    if (redrawall || vvve.isModified()) { // might be from the renderedchars loop above
      rvve.set(vvve.getChar(), vvve.getHighlight(), vvve.getBold(), vvve.getColor(), currentcleariteration, currentredrawiteration);
    }
    if (redrawall || rvve.isModified() || (realrow == realrows - 1 && !horizontalSlider()) || (realcol == realcols - 1 && !verticalSlider())) {
      vvve.render();
      rvve.render();
      renderer.printChar(realrow, realcol, rvve.getChar(), rvve.getHighlight(), rvve.getBold(), rvve.getColor());
    }
  }
  redrawall = false;
  modifiedchars.clear();
  renderVerticalSlider();
  renderHorizontalSlider();
}

void VirtualView::reset() {
  setRedrawAll();
  currentviewrow = 0;
  currentviewcol = 0;
  virtualResize(realrows, realcols);
}

void VirtualView::clear() {
  currentcleariteration++;
  modifiedchars.clear();
  maxrenderedrow = 0;
  maxrenderedcol = 0;
}

void VirtualView::expandIfNeeded(unsigned int row, unsigned int col) {
  unsigned int maxrows = row + 1 > virtualrows ? row + 1 : virtualrows;
  unsigned int maxcols = col + 1 > virtualcols ? col + 1 : virtualcols;
  if (maxrows > virtualrows || maxcols > virtualcols) {
    virtualResize(maxrows, maxcols);
  }
}

void VirtualView::virtualResize(unsigned int row, unsigned int col) {
  virtualchars.resize(row);
  for (auto& cols : virtualchars) {
    cols.resize(col);
  }
  virtualrows = row;
  virtualcols = col;
  for (std::set<std::pair<unsigned int, unsigned int>>::iterator it = renderedchars.begin(); it != renderedchars.end();) {
    if (it->first >= virtualrows || it->second >= virtualcols) {
      it = renderedchars.erase(it);
    }
    else {
      ++it;
    }
  }
  for (std::list<std::pair<unsigned int, unsigned int>>::iterator it = modifiedchars.begin(); it != modifiedchars.end();) {
    if (it->first >= virtualrows || it->second >= virtualcols) {
      it = modifiedchars.erase(it);
    }
    else {
      ++it;
    }
  }
  redrawall = true;
}

void VirtualView::renderVerticalSlider() {
  unsigned int slidersize = 0;
  unsigned int sliderstart = 0;
  unsigned int spanlength = getActualRealRows();
  unsigned int totalspan = maxrenderedrow + 1;
  unsigned int ypos = 0;
  unsigned int xpos = realcols - 1;
  if (currentviewrow + spanlength > totalspan) {
    totalspan = currentviewrow + spanlength;
  }
  if (totalspan > spanlength) {
    slidersize = (spanlength * spanlength) / totalspan;
    sliderstart = (spanlength * currentviewrow) / totalspan;
    if (slidersize == 0) {
      slidersize++;
    }
    if (slidersize == spanlength) {
      slidersize--;
    }
    if (sliderstart + slidersize > spanlength || currentviewrow + spanlength >= totalspan) {
      sliderstart = spanlength - slidersize;
    }
    for (unsigned int i = 0; i < spanlength; i++) {
      if (i >= sliderstart && i < sliderstart + slidersize) {
        drawSliderChar(i + ypos, xpos, ' ', true);
      }
      else {
        drawSliderChar(i + ypos, xpos, BOX_VLINE);
      }
    }
    if (spanlength == 1) {
      drawSliderChar(ypos, xpos, BOX_CROSS);
    }
  }
}

void VirtualView::renderHorizontalSlider() {
  unsigned int slidersize = 0;
  unsigned int sliderstart = 0;
  unsigned int spanlength = getActualRealCols();
  unsigned int totalspan = maxrenderedcol + 1;
  unsigned int ypos = realrows - 1;
  unsigned int xpos = 0;
  if (currentviewcol + spanlength > totalspan) {
    totalspan = currentviewcol + spanlength;
  }
  if (totalspan > spanlength) {
    slidersize = (spanlength * spanlength) / totalspan;
    sliderstart = (spanlength * currentviewcol) / totalspan;
    if (slidersize == 0) {
      slidersize++;
    }
    if (slidersize == spanlength) {
      slidersize--;
    }
    if (sliderstart + slidersize > spanlength || currentviewcol + spanlength >= totalspan) {
      sliderstart = spanlength - slidersize;
    }
    for (unsigned int i = 0; i < spanlength; i++) {
      if (i >= sliderstart && i < sliderstart + slidersize) {
        drawSliderChar(ypos, i + xpos, 0x25A0);

      }
      else {
        drawSliderChar(ypos, i + xpos, BOX_HLINE);
      }
    }
    if (spanlength == 1) {
      drawSliderChar(ypos, xpos, BOX_CROSS);
    }
  }
}

void VirtualView::drawSliderChar(unsigned int row, unsigned int col, unsigned int c, bool highlight) {
  VirtualViewElement& rvve = realchars[row][col];
  VirtualViewElement& vvve = virtualchars[row][col];
  rvve.set(c, highlight, false, encodeColorRepresentation(), currentcleariteration, currentredrawiteration);
  vvve.set(c, highlight, false, encodeColorRepresentation(), currentcleariteration, currentredrawiteration);
  rvve.render();
  vvve.render();
  renderedchars.insert(std::pair<unsigned int, unsigned int>(row, col));
  renderer.printChar(row, col, c, highlight);
}

bool VirtualView::horizontalSlider() const {
  bool toowide = maxrenderedcol >= realcols;
  bool toohigh = maxrenderedrow >= realrows;
  if (toohigh && !toowide) {
    toowide = maxrenderedcol >= realcols - 1;
  }
  return toowide;
}

bool VirtualView::verticalSlider() const {
  bool toowide = maxrenderedcol >= realcols;
  bool toohigh = maxrenderedrow >= realrows;
  if (toowide && !toohigh) {
    toohigh = maxrenderedrow >= realrows - 1;
  }
  return toohigh;
}

bool VirtualView::goUp() {
  if (currentviewrow == 0) {
    return false;
  }
  int newcurrentviewrow = static_cast<int>(currentviewrow) - VERTICAL_STEP;
  if (newcurrentviewrow < 0) {
    newcurrentviewrow = 0;
  }
  currentviewrow = newcurrentviewrow;
  setRedrawAll();
  return true;
}

bool VirtualView::goDown() {
  unsigned int actualrealrows = getActualRealRows();
  if (currentviewrow + actualrealrows >= maxrenderedrow + 1) {
    return false;
  }
  unsigned int newcurrentviewrow = currentviewrow + VERTICAL_STEP;
  if (newcurrentviewrow + actualrealrows > maxrenderedrow + 1) {
    newcurrentviewrow = maxrenderedrow + 1 - actualrealrows;
  }
  currentviewrow = newcurrentviewrow;
  setRedrawAll();
  return true;
}

bool VirtualView::goLeft() {
  if (currentviewcol == 0) {
    return false;
  }
  int newcurrentviewcol = static_cast<int>(currentviewcol) - HORIZONTAL_STEP;
  if (newcurrentviewcol < 0) {
    newcurrentviewcol = 0;
  }
  currentviewcol = newcurrentviewcol;
  setRedrawAll();
  return true;
}

bool VirtualView::goRight() {
  unsigned int actualrealcols = getActualRealCols();
  if (currentviewcol + actualrealcols >= maxrenderedcol + 1) {
    return false;
  }
  unsigned int newcurrentviewcol = currentviewcol + HORIZONTAL_STEP;
  if (newcurrentviewcol + actualrealcols > maxrenderedcol + 1) {
    newcurrentviewcol = maxrenderedcol + 1 - actualrealcols;
  }
  currentviewcol = newcurrentviewcol;
  setRedrawAll();
  return true;
}

bool VirtualView::pageDown() {
  unsigned int actualrealrows = getActualRealRows();
  if (currentviewrow + actualrealrows >= maxrenderedrow + 1) {
    return false;
  }
  unsigned int newcurrentviewrow = currentviewrow + (actualrealrows / 2);
  if (newcurrentviewrow + actualrealrows > maxrenderedrow + 1) {
    newcurrentviewrow = maxrenderedrow + 1 - actualrealrows;
  }
  currentviewrow = newcurrentviewrow;
  setRedrawAll();
  return true;
}

bool VirtualView::pageUp() {
  unsigned int actualrealrows = getActualRealRows();
  if (currentviewrow == 0) {
    return false;
  }
  int newcurrentviewrow = static_cast<int>(currentviewrow) - (actualrealrows / 2);
  if (newcurrentviewrow < 0) {
    newcurrentviewrow = 0;
  }
  currentviewrow = newcurrentviewrow;
  setRedrawAll();
  return true;
}

bool VirtualView::viewPosition(unsigned int row, unsigned int col, unsigned int length) {
  unsigned int actualrealrows = getActualRealRows();
  unsigned int actualrealcols = getActualRealCols();
  unsigned int pagerows = actualrealrows / 2;
  bool changed = false;
  if (row < currentviewrow || row >= currentviewrow + actualrealrows) {
    if (row < pagerows) {
      currentviewrow = 0;
    }
    else {
      currentviewrow = row - pagerows;
    }
    changed = true;
  }
  if (currentviewrow + actualrealrows > maxrenderedrow + 1 && maxrenderedrow + 1 >= actualrealrows) {
    currentviewrow = maxrenderedrow + 1 - actualrealrows;
    if (currentviewrow > row) {
      currentviewrow = row;
    }
    changed = true;
  }
  if (col < currentviewcol || col + length >= currentviewcol + actualrealcols) {
    if (length > actualrealcols) {
      currentviewcol = col;
    }
    else {
      int newcurrentviewcol = static_cast<int>(col) - (actualrealcols - length) / 2;
      if (newcurrentviewcol < 0) {
        newcurrentviewcol = 0;
      }
      currentviewcol = newcurrentviewcol;
    }
    changed = true;
  }
  if (currentviewcol + actualrealcols > maxrenderedcol + 1 && maxrenderedcol + 1 >= actualrealcols) {
    currentviewcol = maxrenderedcol + 1 - actualrealcols;
    if (currentviewcol > col) {
      currentviewcol = col;
    }
    changed = true;
  }
  if (changed) {
    setRedrawAll();
  }
  return changed;
}

void VirtualView::setRedrawAll() {
  currentredrawiteration++;
  renderedchars.clear();
  modifiedchars.clear();
  redrawall = true;
}

bool VirtualView::isClear(unsigned int row, unsigned int col, unsigned int len) const {
  if (row >= virtualrows) {
    return true;
  }
  for (unsigned int i = col; i < col + len; ++i) {
    if (col >= virtualcols) {
      return true;
    }
    const VirtualViewElement& vve = virtualchars.at(row).at(i);
    if (!(vve.isClear() || currentredrawiteration != vve.getRedrawIteration() || currentcleariteration != vve.getClearIteration())) {
      return false;
    }
  }
  return true;
}

unsigned int VirtualView::getActualRealRows() const {
  return realrows - (horizontalSlider() ? 1 : 0);
}

unsigned int VirtualView::getActualRealCols() const {
  return realcols - (verticalSlider() ? 1 : 0);
}

void VirtualView::moveCursor(unsigned int row, unsigned int col) {
  renderer.moveCursor(row - currentviewrow, col - currentviewcol);
}
