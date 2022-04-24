#pragma once

#include <string>

class RequestCallback;

class SiteLogicRequest {
private:
  int requestid;
  int connid;
  int requesttype;
  std::string data;
  std::string data2;
  int numdata;
  RequestCallback* cb;
  void* ptrdata;
public:
  SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, void* ptrdata);
  SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, int numdata);
  SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string& data);
  SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string& data, int numdata);
  SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string& data, const std::string& data2);
  SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string& data, const std::string& data2, int numdata);
  int getId() const;
  int getType() const;
  std::string getData() const;
  std::string getData2() const;
  int getNumData() const;
  void setConnId(int);
  int getConnId() const;
  RequestCallback* getCallback() const;
  void* getPtrData() const;
};
