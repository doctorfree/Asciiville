#include "chardraw.h"

#include "termint.h"

void CharDraw::init() {
  std::string init = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789<>";
  for (unsigned int i = 0; i < init.length(); i++) {
    charmap[init[i]] = std::vector<std::string>();
  }
  push('A', " AHHB ", "AdxxcB", "Vxabxv", "Vxcdxv", "Vxabxv", "ChDChD");
  push('B', "AHHHHB", "Vxabxv", "VxcdaD", "VxabcB", "Vxcdxv", "ChhhhD");
  push('C', "AHHHHB", "Vxabxv", "VxvChD", "VxvAHB", "Vxcdxv", "ChhhhD");
  push('D', "AHHHB ", "VxxxcB", "Vxabxv", "Vxcdxv", "VxxxaD", "ChhhD ");
  push('E', "AHHHHB", "VxahhD", "VxcHHB", "VxahhD", "VxcHHB", "ChhhhD");
  push('F', "AHHHHB", "VxahhD", "VxcHHB", "VxahhD", "Vxv   ", "ChD   ");
  push('G', " AHHHB", "AdahhD", "VxvAHB", "VxvCbv", "CbcHdv", " ChhhD");
  push('H', "AHBAHB", "VxvVxv", "Vxcdxv", "Vxabxv", "VxvVxv", "ChDChD");
  push('I', " AHHB ", " Vxxv ", " Vxxv ", " Vxxv ", " Vxxv ", " ChhD ");
  push('J', "   AHB", "   Vxv", "   Vxv", "AHBVxv", "Vxcdxv", "ChhhhD");
  push('K', "AHBAHB", "VxcdaD", "VxxaD ", "VxxcB ", "VxabcB", "ChDChD");
  push('L', "AHB   ", "Vxv   ", "Vxv   ", "Vxv   ", "VxcHHB", "ChhhhD");
  push('M', "AB  AB", "VcBAdv", "Vxcdxv", "Vababv", "VvCDVv", "CD  CD");
  push('N', "AHB AB", "VxcBVv", "Vxxcdv", "Vabxxv", "VvCbxv", "CD ChD");
  push('O', " AHHB ", "AdxxcB", "Vxabxv", "Vxcdxv", "CbxxaD", " ChhD ");
  push('P', "AHHHHB", "Vxabxv", "Vxcdxv", "VxahhD", "Vxv   ", "ChD   ");
  push('Q', " AHHB ", "AdabcB", "VxvVxv", "CbcdaD", " ChbcB", "   ChD");
  push('R', "AHHHHB", "Vxabxv", "Vxcdxv", "VabxaD", "VvCbcB", "CD ChD");
  push('S', "AHHHHB", "VxahhD", "VxcHHB", "Chhbxv", "AHHdxv", "ChhhhD");
  push('T', "AHHHHB", "ChbahD", "  Vv  ", "  Vv  ", "  Vv  ", "  CD  ");
  push('U', "AHBAHB", "VxvVxv", "VxvVxv", "VxvVxv", "Vxcdxv", "ChhhhD");
  push('V', "AHBAHB", "VxvVxv", "VxvVxv", "VxvVxv", "CbcdaD", " ChhD ");
  push('W', "AB  AB", "VvABVv", "Vcdcdv", "Vxabxv", "VaDCbv", "CD  CD");
  push('X', "AHBAHB", "CbcdaD", " CbaD ", " AdcB ", "AdabcB", "ChDChD");
  push('Y', "AHBAHB", "VxvVxv", "CbcdaD", " CbaD ", "  Vv  ", "  CD  ");
  push('Z', "AHHHHB", "ChbxaD", " AdaD ", "AdaD  ", "VxcHHB", "ChhhhD");
  push('a', "AHHHB ", "ChhbcB", "AHHdxv", "Vxabxv", "Cbcdxv", " ChhhD");
  push('b', "AHB   ", "Vxv   ", "VxcHHB", "Vxabxv", "Vxcdxv", "ChhhhD");
  push('c', "      ", " AHHHB", "AdahhD", "Vxv   ", "CbcHHB", " ChhhD");
  push('d', "   AHB", "   Vxv", "AHHdxv", "Vxabxv", "Vxcdxv", "ChhhhD");
  push('e', " AHHB ", "AdabcB", "Vxcdxv", "VxahhD", "CbcHHB", " ChhhD");
  push('f', "  AHHB", " AdxaD", "AdxxcB", "CbxxaD", " Vxxv ", " ChhD ");
  push('g', " AHHHB", "Adabxv", "Vxcdxv", "Chhhbv", "AHHHdv", "ChhhhD");
  push('h', "AHB   ", "Vxv   ", "VxcHHB", "Vxabxv", "VxvVxv", "ChDChD");
  push('i', " AHHB ", " ChhD ", " AHHB ", " Vxxv ", " Vxxv ", " ChhD ");
  push('j', "   AHB", "   ChD", "   AHB", "AHBVxv", "Vxcdxv", "ChhhhD");
  push('k', "AHB   ", "VxcHHB", "VxxahD", "VxxcB ", "VxabcB", "ChDChD");
  push('l', " AHHB ", " Vxxv ", " Vxxv ", " Vxxv ", " Vxxv ", " ChhD ");
  push('m', "      ", "AHBAHB", "Vxcdxv", "Vababv", "VvCDVv", "CD  CD");
  push('n', "      ", "AHHHB ", "VxxxcB", "Vahhbv", "Vv  Vv", "CD  CD");
  push('o', "      ", "AHHHHB", "Vxabxv", "VxvVxv", "Vxcdxv", "ChhhhD");
  push('p', "      ", "AHHHHB", "Vxabxv", "Vxcdxv", "VxahhD", "ChD   ");
  push('q', "AHHHHB", "Vxabxv", "Vxcdxv", "Chhbxv", "   Vxv", "   ChD");
  push('r', "      ", " AHHHB", "AdahhD", "Vxv   ", "Vxv   ", "ChD   ");
  push('s', "AHHHHB", "VxahhD", "VxcHHB", "Chhbxv", "AHHdxv", "ChhhhD");
  push('t', " AHHB ", " Vxxv ", "AdxxcB", "CbxxaD", " CbxcB", "  ChhD");
  push('u', "      ", "AHBAHB", "VxvVxv", "VxvVxv", "Vxcdxv", "ChhhhD");
  push('v', "      ", "AHBAHB", "VxvVxv", "VxvVxv", "CbcdaD", " ChhD ");
  push('w', "      ", "AB  AB", "VvABVv", "Vcdcdv", "Vxabxv", "ChDChD");
  push('x', "      ", "AHBAHB", "CbcdaD", " Vxxv ", "AdabcB", "ChDChD");
  push('y', "      ", "AHBAHB", "Vxcdxv", "Chhbxv", " AHdxv", " ChhhD");
  push('z', "      ", "AHHHHB", "ChbxaD", " AdaD ", "AdxcHB", "ChhhhD");
  push('0', " AHHB ", "AdabcB", "VxvVxv", "VxvVxv", "CbcdaD", " ChhD ");
  push('1', "  AHB ", " Adxv ", " Cbxv ", "  Vxv ", " AdxcB", " ChhhD");
  push('2', "AHHHB ", "ChhbcB", "  AdaD", " AdaD ", "AdxcHB", "ChhhhD");
  push('3', "AHHHB ", "ChhbcB", "AHHdxv", "Chhbxv", "AHHdaD", "ChhhD ");
  push('4', "AHBAHB", "VxvVxv", "Vxcdxv", "Chhbxv", "   Vxv", "   ChD");
  push('5', "AHHHHB", "VxahhD", "VxcHHB", "Chhbxv", "AHHdaD", "ChhhD ");
  push('6', " AHHHB", "AdahhD", "VxcHHB", "Vxabxv", "CbcdaD", " ChhD ");
  push('7', "AHHHHB", "Chhbxv", "  AdaD", " AdaD ", "AdaD  ", "ChD   ");
  push('8', "AHHHHB", "Vxabxv", "CbcdaD", "AdabcB", "Vxcdxv", "ChhhhD");
  push('9', "AHHHHB", "Vxabxv", "Vxcdxv", "Chhbxv", "   Vxv", "   ChD");
  push(' ', "      ", "      ", "      ", "      ", "      ", "      ");
  push('<', "   AHB", " AHdaD", "AdahD ", "CbcHB ", " ChbcB", "   ChD");
  push('>', "AHB   ", "CbcHB ", " ChbcB", " AHdaD", "AdahD ", "ChD   ");
}

