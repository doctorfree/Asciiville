#pragma once

#include <list>
#include <map>
#include <set>
#include <utility>

#include "core/eventreceiver.h"
#include "http/request.h"

#include "requestcallback.h"

class Path;
class RestApiCallback;
class SiteLogic;

enum class OngoingRequestType {
  RAW_COMMAND,
  FILE_LIST,
  DOWNLOAD_FILE
};

class RestApi;

typedef void(RestApi::*EndpointPointer)(RestApiCallback* cb, int connrequestid, const http::Request& request);

struct OngoingRequest {
  OngoingRequestType type;
  int connrequestid;
  int apirequestid;
  RestApiCallback* cb;
  int timepassed = 0;
  int timeout;
  bool async = false;
  std::set<std::pair<void*, int>> ongoingservicerequests;
  std::list<std::pair<void*, std::string>> successes;
  std::list<std::pair<void*, std::string>> failures;
};

class RestApi : private RequestCallback, private Core::EventReceiver {
public:
  RestApi();
  ~RestApi();
  void handleRequest(RestApiCallback* cb, int connrequestid, const http::Request& request);
  std::string createTemporaryAuthToken();
  void removeTemporaryAuthToken(const std::string& token);
  void cancelOngoingSyncRequests(RestApiCallback* cb);
private:
  void tick(int message) override;
  void requestReady(void* service, int servicerequestid) override;
  void respondAsynced(OngoingRequest& request);
  void finalize(OngoingRequest& request);
  OngoingRequest* findOngoingRequest(void* service, int servicerequestid);
  OngoingRequest* findOngoingRequest(int apirequestid);
  void handlePathDelete(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handlePathGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleInfoGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleFileGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleRawPost(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleRawGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSitesGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSitesPost(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSiteGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSitePatch(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSiteDelete(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSpreadJobPost(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSpreadJobGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSpreadJobsGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSpreadJobReset(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSpreadJobAbort(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSiteSectionsGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSiteSectionGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSiteSectionPost(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSiteSectionPatch(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSiteSectionDelete(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleTransferJobsGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleTransferJobPost(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleTransferJobGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleTransferJobReset(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleTransferJobAbort(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSectionsGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSectionPost(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSectionGet(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSectionPatch(RestApiCallback* cb, int connrequestid, const http::Request& request);
  void handleSectionDelete(RestApiCallback* cb, int connrequestid, const http::Request& request);
  std::list<OngoingRequest> ongoingrequests;
  int nextrequestid;
  std::map<Path, std::map<std::string, EndpointPointer>> endpoints;
  std::set<std::string> tempauthtokens;
};
