#pragma once

class UIBase {
public:
  UIBase();
  virtual ~UIBase();
  static UIBase * instance();
  virtual bool init() = 0;
  virtual void backendPush() = 0;
  virtual void kill() = 0;
  virtual void notify() = 0;
};
