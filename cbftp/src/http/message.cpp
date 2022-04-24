#include "message.h"

namespace {

std::string join(const std::list<std::string>& items) {
  std::string joined;
  for (const std::string& item : items) {
    joined += (joined.empty() ? "" : ", ") + item;
  }
  return joined;
}

std::string normalizeHeader(const std::string& str) {
  std::string data(str);
  for (size_t i = 0; i < data.length(); ++i) {
    data[i] = std::tolower(data[i]);
  }
  return data;
}

const std::list<std::string> emptyreturnlist = std::list<std::string>();
} // namespace

namespace http {

Message::Message(const std::string& protocol) : protocol(protocol) {
}

Message::~Message() {
}

void Message::addHeader(const std::string& header, const std::string& value) {
  std::string key = normalizeHeader(header);
  auto it = headers.find(key);
  if (it == headers.end()) {
    headers.emplace(key, HeaderValues(header));
  }
  else {
    it->second.values.clear();
  }
  headers[key].values.push_back(value);
}

void Message::appendHeader(const std::string& header, const std::string& value) {
  const std::string key = normalizeHeader(header);
  auto it = headers.find(key);
  if (it == headers.end()) {
    headers.emplace(key, HeaderValues(header));
  }
  headers[key].values.push_back(value);
}

bool Message::hasHeader(const std::string& header) const {
  return headers.find(normalizeHeader(header)) != headers.end();
}

void Message::removeHeader(const std::string& header) {
  headers.erase(normalizeHeader(header));
}

std::string Message::getHeaderValue(const std::string& header) const {
  std::map<std::string, HeaderValues>::const_iterator it;
  it = headers.find(normalizeHeader(header));
  if (it != headers.end()) {
    return join(it->second.values);
  }
  return "";
}

const std::list<std::string>& Message::getHeaderValues(const std::string& header) const {
  std::map<std::string, HeaderValues>::const_iterator it;
  it = headers.find(normalizeHeader(header));
  if (it != headers.end()) {
    return it->second.values;
  }
  return emptyreturnlist;
}

size_t Message::getBodySize() const {
  if (!body) {
    return 0;
  }
  return body->size();
}

std::shared_ptr<std::vector<char>> Message::getBody() const {
  return body;
}

void Message::setBody(const std::vector<char>& body) {
  this->body = std::make_shared<std::vector<char>>(body);
  addHeader("Content-Length", std::to_string(body.size()));
}

std::vector<char> Message::serializeCommon(const std::string& startline) const {
  std::string concatdata = startline + "\r\n";
  for (const std::pair<const std::string, HeaderValues>& header : headers) {
    const HeaderValues& values = header.second;
    concatdata += values.name + ": " + join(values.values) + "\r\n";
  }
  concatdata += "\r\n";
  std::vector<char> data(concatdata.begin(), concatdata.end());
  if (body) {
    data.insert(data.end(), body->begin(), body->end());
  }
  return data;
}

Message::HeaderValues::HeaderValues(const std::string& name) :name(name) {
}
} // namespace http
