#include "request.h"

namespace http {

Request::Request() : Request("", "", 0) {
}

Request::Request(const std::string& method, const std::string& path, int minorversion)
    : Message("HTTP/1." + std::to_string(minorversion)), method(method), pathwithqueryparams(path)
{
  size_t queryParamSeparator = path.find("?");
  if (queryParamSeparator != std::string::npos) {
    this->path = path.substr(0, queryParamSeparator);
    parseQueryParams(path.substr(queryParamSeparator + 1));
  }
  else {
    this->path = path;
  }
}

void Request::parseQueryParams(const std::string& queryparams) {
  size_t pos = 0;
  while (true) {
    size_t valueseparator = queryparams.find("=", pos);
    if (valueseparator != std::string::npos) {
      std::string name = queryparams.substr(pos, valueseparator - pos);
      size_t paramseparator = queryparams.find("&", valueseparator);
      if (paramseparator != std::string::npos) {
        this->queryparams[name] = queryparams.substr(valueseparator + 1, paramseparator - valueseparator - 1);
        pos = paramseparator + 1;
      }
      else {
        this->queryparams[name] = queryparams.substr(valueseparator + 1);
        break;
      }
    }
    else {
      break;
    }
  }
}

std::vector<char> Request::serialize() const {
  return serializeCommon(method + " " + pathwithqueryparams + " " + protocol);
}

std::string Request::getMethod() const {
  return method;
}

std::string Request::getPath() const {
  return path;
}

std::string Request::getPathWithQueryParams() const {
  return pathwithqueryparams;
}

void Request::setMethod(const std::string& method) {
  this->method = method;
}

void Request::setPath(const std::string& path) {
  this->path = path;
  generatePathWithQueryParams();
}

void Request::addQueryParam(const std::string& queryparam, const std::string& value) {
  queryparams[queryparam] = value;
  generatePathWithQueryParams();
}

bool Request::hasQueryParam(const std::string& queryparam) const {
  return queryparams.find(queryparam) != queryparams.end();
}

void Request::removeQueryParam(const std::string& queryParam) {
  queryparams.erase(queryParam);
  generatePathWithQueryParams();
}

std::string Request::getQueryParamValue(const std::string& queryparam) const {
  std::map<std::string, std::string>::const_iterator it = queryparams.find(queryparam);
  if (it != queryparams.end()) {
    return it->second;
  }
  return "";
}

std::map<std::string, std::string> Request::getQueryParams() const {
  return queryparams;
}

void Request::generatePathWithQueryParams() {
  pathwithqueryparams = path;
  for (std::map<std::string, std::string>::const_iterator it = queryparams.begin(); it != queryparams.end(); ++it) {
    pathwithqueryparams.append((it == queryparams.begin() ? "?" : "&") + it->first + "=" + it->second);
  }
}
} // namespace EWHTTPParser
