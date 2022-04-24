#include "commandhistory.h"

CommandHistory::CommandHistory() : pos(history.end()) {

}

bool CommandHistory::canBack() const {
  return pos != history.begin();
}

void CommandHistory::back() {
  pos--;
}

bool CommandHistory::forward() {
  if (pos != history.end()) {
    pos++;
    return true;
  }
  return false;
}

bool CommandHistory::current() const {
  return pos == history.end();
}

void CommandHistory::push(std::string text) {
  if (!history.size() || text != history.back()) {
    history.push_back(text);
  }
  pos = history.end();
}

std::string CommandHistory::get() const {
  if (pos == history.end()) {
    return currenttext;
  }
  return *pos;
}

void CommandHistory::setCurrent(std::string text) {
  currenttext = text;
}