std::string CharDraw::getCharLine(char c, unsigned int line) {
  std::map<char, std::vector<std::string> >::const_iterator it = charmap.find(c);
  if (it == charmap.end() || line >= it->second.size()) {
    return "      ";
  }
  return it->second[line];
}

int CharDraw::getMixedChar(int bgchar, int draw) {
  int c = 0;
  switch (draw) {
    case 'A':
      c = BOX_CORNER_TL;
      switch (bgchar) {
        case BOX_CORNER_BR:
        case BOX_VLINE_L:
        case BOX_HLINE_TOP:
        case BOX_CROSS:
          c = BOX_CROSS;
          break;
        case BOX_VLINE_R:
        case BOX_VLINE:
        case BOX_CORNER_BL:
          c = BOX_VLINE_R;
          break;
        case BOX_HLINE_BOT:
        case BOX_HLINE:
        case BOX_CORNER_TR:
          c = BOX_HLINE_BOT;
          break;
      }
      break;
    case 'a':
      c = BOX_CORNER_TL;
      break;
    case 'B':
      c = BOX_CORNER_TR;
      switch (bgchar) {
        case BOX_CORNER_BR:
        case BOX_VLINE:
        case BOX_VLINE_L:
          c = BOX_VLINE_L;
          break;
        case BOX_CORNER_TL:
        case BOX_HLINE:
        case BOX_HLINE_BOT:
          c = BOX_HLINE_BOT;
          break;
        case BOX_CROSS:
        case BOX_CORNER_BL:
        case BOX_HLINE_TOP:
        case BOX_VLINE_R:
          c = BOX_CROSS;
          break;
      }
      break;
    case 'b':
      c = BOX_CORNER_TR;
      break;
    case 'C':
      c = BOX_CORNER_BL;
      switch (bgchar) {
        case BOX_CORNER_BR:
        case BOX_HLINE:
        case BOX_HLINE_TOP:
          c = BOX_HLINE_TOP;
          break;
        case BOX_CORNER_TL:
        case BOX_VLINE_R:
        case BOX_VLINE:
          c = BOX_VLINE_R;
          break;
        case BOX_CORNER_TR:
        case BOX_CROSS:
        case BOX_HLINE_BOT:
        case BOX_VLINE_L:
          c = BOX_CROSS;
          break;
      }
      break;
    case 'c':
      c = BOX_CORNER_BL;
      break;
    case 'D':
      c = BOX_CORNER_BR;
      switch (bgchar) {
        case BOX_CORNER_TL:
        case BOX_CROSS:
        case BOX_VLINE_R:
        case BOX_HLINE_BOT:
          c = BOX_CROSS;
          break;
        case BOX_CORNER_BL:
        case BOX_HLINE:
        case BOX_HLINE_TOP:
          c = BOX_HLINE_TOP;
          break;
        case BOX_CORNER_TR:
        case BOX_VLINE_L:
        case BOX_VLINE:
          c = BOX_VLINE_L;
          break;
      }
      break;
    case 'd':
      c = BOX_CORNER_BR;
      break;
    case 'H':
      c = BOX_HLINE;
      switch (bgchar) {
        case BOX_CORNER_BR:
        case BOX_CORNER_BL:
        case BOX_HLINE_TOP:
        case BOX_CROSS:
        case BOX_VLINE:
        case BOX_VLINE_L:
        case BOX_VLINE_R:
          c = BOX_HLINE_TOP;
          break;
      }
      break;
    case 'h':
      c = BOX_HLINE;
      switch (bgchar) {
        case BOX_CORNER_TR:
        case BOX_CORNER_TL:
        case BOX_HLINE_BOT:
        case BOX_CROSS:
        case BOX_VLINE:
        case BOX_VLINE_L:
        case BOX_VLINE_R:
          c = BOX_HLINE_BOT;
          break;
      }
      break;
    case 'V':
      c = BOX_VLINE;
      switch (bgchar) {
        case BOX_CORNER_BR:
        case BOX_CORNER_TR:
        case BOX_VLINE_L:
        case BOX_CROSS:
        case BOX_HLINE:
        case BOX_HLINE_TOP:
        case BOX_HLINE_BOT:
          c = BOX_VLINE_L;
          break;
      }
      break;
    case 'v':
      c = BOX_VLINE;
      switch (bgchar) {
        case BOX_CORNER_BL:
        case BOX_CORNER_TL:
        case BOX_VLINE_R:
        case BOX_CROSS:
        case BOX_HLINE:
        case BOX_HLINE_TOP:
        case BOX_HLINE_BOT:
          c = BOX_VLINE_R;
          break;
      }
      break;
    case 'x':
      c = ' ';
      break;
  }
  return c;
}

void CharDraw::push(char c, std::string l1, std::string l2, std::string l3,
    std::string l4, std::string l5, std::string l6) {
  charmap[c].push_back(l1);
  charmap[c].push_back(l2);
  charmap[c].push_back(l3);
  charmap[c].push_back(l4);
  charmap[c].push_back(l5);
  charmap[c].push_back(l6);
}

std::map<char, std::vector< std::string > > CharDraw::charmap = std::map<char, std::vector< std::string > >();
