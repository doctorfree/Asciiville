#pragma once

#include <string>
#include <vector>
#include <map>

#define CHARDRAW_SIZE 6

// legend:
// top-left corner outside:     A
// top-left corner inside:      a
// top-right corner outside:    B
// top-right corner inside:     b
// bottom-left corner outside:  C
// bottom-left corner inside:   c
// bottom-right corner outside: D
// bottom-right corner inside:  d
// horizontal line top:         H
// horizontal line bottom:      h
// vertical line left:          V
// vertical line right:         v
// blank:                       x

class CharDraw {
private:
  static void push(char, std::string, std::string, std::string, std::string, std::string, std::string);
  static std::map<char, std::vector< std::string > > charmap;
public:
  static void init();
  static std::string getCharLine(char, unsigned int);
  static int getMixedChar(int, int);
};
