#pragma once

#include <string>

enum BrowseScreenActionOp {
  BROWSESCREENACTION_CLOSE,
  BROWSESCREENACTION_SITE,
  BROWSESCREENACTION_HOME,
  BROWSESCREENACTION_NOOP,
  BROWSESCREENACTION_CAUGHT,
  BROWSESCREENACTION_CHDIR
};

class BrowseScreenAction {
public:
  BrowseScreenAction() : op(BROWSESCREENACTION_NOOP) { }
  BrowseScreenAction(BrowseScreenActionOp op) : op(op) { }
  BrowseScreenAction(BrowseScreenActionOp op, std::string arg) : op(op), arg(arg) { }
  BrowseScreenActionOp getOp() const {
    return op;
  }
  std::string getArg() const {
    return arg;
  }
private:
  BrowseScreenActionOp op;
  std::string arg;
};
