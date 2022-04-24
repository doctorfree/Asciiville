#pragma once

class SiteLogicRequestReady {
private:
  int type;
  int requestid;
  void * data;
  bool status;
public:
  SiteLogicRequestReady(int, int, void *, bool);
  int getType() const;
  int getId() const;
  void * getData() const;
  bool getStatus() const;
};
