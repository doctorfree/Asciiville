#include "sitelogicrequest.h"

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, void* ptrdata) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  cb(cb),
  ptrdata(ptrdata)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, int numdata) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  numdata(numdata),
  cb(cb),
  ptrdata(nullptr)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string & data) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  cb(cb),
  ptrdata(nullptr)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string & data, int numdata) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  numdata(numdata),
  cb(cb),
  ptrdata(nullptr)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string & data, const std::string & data2) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  data2(data2),
  cb(cb),
  ptrdata(nullptr)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string & data, const std::string & data2, int numdata) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  data2(data2),
  numdata(numdata),
  cb(cb),
  ptrdata(nullptr)
{
}

int SiteLogicRequest::getId() const {
  return requestid;
}

int SiteLogicRequest::getType() const {
  return requesttype;
}

std::string SiteLogicRequest::getData() const {
  return data;
}

std::string SiteLogicRequest::getData2() const {
  return data2;
}

int SiteLogicRequest::getNumData() const {
  return numdata;
}
void SiteLogicRequest::setConnId(int id) {
  connid = id;
}

int SiteLogicRequest::getConnId() const {
  return connid;
}

RequestCallback* SiteLogicRequest::getCallback() const {
  return cb;
}

void* SiteLogicRequest::getPtrData() const {
  return ptrdata;
}
