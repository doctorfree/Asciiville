#pragma once

#include <string>
#include <list>

class CommandHistory {
public:
  CommandHistory();
  bool canBack() const;
  void back();
  bool forward();
  bool current() const;
  void push(std::string);
  std::string get() const;
  void setCurrent(std::string);
private:
  std::list<std::string> history;
  std::string currenttext;
  std::list<std::string>::iterator pos;
};
