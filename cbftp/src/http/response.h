#pragma once

#include <string>
#include <vector>

#include "message.h"

namespace http {

class Response : public Message
{
public:
  /* Internally used by ResponseParser. */
  Response();

  /* Create a HTTP response.
   * code is the status code, i.e. 200, 404 etc
   * text is the status text, i.e. OK, File not found, etc
   * minorVersion is the HTTP version, HTTP/1.x
   * protocol is only used by classes that derive from this class.
   */
  Response(int code, const std::string& text, int minorversion = 1);

  /* Create a HTTP response and infer the status text from the given code. */
  Response(int code, int minorversion = 1);

  /* Serialize into a HTTP response message. */
  std::vector<char> serialize() const override;

  int getStatusCode() const;
  std::string getStatusText() const;
  void setStatusCode(int code);

private:
  int statuscode;
  std::string statustext;
};
} // namespace http
