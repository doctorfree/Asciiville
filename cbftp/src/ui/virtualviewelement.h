#pragma once

class VirtualViewElement {
public:
  VirtualViewElement();
  unsigned int getChar() const;
  bool getHighlight() const;
  bool getBold() const;
  int getColor() const;
  bool isModified() const;
  int getClearIteration() const;
  int getRedrawIteration() const;
  bool isClear() const;
  void set(unsigned int c, bool highlight, bool bold, int color, int cleariteration, int redrawiteration);
  void setHighlightOn(int cleariteration, int redrawiteration);
  void setHighlightOff(int cleariteration, int redrawiteration);
  void setColor(int color, int cleariteration, int redrawiteration);
  void setBoldOn(int cleariteration, int redrawiteration);
  void setBoldOff(int cleariteration, int redrawiteration);
  void render();
  void clear();
private:
  unsigned int c;
  unsigned int newc;
  bool highlight;
  bool newhighlight;
  bool bold;
  bool newbold;
  int cleariteration;
  int redrawiteration;
  int newcleariteration;
  int newredrawiteration;
  int color;
  int newcolor;
};
