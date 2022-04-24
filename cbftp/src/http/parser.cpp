#include "parser.h"

#include <cstring>
#include <vector>

#include "message.h"

namespace {
const size_t maxheaderslen = 8192;
const size_t maxbodylen = 1048576; // 1 MiB

enum PicoReturn
{
    NEED_MORE_DATA = -2
};

int parseContentLength(const std::string& str)
{
    try {
        return std::stoi(str);
    }
    catch (std::exception& e) {
        return 0;
    }
}
} // namespace

namespace http
{

Result::Result()
    : Result(State::UNKNOWN)
{
}

Result::Result(State state, const char* data, size_t datalen, bool complete)
    : state(state)
    , data(data)
    , datalen(datalen)
    , complete(complete)
{
}

Parser::Parser() : headers(maxheaderslen), headerslen(0), bodylen(0), contentlen(0), inbody(false) {
}

Parser::~Parser() {

}

Result Parser::feed(const char* data, size_t datalen) {
  if (!inbody) {
    // Expecting at most kMaxHeadersLength bytes of headers.
    size_t headersleftlen = maxheaderslen - headerslen;
    if (datalen < headersleftlen) {
      headersleftlen = datalen;
    }
    size_t prevlen = headerslen;
    memcpy(&headers[headerslen], data, headersleftlen);
    headerslen += headersleftlen;
    int nbytes = parse(&headers[0], headerslen, prevlen);
    if (nbytes > 0) {
      Result res = Result(State::HEADERS, data, nbytes - prevlen, contentlen == 0);
      if (res.complete) {
        // No body expected, reset.
        clear();
      }
      else {
        inbody = true;
      }
      return res;
    }
    else if (headerslen >= maxheaderslen) {
      // More data might be expected but since buffer is already full, error out.
      clear();
      return Result(State::URI_TOO_LONG);
    }
    else if (nbytes == NEED_MORE_DATA){
      return Result(State::PARTIAL);
    }
    else {
      clear();
      return Result(State::PARSING_FAILED);
    }
  }

  size_t prevlen = bodylen;
  bodylen += datalen;
  if (bodylen > maxbodylen) {
    clear();
    return Result(State::BODY_TOO_LONG);
  }
  else if (bodylen > contentlen){
    size_t remaininglen = contentlen - prevlen;
    clear();
    return Result(State::BODY, data, remaininglen, true);
  }
  else if (bodylen < contentlen) {
    return Result(State::BODY, data, datalen, false);
  }
  clear();
  return Result(State::BODY, data, datalen, true);
}

void Parser::clear() {
    headerslen = 0;
    bodylen = 0;
    contentlen = 0;
    inbody = false;
}

void Parser::parseHeaders(Message& msg, struct phr_header* headers, size_t numheaders) {
    for (unsigned int i = 0; i < numheaders; i++)
    {
        if (headers[i].name_len == 14 && !strncmp(headers[i].name, "Content-Length", 14))
        {
            contentlen = parseContentLength(std::string(headers[i].value, headers[i].value_len));
        }
        msg.addHeader(std::string(headers[i].name, headers[i].name_len),
                      std::string(headers[i].value, headers[i].value_len));
    }
}
} // namespace http
