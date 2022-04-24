#include "responseparser.h"

#include <cassert>

#include "../ext/picohttpparser.h"

#include "response.h"

namespace http {

Response ResponseParser::getResponse() {
    return response;
}

int ResponseParser::parse(const char* data, size_t datalen, size_t prevdatalen) {
    const char* message;
    int status;
    int minorversion;
    struct phr_header headers[MAX_HEADERS];
    size_t numheaders = MAX_HEADERS;
    size_t messagelen;
    int ret = phr_parse_response(data, datalen, &minorversion, &status, &message, &messagelen,
                                 headers, &numheaders, prevdatalen);
    if (ret > 0)
    {
        response = Response(status, std::string(message, messagelen), minorversion);
        parseHeaders(response, headers, numheaders);
    }
    return ret;
}
} // namespace EWHTTPParser
