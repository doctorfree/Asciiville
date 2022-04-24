#pragma once

#include <list>
#include <string>
#include <vector>

#include "core/eventreceiver.h"
#include "core/types.h"
#include "http/requestparser.h"
#include "http/response.h"

#include "restapicallback.h"

namespace Core {
class IOManager;
}

namespace http {
class Request;
} // namespace http

class ConfigurationHandler;
class Logger;

enum class HTTPConnState
{
    DISCONNECTED,
    ACTIVE
};

class HTTPConn : private Core::EventReceiver, private RestApiCallback
{
public:
    HTTPConn();
    ~HTTPConn();

    // To be called from the connection handler
    bool activate(int sockid, Core::AddressFamily addrfam, bool tls);
private:
    // IO callbacks
    void FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
    void FDData(int sockid, char* data, unsigned int datalen) override;
    void FDSendComplete(int sockid) override;
    void FDSSLSuccess(int sockid, const std::string& cipher) override;

    // RestApi callback
    void requestHandled(int requestid, const http::Response& response) override;

    bool active();
    void deactivate();
    void sendResponse(const http::Response& response);
    void sendResponses();
    void resetHttpParser();
    void queueResponse(int requestid, const http::Response& response);
    void respondAndClose(int statuscode);
    void sendErrorResponse(int requestid, int statuscode);
    void sendFromBuffer();
    Core::AddressFamily addrfam;
    bool tls;
    int nextrequestid;
    std::string sourceaddress;
    HTTPConnState state;
    int sockid;
    bool paused;
    http::RequestParser parser;
    int latestrequestidresponded;
    std::list<std::pair<int, http::Response>> responses;
    std::vector<char> bodybuffer;
    std::vector<char> sendbuffer;
};
