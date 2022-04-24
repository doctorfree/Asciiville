#include "misc.h"

#include "termint.h"
#include "virtualview.h"

void printSlider(VirtualView* vv, unsigned int row, unsigned int xpos, unsigned int totalspan, unsigned int currentviewspan) {
  printSlider(vv, row, 0, xpos, totalspan, currentviewspan);
}

void printSlider(VirtualView* vv, unsigned int row, unsigned int ypos, unsigned int xpos, unsigned int totalspan, unsigned int currentviewspan) {
  unsigned int slidersize = 0;
  unsigned int sliderstart = 0;
  unsigned int spanlength = row - ypos;
  if (currentviewspan + spanlength > totalspan) {
    totalspan = currentviewspan + spanlength;
  }
  if (totalspan > spanlength) {
    slidersize = (spanlength * spanlength) / totalspan;
    sliderstart = (spanlength * currentviewspan) / totalspan;
    if (slidersize == 0) {
      slidersize++;
    }
    if (slidersize == spanlength) {
      slidersize--;
    }
    if (sliderstart + slidersize > spanlength || currentviewspan + spanlength >= totalspan) {
      sliderstart = spanlength - slidersize;
    }
    for (unsigned int i = 0; i < spanlength; i++) {
      if (i >= sliderstart && i < sliderstart + slidersize) {
        vv->putChar(i + ypos, xpos, ' ', true);
      }
      else {
        vv->putChar(i + ypos, xpos, BOX_VLINE);
      }
    }
    if (spanlength == 1) {
      vv->putChar(ypos, xpos, BOX_CROSS);
    }
  }
}

bool adaptViewSpan(unsigned int & currentviewspan, unsigned int row, unsigned int position, unsigned int listsize) {
  unsigned int pagerows = row / 2;
  bool viewspanchanged = false;
  if (position < currentviewspan || position >= currentviewspan + row) {
    if (position < pagerows) {
      currentviewspan = 0;
    }
    else {
      currentviewspan = position - pagerows;
    }
    viewspanchanged = true;
  }
  if (currentviewspan + row > listsize && listsize >= row) {
    currentviewspan = listsize - row;
    if (currentviewspan > position) {
      currentviewspan = position;
    }
    viewspanchanged = true;
  }
  return viewspanchanged;
}

bool isYearEnd() {
  time_t rawtime = time(nullptr);
  struct tm* ti = localtime(&rawtime);
  return !ti->tm_yday || (ti->tm_mon == 11 && ti->tm_mday >= 24);
}
