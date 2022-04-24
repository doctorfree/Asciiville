#include "requestparser.h"

#include "../ext/picohttpparser.h"

#include "request.h"

namespace http {

const Request& RequestParser::getRequest() const {
  return request;
}

int RequestParser::parse(const char* data, size_t datalen, size_t prevdatalen) {
  const char* method;
  const char* path;
  int minorversion;
  struct phr_header headers[MAX_HEADERS];
  size_t numheaders = MAX_HEADERS;
  size_t methodlen, pathlen;
  int ret = phr_parse_request(data, datalen, &method, &methodlen, &path, &pathlen,
                              &minorversion, headers, &numheaders, prevdatalen);
  if (ret > 0) {
    request = Request(std::string(method, methodlen), std::string(path, pathlen), minorversion);
    parseHeaders(request, headers, numheaders);
  }
  return ret;
}
} // namespace http
