#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace http {

/* Base class for the common parts of HTTPRequests and HTTPResponses. */
class Message {
public:
  explicit Message(const std::string& protocol = "HTTP/1.1");
  virtual ~Message();

  /* Add header or append to already existing header through comma separation. */
  void appendHeader(const std::string& header, const std::string& value);

  /* Add header or overwrite existing header value. */
  void addHeader(const std::string& header, const std::string& value);

  bool hasHeader(const std::string& header) const;

  void removeHeader(const std::string& header);

  std::string getHeaderValue(const std::string& header) const;

  const std::list<std::string>& getHeaderValues(const std::string& header) const;

  size_t getBodySize() const;

  std::shared_ptr<std::vector<char>> getBody() const;

  /* Set optional body, used as part of the serialized representation. */
  void setBody(const std::vector<char>& body);

  /* Serialize into a HTTP stream message */
  virtual std::vector<char> serialize() const = 0;

protected:
  /* Called by serialize() in subclasses */
  std::vector<char> serializeCommon(const std::string& startline) const;

  std::string protocol;

private:
  struct HeaderValues {
    explicit HeaderValues(const std::string& name);
    HeaderValues() = default;

    std::string name;
    std::list<std::string> values;
  };
  std::map<std::string, HeaderValues> headers;
  std::shared_ptr<std::vector<char>> body;
};
} // namespace http
