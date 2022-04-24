#include "httpconn.h"

#include <cassert>
#include <regex>

#include "core/iomanager.h"
#include "core/logger.h"
#include "http/request.h"

#include "globalcontext.h"
#include "eventlog.h"
#include "restapi.h"

namespace {
const size_t BODY_BUFFER_INIT_CAPACITY = 64;
const size_t MAX_SEND_BLOCK = 65536;
} // namespace

HTTPConn::HTTPConn() : addrfam(Core::AddressFamily::IPV4), tls(true), nextrequestid(0),
    state(HTTPConnState::DISCONNECTED), sockid(-1), paused(false), latestrequestidresponded(-1)
{
}

HTTPConn::~HTTPConn() {
  if (state != HTTPConnState::DISCONNECTED) {
    global->getIOManager()->closeSocket(sockid);
    deactivate();
  }
}

bool HTTPConn::activate(int sockid, Core::AddressFamily addrfam, bool tls) {
  if (active()) {
      return false;
  }
  this->sockid = sockid;
  this->paused = false;
  this->addrfam = addrfam;
  this->tls = tls;
  state = HTTPConnState::ACTIVE;
  nextrequestid = 0;
  latestrequestidresponded = -1;
  parser.clear();
  global->getIOManager()->registerTCPServerClientSocket(this, sockid, !tls);
  sourceaddress = global->getIOManager()->getSocketAddress(sockid);
  global->getEventLog()->log("HTTPConn", "New client connection", Core::LogLevel::DEBUG);
  if (tls) {
    global->getIOManager()->negotiateSSLAccept(sockid);
    global->getEventLog()->log("HTTPConn", "Performing server-side TLS negotiation...", Core::LogLevel::DEBUG);
  }
  return true;
}

void HTTPConn::FDSSLSuccess(int sockid, const std::string& cipher) {
  global->getEventLog()->log("HTTPConn", "Server-side TLS negotiation successful.", Core::LogLevel::DEBUG);
}

void HTTPConn::FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  global->getEventLog()->log("HTTPConn", "Disconnected: " + details, Core::LogLevel::DEBUG);
  deactivate();
}

bool HTTPConn::active() {
  return state != HTTPConnState::DISCONNECTED;
}

void HTTPConn::respondAndClose(int statuscode) {
  http::Response response(statuscode);
  response.addHeader("Connection", "Close");
  std::vector<char> responsedata = response.serialize();
  global->getIOManager()->sendData(sockid, responsedata.data(), responsedata.size());
  global->getIOManager()->closeSocket(sockid);
  deactivate();
}

void HTTPConn::deactivate() {
  state = HTTPConnState::DISCONNECTED;
  responses.clear();
  sendbuffer.clear();
  global->getRestApi()->cancelOngoingSyncRequests(this);
}

void HTTPConn::FDData(int sockid, char* data, unsigned int datalen) {
  if (sockid != this->sockid) {
    return;
  }
  while (datalen > 0) {
    http::Result result = parser.feed(data, datalen);
    switch (result.state) {
      case http::State::HEADERS:
        break;
      case http::State::BODY: {
        bodybuffer.insert(bodybuffer.end(), result.data, result.data + result.datalen);
        break;
      }
      case http::State::PARSING_FAILED: {
        global->getEventLog()->log("HTTPConn", "Bad request from client. Closing connection.");
        respondAndClose(400);
        return;
      }
      case http::State::PARTIAL:
        // More data needed.
        return;
      case http::State::BODY_TOO_LONG: {
        global->getEventLog()->log("HTTPConn", "Request Body too long from client. Closing connection.");
        respondAndClose(413);
        return;
      }
      case http::State::URI_TOO_LONG: {
        global->getEventLog()->log("HTTPConn", "Request URI Too Long from client, closing connection.");
        respondAndClose(414);
        return;
      }
      default:
        global->getEventLog()->log("HTTPConn", "Unknown HTTP parser state: " + std::to_string(static_cast<int>(result.state)));
        return;
    }

    data += result.datalen;
    datalen -= result.datalen;
    if (result.complete) {
      http::Request request = parser.getRequest();
      if (!bodybuffer.empty()) {
        request.setBody(bodybuffer);
      }
      int requestid = nextrequestid++;
      global->getRestApi()->handleRequest(this, requestid, request);
      resetHttpParser();
    }
  }
}

void HTTPConn::sendErrorResponse(int requestid, int statuscode) {
  http::Response response(statuscode);
  response.addHeader("Content-Length", "0");
  queueResponse(requestid, response);
}

void HTTPConn::FDSendComplete(int) {
  paused = false;
  sendResponses();
}

void HTTPConn::queueResponse(int requestid, const http::Response& response) {
  bool inserted = false;
  for (auto it = responses.begin(); it != responses.end(); ++it) {
    if (it->first > requestid) {
      responses.insert(it, std::make_pair(requestid, response));
      inserted = true;
      break;
    }
  }
  if (!inserted) {
    responses.emplace_back(requestid, response);
  }
  sendResponses();
}

void HTTPConn::sendResponses() {
  sendFromBuffer();
  while (!paused && !responses.empty() && responses.front().first == latestrequestidresponded + 1) {
    sendResponse(responses.front().second);
    responses.pop_front();
  }
}

void HTTPConn::sendResponse(const http::Response& response) {
  http::Response newresponse(response);
  if (newresponse.getHeaderValue("Connection") == "Close") {
    newresponse.removeHeader("Connection");
    if (!newresponse.hasHeader("Content-Length")) {
      newresponse.addHeader("Content-Length", "0");
    }
  }
  std::vector<char> responsedata = newresponse.serialize();
  sendbuffer.insert(sendbuffer.end(), responsedata.begin(), responsedata.end());
  sendFromBuffer();
  ++latestrequestidresponded;
}

void HTTPConn::sendFromBuffer() {
  while (!paused && sendbuffer.size()) {
    size_t size = std::min(sendbuffer.size(), MAX_SEND_BLOCK);
    if (global->getIOManager()->sendData(sockid, sendbuffer.data(), size)) {
      paused = true;
    }
    sendbuffer.erase(sendbuffer.begin(), sendbuffer.begin() + size);
  }
}

void HTTPConn::requestHandled(int requestid, const http::Response& response) {
  assert(state != HTTPConnState::DISCONNECTED);
  queueResponse(requestid, response);
}

void HTTPConn::resetHttpParser() {
  bodybuffer.clear();
  bodybuffer.shrink_to_fit();
  bodybuffer.reserve(BODY_BUFFER_INIT_CAPACITY);
  bodybuffer.clear();
}
