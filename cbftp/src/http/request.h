#pragma once

#include <map>
#include <string>
#include <vector>

#include "message.h"

namespace http {

class Request : public Message {
public:
  /* Internally used by RequestParser. */
  Request();

  /* Create a HTTP request.
   * method: GET, POST, PUT, HEAD etc
   * path: the target path of the request
   * minorVersion: the HTTP version, HTTP/1.x
   * protocol is only used by classes that derive from this class.
   */
  Request(const std::string& method, const std::string& path, int minorVersion = 1);

  /* Serialize into a HTTP request message. */
  std::vector<char> serialize() const override;

  std::string getMethod() const;
  std::string getPath() const;
  std::string getPathWithQueryParams() const;

  void setMethod(const std::string& method);
  void setPath(const std::string& path);
  void addQueryParam(const std::string& queryparam, const std::string& value);
  bool hasQueryParam(const std::string& queryparam) const;
  void removeQueryParam(const std::string& queryparam);
  std::string getQueryParamValue(const std::string& queryparam) const;
  std::map<std::string, std::string> getQueryParams() const;

private:
  void parseQueryParams(const std::string& queryparams);
  void generatePathWithQueryParams();
  std::string method;
  std::string path;
  std::string pathwithqueryparams;
  std::map<std::string, std::string> queryparams;
};
} // namespace http
