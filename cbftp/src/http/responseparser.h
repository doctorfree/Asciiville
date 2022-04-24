#pragma once

#include "parser.h"
#include "response.h"

namespace http {

class ResponseParser : public Parser
{
public:
  using Parser::Parser;
  Response getResponse();
private:
  int parse(const char* data, size_t datalen, size_t prevdatalen) override;
  Response response;
};
} // namespace http
