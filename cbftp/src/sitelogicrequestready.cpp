#include "sitelogicrequestready.h"

SiteLogicRequestReady::SiteLogicRequestReady(int type, int requestid, void * data, bool status) :
  type(type),
  requestid(requestid),
  data(data),
  status(status) {
}

int SiteLogicRequestReady::getType() const {
  return type;
}

int SiteLogicRequestReady::getId() const {
  return requestid;
}

void * SiteLogicRequestReady::getData() const {
  return data;
}

bool SiteLogicRequestReady::getStatus() const {
  return status;
}
