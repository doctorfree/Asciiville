#include "legendprinterkeybinds.h"

#include "ui.h"

LegendPrinterKeybinds::LegendPrinterKeybinds(Ui* ui) : ui(ui), offset(0), staticcount(0) {

}

bool LegendPrinterKeybinds::print() {
  unsigned int printpos = 4;
  if (text.length() > 0) {
    if (ui->getLegendMode() == LEGEND_SCROLLING) {
      std::string scrollingtext = text + "  ::  ";
      unsigned int textlen = scrollingtext.length();
      unsigned int internalpos = printpos - offset++;
      if (offset >= textlen) offset = 0;
      while (printpos < col - 4) {
        while (printpos - internalpos < textlen && printpos < col - 4) {
          ui->getRenderer().printChar(1, printpos, scrollingtext[printpos - internalpos], false, false, -1, Window::LEGEND);
          ++printpos;
        }
        internalpos = printpos;
      }
    }
    else if (ui->getLegendMode() == LEGEND_STATIC) {
      if (staticcount++ > 20) { // 5 seconds
        staticcount = 0;
        if (text.length() - offset > col - 8) {
          size_t nextoffset = text.rfind(" - ", offset + col - 8);
          if (nextoffset != std::string::npos) {
            offset = nextoffset + 3;
          }
          else {
            offset += col - 8;
          }
        }
        else if (offset) {
          offset = 0;
        }
      }
      unsigned int textpos = offset;
      while (printpos < col - 4) {
        if (textpos >= text.length()) {
          ui->getRenderer().printChar(1, printpos, ' ', false, false, -1, Window::LEGEND);
        }
        else {
          ui->getRenderer().printChar(1, printpos, text[textpos], false, false, -1, Window::LEGEND);
        }
        ++printpos;
        ++textpos;
      }
    }
  }
  return true;
}

void LegendPrinterKeybinds::setText(const std::string & text) {
  this->text = text;
  offset = 0;
  staticcount = 0;
}
