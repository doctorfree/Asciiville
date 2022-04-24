#pragma once

#include "parser.h"
#include "request.h"

namespace http {

class RequestParser : public Parser {
public:
  using Parser::Parser;
  const Request& getRequest() const;

private:
  int parse(const char* data, size_t datalen, size_t prevdatalen) override;
  Request request;
};
} // namespace http
