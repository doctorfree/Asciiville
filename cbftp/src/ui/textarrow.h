#pragma once

#include <string>
#include <vector>

class TextArrow {
public:
  TextArrow();
  int getOption() const;
  std::string getOptionText() const;
  bool setOption(int);
  bool setOptionText(std::string);
  bool next();
  bool previous();
  std::string getVisual() const;
  void activate();
  void deactivate();
  bool isActive() const;
  void addOption(std::string, int);
  void clear();
private:
  bool active;
  std::vector<std::pair<int, std::string> > options;
  unsigned int currentpos;
  unsigned int maxlen;
};
