#pragma once

class LegendPrinter {
public:
  virtual ~LegendPrinter();
  virtual bool print() = 0;
  void setColumns(unsigned int col);
protected:
  unsigned int col;
};
