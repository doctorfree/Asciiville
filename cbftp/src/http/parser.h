#pragma once

#include <string>
#include <vector>

#include "../ext/picohttpparser.h"

namespace http {

class Message;

enum class State {
  BODY = 0,
  HEADERS = 1,
  PARSING_FAILED = -1,
  PARTIAL = -2,
  BODY_TOO_LONG = -3,
  URI_TOO_LONG = -4,
  UNKNOWN = -5
};

struct Result {
  Result();
  Result(State state, const char* data = nullptr, size_t datalen = 0, bool complete = false);
  State state;
  const char* data;
  size_t datalen;
  bool complete;
};

#define MAX_HEADERS 100

/* Base class for the common parts of HTTP request and response parsing. */
class Parser {
public:
  explicit Parser();
  virtual ~Parser();

  /* Feed incoming data from a socket into the parser. Returns false if a parsing
   * error occurred.
   */
  Result feed(const char* data, size_t datalen);

  /* Clear the parsing buffer. */
  void clear();

protected:
    /* Called by parse() in subclasses. */
    void parseHeaders(Message& msg, struct phr_header* headers, size_t numheaders);

private:
    virtual int parse(const char* data, size_t datalen, size_t prevdatalen = 0) = 0;
    std::vector<char> headers;
    size_t headerslen;
    size_t bodylen;
    size_t contentlen;
    bool inbody;
};
} // namespace http
