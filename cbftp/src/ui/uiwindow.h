#pragma once

#include <string>
#include <list>

#include "keybinds.h"

class Ui;
class VirtualView;

class UIWindow {
protected:
  bool isTop() const;
  std::string name;
  unsigned int row;
  unsigned int col;
  bool autoupdate;
  bool expectbackendpush;
  Ui * ui;
  VirtualView* vv;
  KeyBinds keybinds;
  bool allowimplicitgokeybinds;
public:
  void init(unsigned int row, unsigned int col);
  UIWindow(Ui* ui, const std::string& name);
  virtual ~UIWindow();
  virtual void redraw() = 0;
  void resize(unsigned int row, unsigned int col);
  virtual void update();
  virtual void command(const std::string& command);
  virtual void command(const std::string& command, const std::string& arg);
  virtual std::string getInfoLabel() const;
  virtual std::string getInfoText() const;
  virtual std::string getLegendText() const;
  virtual std::basic_string<unsigned int> getWideInfoLabel() const;
  virtual std::basic_string<unsigned int> getWideInfoText() const;
  virtual std::basic_string<unsigned int> getWideLegendText() const;
  bool keyPressedBase(unsigned int);
  virtual bool keyPressed(unsigned int);
  bool autoUpdate() const;
  bool expectBackendPush() const;
  virtual bool keyUp();
  virtual bool keyDown();
};
