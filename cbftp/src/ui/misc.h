#pragma once

#include <utility>

class VirtualView;

void printSlider(VirtualView* vv, unsigned int, unsigned int, unsigned int, unsigned int);
void printSlider(VirtualView* vv, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
bool adaptViewSpan(unsigned int &, unsigned int, unsigned int, unsigned int);
bool isYearEnd();

constexpr int encodeColorRepresentation(int fgcolor = -1, int bgcolor = -1) {
  return (fgcolor + 2) * 10 + bgcolor + 2;
}

struct Color {
  int fgcolor;
  int bgcolor;
};

constexpr Color decodeColorRepresentation(int repr) {
  return Color{ repr / 10 - 2, repr % 10 - 2 };
}
