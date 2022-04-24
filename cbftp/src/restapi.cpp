#include "restapi.h"

#include <cstdlib>
#include <ctime>
#include <list>
#include <memory>
#include <stdexcept>

#include "core/polling.h"
#include "core/sslmanager.h"
#include "core/tickpoke.h"
#include "core/types.h"
#include "ext/json.hpp"
#include "http/request.h"
#include "http/response.h"

#include "buildinfo.h"
#include "crypto.h"
#include "engine.h"
#include "eventlog.h"
#include "downloadfiledata.h"
#include "file.h"
#include "filelist.h"
#include "filelistdata.h"
#include "globalcontext.h"
#include "loadmonitor.h"
#include "localstorage.h"
#include "path.h"
#include "race.h"
#include "racestatus.h"
#include "remotecommandhandler.h"
#include "restapicallback.h"
#include "sectionmanager.h"
#include "sectionutil.h"
#include "settingsloadersaver.h"
#include "site.h"
#include "sitelogic.h"
#include "sitelogicmanager.h"
#include "sitemanager.h"
#include "siterace.h"
#include "skiplist.h"
#include "transferjob.h"
#include "util.h"

namespace {

#define RESTAPI_TICK_INTERVAL_MS 500
#define DEFAULT_TIMEOUT_SECONDS 60

enum class DeleteMode {
  NONE,
  INCOMPLETE,
  OWN,
  ALL
};

std::list<std::shared_ptr<SiteLogic>> getSiteLogicList(const nlohmann::json& jsondata) {
  std::set<std::shared_ptr<SiteLogic>> sitelogics;
  std::list<std::string> sites;
  auto sitesit = jsondata.find("sites");
  if (sitesit != jsondata.end()) {
    for (auto it = sitesit->begin(); it != sitesit->end(); ++it) {
      sites.push_back(*it);
    }
  }
  for (std::list<std::string>::const_iterator it = sites.begin(); it != sites.end(); it++) {
    const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(*it);
    if (!sl) {
      global->getEventLog()->log("RemoteCommandHandler", "Site not found: " + *it);
      continue;
    }
    sitelogics.insert(sl);
  }
  std::list<std::string> sections;
  auto sectionsit = jsondata.find("sites_with_sections");
  if (sectionsit != jsondata.end()) {
    for (auto it = sectionsit->begin(); it != sectionsit->end(); ++it) {
      sections.push_back(*it);
    }
  }

  auto allit = jsondata.find("sites_all");
  if (allit != jsondata.end() && allit->get<bool>()) {
    std::vector<std::shared_ptr<Site> >::const_iterator it;
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      if (!(*it)->getDisabled()) {
        std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic((*it)->getName());
        sitelogics.insert(sl);
      }
    }
  }
  for (const std::string& section : sections) {
    if (!global->getSectionManager()->getSection(section)) {
      global->getEventLog()->log("RemoteCommandHandler", "Section not found: " + section);
      continue;
    }
    for (std::vector<std::shared_ptr<Site> >::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); ++it) {
      if ((*it)->hasSection(section) && !(*it)->getDisabled()) {
        std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic((*it)->getName());
        sitelogics.insert(sl);
      }
    }
  }
  return std::list<std::shared_ptr<SiteLogic>>(sitelogics.begin(), sitelogics.end());
}

bool useOrSectionTranslate(Path& path, const std::shared_ptr<Site>& site) {
  if (path.isRelative()) {
    path.level(0);
    std::string section = path.level(0).toString();
    if (site->hasSection(section)) {
      path = site->getSectionPath(section) / path.cutLevels(-1);
    }
    else {
      return false;
    }
  }
  return true;
}

bool getQueryParamBoolValue(const http::Request& request, const std::string& param, bool defaultvalue = false) {
  bool boolvalue = defaultvalue;
  if (request.hasQueryParam(param)) {
    std::string value = request.getQueryParamValue(param);
    if (value != "0" && value != "false" && value != "NO") {
      boolvalue = true;
    }
    else {
      boolvalue = false;
    }
  }
  return boolvalue;
}

http::Response badRequestResponse(const std::string& error, int code = 400)
{
  http::Response response(400);
  nlohmann::json j = {{"error", error}};
  std::string errorjson = j.dump(2);
  response.setBody(std::vector<char>(errorjson.begin(), errorjson.end()));
  response.addHeader("Content-Type", "application/json");
  return response;
}

http::Response notFoundResponse()
{
  http::Response response(404);
  response.appendHeader("Content-Length", "0");
  return response;
}

http::Response emptySectionPathResponse() {
  return badRequestResponse("section path must not be empty");
}

std::string listCommandToString(int listcommand) {
  switch (listcommand) {
    case SITE_LIST_STAT:
      return "STAT_L";
    case SITE_LIST_LIST:
      return "LIST";
  }
  return "<unknown list command type " + std::to_string(listcommand) + ">";
}

std::string tlsModeToString(TLSMode mode) {
  switch (mode) {
    case TLSMode::NONE:
      return "NONE";
    case TLSMode::AUTH_TLS:
      return "AUTH_TLS";
    case TLSMode::IMPLICIT:
      return "IMPLICIT";
  }
  return "<unknown tls mode type " + std::to_string(static_cast<int>(mode)) + ">";
}

std::string tlsTransferPolicyToString(int policy) {
  switch (policy) {
    case SITE_SSL_ALWAYS_OFF:
      return "ALWAYS_OFF";
    case SITE_SSL_PREFER_OFF:
      return "PREFER_OFF";
    case SITE_SSL_PREFER_ON:
      return "PREFER_ON";
    case SITE_SSL_ALWAYS_ON:
      return "ALWAYS_ON";
  }
  return "<unknown tls policy type " + std::to_string(policy) + ">";
}

std::string transferProtocolToString(TransferProtocol proto) {
  switch (proto) {
    case TransferProtocol::IPV4_ONLY:
      return "IPV4_ONLY";
    case TransferProtocol::PREFER_IPV4:
      return "PREFER_IPV4";
    case TransferProtocol::PREFER_IPV6:
      return "PREFER_IPV6";
    case TransferProtocol::IPV6_ONLY:
      return "IPV6_ONLY";
  }
  return "<unknown transfer protocol type " + std::to_string(static_cast<int>(proto)) + ">";
}

std::string siteAllowTransferToString(SiteAllowTransfer allow) {
  switch (allow) {
    case SITE_ALLOW_TRANSFER_NO:
      return "NO";
    case SITE_ALLOW_TRANSFER_YES:
      return "YES";
    case SITE_ALLOW_DOWNLOAD_MATCH_ONLY:
      return "MATCH_ONLY";
  }
  return "<unknown site allow transfer type " + std::to_string(static_cast<int>(allow)) + ">";
}

std::string priorityToString(SitePriority priority) {
  switch (priority) {
    case SitePriority::VERY_LOW:
      return "VERY_LOW";
    case SitePriority::LOW:
      return "LOW";
    case SitePriority::NORMAL:
      return "NORMAL";
    case SitePriority::HIGH:
      return "HIGH";
    case SitePriority::VERY_HIGH:
      return "VERY_HIGH";
  }
  return "<unknown priority type " + std::to_string(static_cast<int>(priority)) + ">";
}

std::string refreshRateToString(RefreshRate rate) {
  switch (rate) {
    case RefreshRate::VERY_LOW:
      return "VERY_LOW";
    case RefreshRate::FIXED_LOW:
      return "FIXED_LOW";
    case RefreshRate::FIXED_AVERAGE:
      return "FIXED_AVERAGE";
    case RefreshRate::FIXED_HIGH:
      return "FIXED_HIGH";
    case RefreshRate::FIXED_VERY_HIGH:
      return "FIXED_VERY_HIGH";
    case RefreshRate::AUTO:
      return "AUTO";
    case RefreshRate::DYNAMIC_LOW:
      return "DYNAMIC_LOW";
    case RefreshRate::DYNAMIC_AVERAGE:
      return "DYNAMIC_AVERAGE";
    case RefreshRate::DYNAMIC_HIGH:
      return "DYNAMIC_HIGH";
    case RefreshRate::DYNAMIC_VERY_HIGH:
      return "DYNAMIC_VERY_HIGH";
  }
  return "Unknown";
}
std::string siteTransferPolicyToString(int policy) {
  switch (policy) {
    case SITE_TRANSFER_POLICY_ALLOW:
      return "ALLOW";
    case SITE_TRANSFER_POLICY_BLOCK:
      return "BLOCK";
  }
  return "<unknown site transfer policy type " + std::to_string(policy) + ">";
}

std::string skiplistActionToString(SkipListAction action) {
  switch (action) {
    case SKIPLIST_ALLOW:
      return "ALLOW";
    case SKIPLIST_DENY:
      return "DENY";
    case SKIPLIST_UNIQUE:
      return "UNIQUE";
    case SKIPLIST_SIMILAR:
      return "SIMILAR";
    case SKIPLIST_NONE:
      return "NONE";
  }
  return "<unknown skiplist action type " + std::to_string(action) + ">";
}

std::string skiplistScopeToString(int scope) {
  switch (scope) {
    case SCOPE_IN_RACE:
      return "IN_RACE";
    case SCOPE_ALL:
      return "ALL";
  }
  return "<unknown skiplist scope type " + std::to_string(scope) + ">";
}

std::string proxyTypeToString(int type) {
  switch (type) {
    case SITE_PROXY_GLOBAL:
      return "GLOBAL";
    case SITE_PROXY_NONE:
      return "NONE";
    case SITE_PROXY_USE:
      return "USE";
  }
  return "<unknown proxy type " + std::to_string(type) + ">";
}

std::string profileToString(SpreadProfile profile) {
  switch (profile) {
    case SPREAD_RACE:
      return "RACE";
    case SPREAD_DISTRIBUTE:
      return "DISTRIBUTE";
    case SPREAD_PREPARE:
      return "PREPARE";
  }
  return "<unknown profile type " + std::to_string(profile) + ">";
}

std::string raceStatusToString(RaceStatus status) {
  switch (status) {
    case RaceStatus::RUNNING:
      return "RUNNING";
    case RaceStatus::DONE:
      return "DONE";
    case RaceStatus::TIMEOUT:
      return "TIMEOUT";
    case RaceStatus::ABORTED:
      return "ABORTED";
  }
  return "<unknown race status type " + std::to_string(static_cast<int>(status)) + ">";
}

std::string transferJobStatusToString(TransferJobStatus status) {
  switch (status) {
    case TransferJobStatus::TRANSFERJOB_RUNNING:
      return "RUNNING";
    case TransferJobStatus::TRANSFERJOB_DONE:
      return "DONE";
    case TransferJobStatus::TRANSFERJOB_QUEUED:
      return "QUEUED";
    case TransferJobStatus::TRANSFERJOB_ABORTED:
      return "ABORTED";
  }
  return "<unknown transferjob status type " + std::to_string(static_cast<int>(status)) + ">";
}

std::string transferJobTypeToString(int type) {
  switch (type) {
    case TransferJobType::TRANSFERJOB_DOWNLOAD:
      return "DOWNLOAD";
    case TransferJobType::TRANSFERJOB_UPLOAD:
      return "UPLOAD";
    case TransferJobType::TRANSFERJOB_FXP:
      return "FXP";
  }
  return "<unknown transferjob type " + std::to_string(static_cast<int>(type)) + ">";
}

nlohmann::json jsonSkipList(const SkipList& skiplist) {
  nlohmann::json out = nlohmann::json::array();
  for (std::list<SkiplistItem>::const_iterator it = skiplist.entriesBegin(); it != skiplist.entriesEnd(); ++it) {
    nlohmann::json entry;
    entry["regex"] = it->matchRegex();
    entry["pattern"] = it->matchPattern();
    entry["file"] = it->matchFile();
    entry["dir"] = it->matchDir();
    entry["action"] = skiplistActionToString(it->getAction());
    entry["scope"] = skiplistScopeToString(it->matchScope());
    out.push_back(entry);
  }
  return out;
}

nlohmann::json jsonSiteSections(const std::shared_ptr<Site>& site) {
  nlohmann::json j = nlohmann::json::array();
  for (std::map<std::string, Path>::const_iterator it = site->sectionsBegin(); it != site->sectionsEnd(); ++it) {
    nlohmann::json sec;
    sec["name"] = it->first;
    sec["path"] = it->second.toString();
    j.push_back(sec);
  }
  return j;
}

nlohmann::json jsonSection(const Section& section) {
  nlohmann::json sec;
  sec["name"] = section.getName();
  sec["hotkey"] = section.getHotKey();
  sec["num_jobs"] = section.getNumJobs();
  sec["skiplist"] = jsonSkipList(section.getSkipList());
  return sec;
}

int stringToSkiplistScope(const std::string& scope) {
  if (scope == "IN_RACE") {
    return SCOPE_IN_RACE;
  }
  else if (scope == "ALL") {
    return SCOPE_ALL;
  }
  throw std::range_error("Unknown skiplist scope: " + scope);
}

SkipListAction stringToSkiplistAction(const std::string& action) {
  if (action == "ALLOW") {
    return SKIPLIST_ALLOW;
  }
  else if (action == "DENY") {
    return SKIPLIST_DENY;
  }
  else if (action == "UNIQUE") {
    return SKIPLIST_UNIQUE;
  }
  else if (action == "SIMILAR") {
    return SKIPLIST_SIMILAR;
  }
  throw std::range_error("Unknown skiplist action: " + action);
}

int stringToTransferPolicy(const std::string& policy) {
  if (policy == "ALLOW") {
    return SITE_TRANSFER_POLICY_ALLOW;
  }
  else if (policy == "BLOCK") {
    return SITE_TRANSFER_POLICY_BLOCK;
  }
  throw std::range_error("Unknown transfer policy: " + policy);
}

SitePriority stringToSitePriority(const std::string& priority) {
  if (priority == "VERY_LOW") {
    return SitePriority::VERY_LOW;
  }
  if (priority == "LOW") {
    return SitePriority::LOW;
  }
  if (priority == "HIGH") {
    return SitePriority::HIGH;
  }
  if (priority == "VERY_HIGH") {
    return SitePriority::VERY_HIGH;
  }
  if (priority == "NORMAL") {
    return SitePriority::NORMAL;
  }
  throw std::range_error("Unknown site priority: " + priority);
}

RefreshRate stringToRefreshRate(const std::string& rate) {
  if (rate == "VERY_LOW") {
    return RefreshRate::VERY_LOW;
  }
  if (rate == "FIXED_LOW") {
    return RefreshRate::FIXED_LOW;
  }
  if (rate == "FIXED_AVERAGE") {
    return RefreshRate::FIXED_AVERAGE;
  }
  if (rate == "FIXED_HIGH") {
    return RefreshRate::FIXED_HIGH;
  }
  if (rate == "FIXED_VERY_HIGH") {
    return RefreshRate::FIXED_VERY_HIGH;
  }
  if (rate == "DYNAMIC_LOW") {
    return RefreshRate::DYNAMIC_LOW;
  }
  if (rate == "DYNAMIC_AVERAGE") {
    return RefreshRate::DYNAMIC_AVERAGE;
  }
  if (rate == "DYNAMIC_HIGH") {
    return RefreshRate::DYNAMIC_HIGH;
  }
  if (rate == "DYNAMIC_VERY_HIGH") {
    return RefreshRate::DYNAMIC_VERY_HIGH;
  }
  if (rate == "AUTO") {
    return RefreshRate::AUTO;
  }
  throw std::range_error("Unknown refresh rate: " + rate);
}

TransferProtocol stringToTransferProtocol(const std::string& protocol) {
  if (protocol == "IPV4_ONLY") {
    return TransferProtocol::IPV4_ONLY;
  }
  if (protocol == "PREFER_IPV4") {
    return TransferProtocol::PREFER_IPV4;
  }
  if (protocol == "PREFER_IPV6") {
    return TransferProtocol::PREFER_IPV6;
  }
  if (protocol == "IPV6_ONLY") {
    return TransferProtocol::IPV6_ONLY;
  }
  throw std::range_error("Unknown transfer protocol: " + protocol);
}

int stringToTlsTransferPolicy(const std::string& policy) {
  if (policy == "ALWAYS_OFF") {
    return SITE_SSL_ALWAYS_OFF;
  }
  if (policy == "PREFER_OFF") {
    return SITE_SSL_PREFER_OFF;
  }
  if (policy == "PREFER_ON") {
    return SITE_SSL_PREFER_ON;
  }
  if (policy == "ALWAYS_ON") {
    return SITE_SSL_ALWAYS_ON;
  }
  throw std::range_error("Unknown TLS transfer policy: " + policy);
}

TLSMode stringToTlsMode(const std::string& mode) {
  if (mode == "NONE") {
    return TLSMode::NONE;
  }
  if (mode == "AUTH_TLS") {
    return TLSMode::AUTH_TLS;
  }
  if (mode == "IMPLICIT") {
    return TLSMode::IMPLICIT;
  }
  throw std::range_error("Unknown TLS mode: " + mode);
}

int stringToListCommand(const std::string& command) {
  if (command == "STAT_L") {
    return SITE_LIST_STAT;
  }
  if (command == "LIST") {
    return SITE_LIST_LIST;
  }
  throw std::range_error("Unknown list command: " + command);
}

SiteAllowTransfer stringToSiteAllowTransfer(const std::string& allow) {
  if (allow == "NO") {
    return SITE_ALLOW_TRANSFER_NO;
  }
  if (allow == "YES") {
    return SITE_ALLOW_TRANSFER_YES;
  }
  if (allow == "MATCH_ONLY") {
    return SITE_ALLOW_DOWNLOAD_MATCH_ONLY;
  }
  throw std::range_error("Unknown allow transfer setting: " + allow);
}

int stringToProxyType(const std::string& type) {
  if (type == "GLOBAL") {
    return SITE_PROXY_GLOBAL;
  }
  if (type == "NONE") {
    return SITE_PROXY_NONE;
  }
  if (type == "USE") {
    return SITE_PROXY_USE;
  }
  throw std::range_error("Unknown proxy type: " + type);
}

SpreadProfile stringToProfile(const std::string& profile) {
  if (profile == "DISTRIBUTE") {
    return SPREAD_DISTRIBUTE;
  }
  if (profile == "PREPARE") {
    return SPREAD_PREPARE;
  }
  if (profile == "RACE") {
    return SPREAD_RACE;
  }
  throw std::range_error("Unknown spread job profile: " + profile);
}

DeleteMode stringToDeleteMode(const std::string& mode) {
  if (mode == "INCOMPLETE") {
    return DeleteMode::INCOMPLETE;
  }
  if (mode == "OWN") {
    return DeleteMode::OWN;
  }
  if (mode == "ALL") {
    return DeleteMode::ALL;
  }
  if (mode == "NONE") {
    return DeleteMode::NONE;
  }
  throw std::range_error("Unknown delete mode: " + mode);
}

TransferJobStatus stringToTransferJobStatus(const std::string& status) {
  if (status == "RUNNING") {
    return TransferJobStatus::TRANSFERJOB_RUNNING;
  }
  else if (status == "DONE") {
    return TransferJobStatus::TRANSFERJOB_DONE;
  }
  else if (status == "QUEUED") {
    return TransferJobStatus::TRANSFERJOB_QUEUED;
  }
  else if (status == "ABORTED") {
    return TransferJobStatus::TRANSFERJOB_ABORTED;
  }
  throw std::range_error("Unknown transfer job status: " + status);
}

RaceStatus stringToRaceStatus(const std::string& status) {
  if (status == "RUNNING") {
    return RaceStatus::RUNNING;
  }
  else if (status == "DONE") {
    return RaceStatus::DONE;
  }
  else if (status == "ABORTED") {
    return RaceStatus::ABORTED;
  }
  else if (status == "TIMEOUT") {
    return RaceStatus::TIMEOUT;
  }
  throw std::range_error("Unknown spread job status: " + status);
}

TransferJobType stringToTransferJobType(const std::string& type) {
  if (type == "FXP") {
    return TRANSFERJOB_FXP;
  }
  else if (type == "DOWNLOAD") {
    return TRANSFERJOB_DOWNLOAD;
  }
  else if (type == "UPLOAD") {
    return TRANSFERJOB_UPLOAD;
  }
  throw std::range_error("Unknown transfer job type: " + type);
}

void updateSkipList(SkipList& skiplist, nlohmann::json jsonlist) {
  skiplist.clearEntries();
  for (nlohmann::json::const_iterator it = jsonlist.begin(); it != jsonlist.end(); ++it) {
    bool regex = it.value()["regex"];
    std::string pattern = it.value()["pattern"];
    bool file = it.value()["file"];
    bool dir = it.value()["dir"];
    int scope = stringToSkiplistScope(it.value()["scope"]);
    SkipListAction action = stringToSkiplistAction(it.value()["action"]);
    skiplist.addEntry(regex, pattern, file, dir, scope, action);
  }
}

void updateSite(std::shared_ptr<Site>& site, nlohmann::json jsondata, bool add) {
  bool changedname = false;
  std::list<std::string> exceptsrclist;
  std::list<std::string> exceptdstlist;
  for (std::set<std::shared_ptr<Site> >::const_iterator it = site->exceptSourceSitesBegin(); it != site->exceptSourceSitesEnd(); ++it) {
    exceptsrclist.push_back((*it)->getName());
  }
  for (std::set<std::shared_ptr<Site> >::const_iterator it = site->exceptTargetSitesBegin(); it != site->exceptTargetSitesEnd(); ++it) {
    exceptdstlist.push_back((*it)->getName());
  }
  for (nlohmann::json::const_iterator it = jsondata.begin(); it != jsondata.end(); ++it) {
    if (it.key() == "name") {
      changedname = !add && std::string(it.value()) != site->getName();
      site->setName(it.value());
    }
    else if (it.key() == "addresses") {
      std::string addrports;
      for (nlohmann::json::const_iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
        addrports += std::string(it2.value()) + " ";
      }
      site->setAddresses(addrports);
    }
    else if (it.key() == "user") {
      site->setUser(it.value());
    }
    else if (it.key() == "password") {
      site->setPass(it.value());
    }
    else if (it.key() == "base_path") {
      site->setBasePath(it.value());
    }
    else if (it.key() == "max_logins") {
      site->setMaxLogins(it.value());
    }
    else if (it.key() == "max_sim_up") {
      site->setMaxUp(it.value());
    }
    else if (it.key() == "max_sim_down") {
      site->setMaxDn(it.value());
    }
    else if (it.key() == "max_sim_down_pre") {
      site->setMaxDnPre(it.value());
    }
    else if (it.key() == "max_sim_down_complete") {
      site->setMaxDnComplete(it.value());
    }
    else if (it.key() == "max_sim_down_transferjob") {
      site->setMaxDnTransferJob(it.value());
    }
    else if (it.key() == "max_idle_time") {
      site->setMaxIdleTime(it.value());
    }
    else if (it.key() == "pret") {
      site->setPRET(it.value());
    }
    else if (it.key() == "force_binary_mode") {
      site->setForceBinaryMode(it.value());
    }
    else if (it.key() == "list_command") {
      site->setListCommand(stringToListCommand(it.value()));
    }
    else if (it.key() == "tls_mode") {
      site->setTLSMode(stringToTlsMode(it.value()));
    }
    else if (it.key() == "tls_transfer_policy") {
      site->setSSLTransferPolicy(stringToTlsTransferPolicy(it.value()));
    }
    else if (it.key() == "transfer_protocol") {
      site->setTransferProtocol(stringToTransferProtocol(it.value()));
    }
    else if (it.key() == "sscn") {
      site->setSupportsSSCN(it.value());
    }
    else if (it.key() == "cpsv") {
      site->setSupportsCPSV(it.value());
    }
    else if (it.key() == "cepr") {
      site->setSupportsCEPR(it.value());
    }
    else if (it.key() == "broken_pasv") {
      site->setBrokenPASV(it.value());
    }
    else if (it.key() == "disabled") {
      site->setDisabled(it.value());
    }
    else if (it.key() == "allow_upload") {
      site->setAllowUpload(stringToSiteAllowTransfer(it.value()));
    }
    else if (it.key() == "allow_download") {
      site->setAllowDownload(stringToSiteAllowTransfer(it.value()));
    }
    else if (it.key() == "priority") {
      site->setPriority(stringToSitePriority(it.value()));
    }
    else if (it.key() == "list_frequency") {
      site->setRefreshRate(stringToRefreshRate(it.value()));
    }
    else if (it.key() == "xdupe") {
      site->setUseXDUPE(it.value());
    }
    else if (it.key() == "sections") {
      site->clearSections();
      for (nlohmann::json::const_iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
        std::string sectionpath = it2.value()["path"];
        if (sectionpath.empty()) {
          throw std::invalid_argument("Section path must not be empty");
        }
        site->addSection(it2.value()["name"], it2.value()["path"]);
      }
    }
    else if (it.key() == "avg_speed") {
      for (nlohmann::json::const_iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
        site->setAverageSpeed(it2.key(), it2.value());
      }
    }
    else if (it.key() == "affils") {
      site->clearAffils();
      for (nlohmann::json::const_iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
        site->addAffil(it2.value());
      }
    }
    else if (it.key() == "transfer_source_policy") {
      site->setTransferSourcePolicy(stringToTransferPolicy(it.value()));
    }
    else if (it.key() == "transfer_target_policy") {
      site->setTransferTargetPolicy(stringToTransferPolicy(it.value()));
    }
    else if (it.key() == "except_source_sites") {
      exceptsrclist.clear();
      for (nlohmann::json::const_iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
        exceptsrclist.push_back(it2.value());
      }
    }
    else if (it.key() == "except_target_sites") {
      exceptdstlist.clear();
      for (nlohmann::json::const_iterator it2 = it.value().begin(); it2 != it.value().end(); ++it2) {
        exceptdstlist.push_back(it2.value());
      }
    }
    else if (it.key() == "leave_free_slot") {
      site->setLeaveFreeSlot(it.value());
    }
    else if (it.key() == "stay_logged_in") {
      site->setStayLoggedIn(it.value());
    }
    else if (it.key() == "skiplist") {
      updateSkipList(site->getSkipList(), it.value());
    }
    else if (it.key() == "proxy_type") {
      site->setProxyType(stringToProxyType(it.value()));
    }
    else if (it.key() == "proxy_name") {
      site->setProxy(it.value());
    }
    else {
      throw std::range_error("Unrecognized key: " + it.key());
    }
  }
  if (add) {
    global->getSiteManager()->addSite(site);
  }
  else {
    global->getSiteManager()->sortSites();
  }
  std::string sitename = site->getName();
  global->getSiteManager()->resetSitePairsForSite(sitename);
  for (const std::string& exceptsrcsite : exceptsrclist) {
    global->getSiteManager()->addExceptSourceForSite(sitename, exceptsrcsite);
  }
  for (const std::string& exceptdstsite : exceptdstlist) {
    global->getSiteManager()->addExceptTargetForSite(sitename, exceptdstsite);
  }

  global->getSiteLogicManager()->getSiteLogic(site->getName())->siteUpdated();
  if (changedname) {
    global->getSiteLogicManager()->getSiteLogic(site->getName())->updateName();
  }

  global->getSettingsLoaderSaver()->saveSettings();
}

void updateSection(Section* section, nlohmann::json jsondata) {
  for (nlohmann::json::const_iterator it = jsondata.begin(); it != jsondata.end(); ++it) {
    if (it.key() == "name") {
      section->setName(it.value());
    }
    else if (it.key() == "skiplist") {
      updateSkipList(section->getSkipList(), it.value());
    }
    else if (it.key() == "num_jobs") {
      section->setNumJobs(it.value());
    }
    else if (it.key() == "hotkey") {
      section->setHotKey(it.value());
    }
    else {
      throw std::range_error("Unrecognized key: " + it.key());
    }
  }
  global->getSettingsLoaderSaver()->saveSettings();
}

nlohmann::json getJsonFromBody(const http::Request& request) {
  std::shared_ptr<std::vector<char>> body = request.getBody();
  nlohmann::json jsondata;
  if (body && !body->empty()) {
    jsondata = nlohmann::json::parse(std::string(body->begin(), body->end()));
  }
  return jsondata;
}

}

RestApi::RestApi() : nextrequestid(0) {
  endpoints["/file"]["GET"] = &RestApi::handleFileGet;
  endpoints["/filelist"]["GET"] = &RestApi::handlePathGet; // deprecated, use /path
  endpoints["/info"]["GET"] = &RestApi::handleInfoGet;
  endpoints["/path"]["DELETE"] = &RestApi::handlePathDelete;
  endpoints["/path"]["GET"] = &RestApi::handlePathGet;
  endpoints["/raw"]["POST"] = &RestApi::handleRawPost;
  endpoints["/raw/*"]["GET"] = &RestApi::handleRawGet;
  endpoints["/sites"]["GET"] = &RestApi::handleSitesGet;
  endpoints["/sites"]["POST"] = &RestApi::handleSitesPost;
  endpoints["/sites/*"]["GET"] = &RestApi::handleSiteGet;
  endpoints["/sites/*"]["PATCH"] = &RestApi::handleSitePatch;
  endpoints["/sites/*"]["DELETE"] = &RestApi::handleSiteDelete;
  endpoints["/sites/*/sections"]["GET"] = &RestApi::handleSiteSectionsGet;
  endpoints["/sites/*/sections"]["POST"] = &RestApi::handleSiteSectionPost;
  endpoints["/sites/*/sections/*"]["GET"] = &RestApi::handleSiteSectionGet;
  endpoints["/sites/*/sections/*"]["PATCH"] = &RestApi::handleSiteSectionPatch;
  endpoints["/sites/*/sections/*"]["DELETE"] = &RestApi::handleSiteSectionDelete;
  endpoints["/sections"]["GET"] = &RestApi::handleSectionsGet;
  endpoints["/sections"]["POST"] = &RestApi::handleSectionPost;
  endpoints["/sections/*"]["GET"] = &RestApi::handleSectionGet;
  endpoints["/sections/*"]["PATCH"] = &RestApi::handleSectionPatch;
  endpoints["/sections/*"]["DELETE"] = &RestApi::handleSectionDelete;
  endpoints["/spreadjobs"]["POST"] = &RestApi::handleSpreadJobPost;
  endpoints["/spreadjobs"]["GET"] = &RestApi::handleSpreadJobsGet;
  endpoints["/spreadjobs/*"]["GET"] = &RestApi::handleSpreadJobGet;
  endpoints["/spreadjobs/*/reset"]["POST"] = &RestApi::handleSpreadJobReset;
  endpoints["/spreadjobs/*/abort"]["POST"] = &RestApi::handleSpreadJobAbort;
  endpoints["/transferjobs"]["POST"] = &RestApi::handleTransferJobPost;
  endpoints["/transferjobs"]["GET"] = &RestApi::handleTransferJobsGet;
  endpoints["/transferjobs/*"]["GET"] = &RestApi::handleTransferJobGet;
  endpoints["/transferjobs/*/reset"]["POST"] = &RestApi::handleTransferJobReset;
  endpoints["/transferjobs/*/abort"]["POST"] = &RestApi::handleTransferJobAbort;

  global->getTickPoke()->startPoke(this, "RestApi", RESTAPI_TICK_INTERVAL_MS, 0);
}

RestApi::~RestApi() {
  global->getTickPoke()->stopPoke(this, 0);
}

void RestApi::handleFileGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  if (!request.hasQueryParam("site")) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing query parameter: site"));
    return;
  }
  if (!request.hasQueryParam("path")) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing query parameter: path"));
    return;
  }
  std::string sitestr = request.getQueryParamValue("site");
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitestr);
  std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(sitestr);
  if (!site || !sl) {
    cb->requestHandled(connrequestid, badRequestResponse("Site not found: " + sitestr));
    return;
  }
  Path path = request.getQueryParamValue("path");
  int timeout = DEFAULT_TIMEOUT_SECONDS;
  if (request.hasQueryParam("timeout")) {
    std::string timeoutstr = request.getQueryParamValue("timeout");
    try {

      timeout = std::stoi(timeoutstr);
    }
    catch(std::exception& e) {
      cb->requestHandled(connrequestid, badRequestResponse("Invalid timeout value: " + timeoutstr));
      return;
    }
  }
  int servicerequestid = sl->requestDownloadFile(this, path.dirName(), path.baseName(), true);
  OngoingRequest ongoingrequest;
  ongoingrequest.type = OngoingRequestType::DOWNLOAD_FILE;
  ongoingrequest.connrequestid = connrequestid;
  ongoingrequest.apirequestid = nextrequestid++;
  ongoingrequest.cb = cb;
  ongoingrequest.timeout = timeout;
  ongoingrequest.ongoingservicerequests.insert(std::make_pair(sl.get(), servicerequestid));
  ongoingrequests.push_back(ongoingrequest);
}

void RestApi::handleRawPost(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  std::list<std::shared_ptr<SiteLogic>> sites = getSiteLogicList(jsondata);
  if (sites.empty()) {
    cb->requestHandled(connrequestid, badRequestResponse("No sites specified"));
    return;
  }
  auto commandit = jsondata.find("command");
  if (commandit == jsondata.end()) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing key: command"));
    return;
  }
  std::string command = *commandit;
  auto pathit = jsondata.find("path_section");
  std::string path;
  if (pathit != jsondata.end()) {
    path = *pathit;
  }
  else {
    pathit = jsondata.find("path");
    if (pathit != jsondata.end()) {
      path = *pathit;
    }
  }
  auto asyncit = jsondata.find("async");
  auto timeoutit = jsondata.find("timeout");
  OngoingRequest ongoingrequest;
  ongoingrequest.type = OngoingRequestType::RAW_COMMAND;
  ongoingrequest.connrequestid = connrequestid;
  ongoingrequest.apirequestid = nextrequestid++;
  ongoingrequest.cb = cb;
  ongoingrequest.async = asyncit != jsondata.end() && static_cast<bool>(*asyncit);
  ongoingrequest.timeout = timeoutit != jsondata.end() ? static_cast<int>(*timeoutit) : DEFAULT_TIMEOUT_SECONDS;
  std::list<std::pair<std::shared_ptr<SiteLogic>, Path>> verifiedsites;
  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
    Path thispath;
    if (!path.empty()) {
      util::Result res = SectionUtil::useOrSectionTranslate(path, (*it)->getSite(), thispath);
      if (!res.success) {
        cb->requestHandled(connrequestid, badRequestResponse(res.error));
        return;
      }
    }
    verifiedsites.emplace_back(*it, thispath.empty() ? (*it)->getSite()->getBasePath().toString() : thispath);
  }
  for (const std::pair<std::shared_ptr<SiteLogic>, Path>& site : verifiedsites) {
    int servicerequestid = site.first->requestRawCommand(this, site.second, command);
    ongoingrequest.ongoingservicerequests.insert(std::make_pair(site.first.get(), servicerequestid));
  }
  ongoingrequests.push_back(ongoingrequest);
  if (ongoingrequest.async) {
    respondAsynced(ongoingrequest);
  }
}

void RestApi::handleRawGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string apirequestidstr = path.level(1).toString();
  int apirequestid;
  try {
    apirequestid = std::stoi(apirequestidstr);
  }
  catch(std::exception& e) {
    cb->requestHandled(connrequestid, badRequestResponse("Invalid request id: " + apirequestidstr));
    return;
  }
  OngoingRequest* ongoingrequest = findOngoingRequest(apirequestid);
  if (!ongoingrequest) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  if (ongoingrequest->ongoingservicerequests.empty()) {
    finalize(*ongoingrequest);
    return;
  }
  else {
    http::Response response(202);
    response.appendHeader("Content-Length", "0");
    cb->requestHandled(connrequestid, response);
    return;
  }
}

void RestApi::handleSitesGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json sitelist = nlohmann::json::array();
  bool filtersection = request.hasQueryParam("section");
  std::string section = request.getQueryParamValue("section");
  if (filtersection && !global->getSectionManager()->getSection(section)) {
    cb->requestHandled(connrequestid, badRequestResponse("Unknown section: " + section));
    return;
  }
  for (std::vector<std::shared_ptr<Site>>::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); ++it) {
    if (!filtersection || (*it)->hasSection(section)) {
      sitelist.push_back((*it)->getName());
    }
  }
  http::Response response(200);
  std::string jsondump = sitelist.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSitesPost(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  std::shared_ptr<Site> site = global->getSiteManager()->createNewSite();
  auto nameit = jsondata.find("name");
  if (nameit == jsondata.end()) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing key: name"));
    return;
  }
  if (global->getSiteManager()->getSite(std::string(*nameit))) {
    http::Response response(409);
    response.appendHeader("Content-Length", "0");
    cb->requestHandled(connrequestid, response);
    return;
  }
  updateSite(site, jsondata, true);
  http::Response response(201);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSiteGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sitename = path.level(1).toString();
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
  std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(sitename);
  if (!site || !sl) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json j;
  nlohmann::json addrlist = nlohmann::json::array();
  for (Address addr : site->getAddresses()) {
    addrlist.push_back(addr.toString());
  }
  j["addresses"] = addrlist;
  j["user"] = site->getUser();
  j["password"] = site->getPass();
  j["base_path"] = site->getBasePath().toString();
  j["max_logins"] = site->getMaxLogins();
  j["max_sim_up"] = site->getInternMaxUp();
  j["max_sim_down"] = site->getInternMaxDown();
  j["max_sim_down_pre"] = site->getInternMaxDownPre();
  j["max_sim_down_complete"] = site->getInternMaxDownComplete();
  j["max_sim_down_transferjob"] = site->getInternMaxDownTransferJob();
  j["max_idle_time"] = site->getMaxIdleTime();
  j["pret"] = site->needsPRET();
  j["force_binary_mode"] = site->forceBinaryMode();
  j["list_command"] = listCommandToString(site->getListCommand());
  j["tls_mode"] = tlsModeToString(site->getTLSMode());
  j["tls_transfer_policy"] = tlsTransferPolicyToString(site->getSSLTransferPolicy());
  j["transfer_protocol"] = transferProtocolToString(site->getTransferProtocol());
  j["sscn"] = site->supportsSSCN();
  j["cpsv"] = site->supportsCPSV();
  j["cepr"] = site->supportsCEPR();
  j["broken_pasv"] = site->hasBrokenPASV();
  j["disabled"] = site->getDisabled();
  j["allow_upload"] = siteAllowTransferToString(site->getAllowUpload());
  j["allow_download"] = siteAllowTransferToString(site->getAllowDownload());
  j["priority"] = priorityToString(site->getPriority());
  j["list_frequency"] = refreshRateToString(site->getRefreshRate());
  j["xdupe"] = site->useXDUPE();
  j["sections"] = jsonSiteSections(site);
  for (std::map<std::string, int>::const_iterator it = site->avgspeedBegin(); it != site->avgspeedEnd(); ++it) {
    j["avg_speed"][it->first] = it->second;
  }
  nlohmann::json affils = nlohmann::json::array();
  for (std::set<std::string>::const_iterator it = site->affilsBegin(); it != site->affilsEnd(); ++it) {
    affils.push_back(*it);
  }
  j["affils"] = affils;
  j["transfer_source_policy"] = siteTransferPolicyToString(site->getTransferSourcePolicy());
  j["transfer_target_policy"] = siteTransferPolicyToString(site->getTransferTargetPolicy());
  nlohmann::json exceptsource = nlohmann::json::array();
  for (std::set<std::shared_ptr<Site> >::const_iterator it = site->exceptSourceSitesBegin(); it != site->exceptSourceSitesEnd(); ++it) {
    exceptsource.push_back((*it)->getName());
  }
  j["except_source_sites"] = exceptsource;
  nlohmann::json excepttarget = nlohmann::json::array();
  for (std::set<std::shared_ptr<Site> >::const_iterator it = site->exceptTargetSitesBegin(); it != site->exceptTargetSitesEnd(); ++it) {
    excepttarget.push_back((*it)->getName());
  }
  j["except_target_sites"] = excepttarget;
  j["leave_free_slot"] = site->getLeaveFreeSlot();
  j["stay_logged_in"] = site->getStayLoggedIn();
  j["skiplist"] = jsonSkipList(site->getSkipList());
  j["proxy_type"] = proxyTypeToString(site->getProxyType());
  j["proxy_name"] = site->getProxy();
  j["var"]["size_up_all"] = site->getSizeUp().getAll();
  j["var"]["size_up_24h"] = site->getSizeUp().getLast24Hours();
  j["var"]["files_up_all"] = site->getFilesUp().getAll();
  j["var"]["files_up_24h"] = site->getFilesUp().getLast24Hours();
  j["var"]["size_down_all"] = site->getSizeDown().getAll();
  j["var"]["size_down_24h"] = site->getSizeDown().getLast24Hours();
  j["var"]["files_down_all"] = site->getFilesDown().getAll();
  j["var"]["files_down_24h"] = site->getFilesDown().getLast24Hours();
  j["var"]["current_logins"] = sl->getCurrLogins();
  j["var"]["current_up"] = sl->getCurrUp();
  j["var"]["current_down"] = sl->getCurrDown();
  http::Response response(200);
  std::string jsondump = j.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSitePatch(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sitename = path.level(1).toString();
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
  if (!site) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json jsondata = getJsonFromBody(request);
  updateSite(site, jsondata, false);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSiteDelete(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sitename = path.level(1).toString();
  global->getSiteManager()->deleteSite(sitename);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handlePathDelete(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  if (!request.hasQueryParam("site")) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing query parameter: site"));
    return;
  }
  if (!request.hasQueryParam("path")) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing query parameter: path"));
    return;
  }
  bool allfiles = true;
  if (request.hasQueryParam("type")) {
    std::string deltype = request.getQueryParamValue("type");
    if (deltype == "OWN") {
      allfiles = false;
    }
    else if (deltype != "OWN") {
      cb->requestHandled(connrequestid, badRequestResponse("Invalid type: " + deltype));
      return;
    }
  }
  std::string sitestr = request.getQueryParamValue("site");
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitestr);
  std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(sitestr);
  if (!site || !sl) {
    cb->requestHandled(connrequestid, badRequestResponse("Site not found: " + sitestr));
    return;
  }
  Path path = request.getQueryParamValue("path");
  if (!useOrSectionTranslate(path, site)) {
    cb->requestHandled(connrequestid, badRequestResponse("Path must be absolute or a section on " + sitestr + ": " + path.toString()));
    return;
  }
  sl->requestDelete(nullptr, path, true, allfiles);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handlePathGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  if (!request.hasQueryParam("site")) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing query parameter: site"));
    return;
  }
  if (!request.hasQueryParam("path")) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing query parameter: path"));
    return;
  }
  std::string sitestr = request.getQueryParamValue("site");
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitestr);
  std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(sitestr);
  if (!site || !sl) {
    cb->requestHandled(connrequestid, badRequestResponse("Site not found: " + sitestr));
    return;
  }
  Path path = request.getQueryParamValue("path");
  if (!useOrSectionTranslate(path, site)) {
    cb->requestHandled(connrequestid, badRequestResponse("Path must be absolute or a section on " + sitestr + ": " + path.toString()));
    return;
  }
  int timeout = DEFAULT_TIMEOUT_SECONDS;
  if (request.hasQueryParam("timeout")) {
    std::string timeoutstr = request.getQueryParamValue("timeout");
    try {

      timeout = std::stoi(timeoutstr);
    }
    catch(std::exception& e) {
      cb->requestHandled(connrequestid, badRequestResponse("Invalid timeout value: " + timeoutstr));
      return;
    }
  }
  int servicerequestid = sl->requestFileList(this, path);
  OngoingRequest ongoingrequest;
  ongoingrequest.type = OngoingRequestType::FILE_LIST;
  ongoingrequest.connrequestid = connrequestid;
  ongoingrequest.apirequestid = nextrequestid++;
  ongoingrequest.cb = cb;
  ongoingrequest.timeout = timeout;
  ongoingrequest.ongoingservicerequests.insert(std::make_pair(sl.get(), servicerequestid));
  ongoingrequests.push_back(ongoingrequest);
}

void RestApi::handleInfoGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  unsigned long long int sizefxpday = global->getStatistics()->getSizeFXP().getLast24Hours();
  unsigned int filesfxpday = global->getStatistics()->getFilesFXP().getLast24Hours();
  unsigned long long int sizefxpall = global->getStatistics()->getSizeFXP().getAll();
  unsigned int filesfxpall = global->getStatistics()->getFilesFXP().getAll();
  unsigned long long int sizeupday = global->getStatistics()->getSizeUp().getLast24Hours();
  unsigned int filesupday = global->getStatistics()->getFilesUp().getLast24Hours();
  unsigned long long int sizeupall = global->getStatistics()->getSizeUp().getAll();
  unsigned int filesupall = global->getStatistics()->getFilesUp().getAll();
  unsigned long long int sizedownday = global->getStatistics()->getSizeDown().getLast24Hours();
  unsigned int filesdownday = global->getStatistics()->getFilesDown().getLast24Hours();
  unsigned long long int sizedownall = global->getStatistics()->getSizeDown().getAll();
  unsigned int filesdownall = global->getStatistics()->getFilesDown().getAll();
  nlohmann::json j;
  nlohmann::json& buildinfo = j["build_info"];
  nlohmann::json& stats = j["stats"];
  nlohmann::json& load = j["load"];
  buildinfo["compile_time"] = BuildInfo::compileTime();
  buildinfo["version_tag"] = BuildInfo::version();
  buildinfo["distribution_tag"] = BuildInfo::tag();
  buildinfo["openssl_version"] = Core::SSLManager::version();
  buildinfo["polling_syscall"] = Core::Polling::type();
  j["data_file_encryption_enabled"] = global->getSettingsLoaderSaver()->getState() == DataFileState::EXISTS_DECRYPTED;
  stats["spread_jobs_all"] = global->getStatistics()->getSpreadJobs();
  stats["transfer_jobs_all"] = global->getStatistics()->getTransferJobs();
  load["cpu_load_total_pc"] = global->getLoadMonitor()->getCurrentCpuUsageAll();
  load["cpu_load_worker_pc"] = global->getLoadMonitor()->getCurrentCpuUsageWorker();
  load["performance_level"] = global->getLoadMonitor()->getCurrentRecommendedPerformanceLevel();
  load["worker_queue_size"] = global->getLoadMonitor()->getCurrentWorkerQueueSize();
  load["file_list_refresh_rate"] = global->getLoadMonitor()->getCurrentFileListRefreshRate();
  stats["files_fxp_24h"] = filesfxpday;
  stats["size_fxp_24h"] = sizefxpday;
  stats["files_fxp_all"] = filesfxpall;
  stats["size_fxp_all"] = sizefxpall;
  stats["files_down_24h"] = filesdownday;
  stats["size_down_24h"] = sizedownday;
  stats["files_down_all"] = filesdownall;
  stats["size_down_all"] = sizedownall;
  stats["files_up_24h"] = filesupday;
  stats["size_up_24h"] = sizeupday;
  stats["files_up_all"] = filesupall;
  stats["size_up_all"] = sizeupall;
  http::Response response(200);
  std::string jsondump = j.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSpreadJobPost(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  auto sectionit = jsondata.find("section");
  if (sectionit == jsondata.end()) {

    cb->requestHandled(connrequestid, badRequestResponse("Missing key: section"));
    return;
  }
  std::string section = *sectionit;
  auto nameit = jsondata.find("name");
  if (nameit == jsondata.end()) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing key: name"));
    return;
  }
  std::string name = *nameit;
  std::list<std::string> sites;
  auto sitesit = jsondata.find("sites");
  if (sitesit != jsondata.end()) {
    for (auto it = sitesit->begin(); it != sitesit->end(); ++it) {
      sites.push_back(*it);
    }
  }
  std::list<std::string> dlonlysites;
  auto dlonlysitesit = jsondata.find("sites_dlonly");
  if (dlonlysitesit != jsondata.end()) {
    for (auto it = dlonlysitesit->begin(); it != dlonlysitesit->end(); ++it) {
      dlonlysites.push_back(*it);
    }
  }
  auto allit = jsondata.find("sites_all");
  if (allit != jsondata.end() && allit->get<bool>()) {
    sites.clear();
    std::vector<std::shared_ptr<Site>>::const_iterator it;
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      if (!(*it)->getDisabled() && (*it)->hasSection(section)) {
        sites.push_back((*it)->getName());
      }
    }
  }
  auto resetit = jsondata.find("reset");
  bool reset = resetit != jsondata.end() && resetit->get<bool>();
  bool success = false;
  SpreadProfile profile = SPREAD_RACE;
  auto profileit = jsondata.find("profile");
  if (profileit != jsondata.end()) {
    profile = stringToProfile(*profileit);
  }
  switch (profile) {
    case  SPREAD_DISTRIBUTE:
      success = !!global->getEngine()->newDistribute(name, section, sites, reset, dlonlysites);
      break;
    case SPREAD_PREPARE:
      success = global->getEngine()->prepareRace(name, section, sites, reset, dlonlysites);
      break;
    case SPREAD_RACE:
      success = !!global->getEngine()->newRace(name, section, sites, reset, dlonlysites);
      break;
  }
  http::Response response(201);
  response.appendHeader("Content-Length", "0");
  if (!success) {
    response.setStatusCode(503);
  }
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSpreadJobGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  std::string name = Path(request.getPath()).baseName();
  std::shared_ptr<Race> spreadjob = global->getEngine()->getRace(name);
  if (!spreadjob) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json j;

  nlohmann::json sites = nlohmann::json::array();
  nlohmann::json dlonlysites = nlohmann::json::array();
  nlohmann::json incsites = nlohmann::json::array();
  std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic>>>::const_iterator it;
  for (it = spreadjob->begin(); it != spreadjob->end(); ++it) {
    std::string sitename = it->first->getSiteName();
    sites.push_back(sitename);
    if (it->first->isDownloadOnly()) {
      dlonlysites.push_back(sitename);
    }
    if (it->first->getStatus() != RaceStatus::DONE) {
      incsites.push_back(sitename);
    }
  }
  j["section"] = spreadjob->getSection();
  j["sites"] = sites;
  j["sites_dlonly"] = dlonlysites;
  j["sites_incomplete"] = incsites;
  j["profile"] = profileToString(spreadjob->getProfile());
  j["status"] = raceStatusToString(spreadjob->getStatus());
  j["time_started"] = spreadjob->getTimeStamp();
  j["time_spent_seconds"] = spreadjob->getTimeSpent();
  j["size_estimated_bytes"] = spreadjob->estimatedTotalSize();
  j["percentage_complete_worst"] = spreadjob->getWorstCompletionPercentage();
  j["percentage_complete_average"] = spreadjob->getAverageCompletionPercentage();
  j["percentage_complete_best"] = spreadjob->getBestCompletionPercentage();
  http::Response response(200);
  std::string jsondump = j.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSpreadJobsGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json joblist = nlohmann::json::array();
  std::list<std::shared_ptr<Race>>::const_iterator it;
  bool filterstatus = request.hasQueryParam("status");
  bool filtersite = request.hasQueryParam("site");
  bool filterprofile = request.hasQueryParam("profile");
  bool filtersection = request.hasQueryParam("section");
  bool filtername = request.hasQueryParam("name");
  std::string statusstr = request.getQueryParamValue("status");
  std::string namestr = request.getQueryParamValue("name");
  RaceStatus status = RaceStatus::DONE;
  if (filterstatus) {
    try {
      status = stringToRaceStatus(statusstr);
    }
    catch (std::range_error& e) {
      cb->requestHandled(connrequestid, badRequestResponse(e.what()));
      return;
    }
  }
  std::string sectionstr = request.getQueryParamValue("section");
  std::string profilestr = request.getQueryParamValue("profile");
  SpreadProfile profile = SPREAD_RACE;
  if (filterprofile) {
    try {
      profile = stringToProfile(profilestr);
    }
    catch (std::range_error& e) {
      cb->requestHandled(connrequestid, badRequestResponse(e.what()));
      return;
    }
  }
  if (filtersection && !global->getSectionManager()->getSection(sectionstr)) {
    cb->requestHandled(connrequestid, badRequestResponse("Unknown section: " + sectionstr));
    return;
  }
  std::string sitestr = request.getQueryParamValue("site");
  if (filtersite && !global->getSiteManager()->getSite(sitestr)) {
    cb->requestHandled(connrequestid, badRequestResponse("No such site: " + sitestr));
    return;
  }
  for (it = global->getEngine()->getRacesBegin(); it != global->getEngine()->getRacesEnd(); ++it) {
    if ((filterstatus && (*it)->getStatus() != status) ||
        (filtersite && (*it)->getSiteRace(sitestr) == nullptr) ||
        (filterprofile && (*it)->getProfile() != profile) ||
        (filtersection && (*it)->getSection() != sectionstr) ||
        (filtername && !util::wildcmp(namestr.c_str(), (*it)->getName().c_str())))
    {
      continue;
    }
    joblist.push_back((*it)->getName());
  }
  http::Response response(200);
  std::string jsondump = joblist.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSpreadJobReset(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  auto hardit = jsondata.find("hard");
  bool hard = hardit != jsondata.end() && hardit->get<bool>();
  Path path = request.getPath();
  std::string spreadjob = path.level(1).toString();
  std::shared_ptr<Race> race = global->getEngine()->getRace(spreadjob);
  if (!race) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  global->getEngine()->resetRace(race, hard);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSpreadJobAbort(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  DeleteMode deletemode(DeleteMode::NONE);
  auto deleteit = jsondata.find("delete");
  if (deleteit != jsondata.end()) {
    deletemode = stringToDeleteMode(*deleteit);
  }
  std::list<std::string> sites;
  auto sitesit = jsondata.find("sites");
  if (sitesit != jsondata.end()) {
    for (auto it = sitesit->begin(); it != sitesit->end(); ++it) {
      sites.push_back(*it);
    }
  }
  Path path = request.getPath();
  std::string spreadjob = path.level(1).toString();
  std::shared_ptr<Race> race = global->getEngine()->getRace(spreadjob);
  if (!race) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  if (sites.empty()) {
    switch (deletemode) {
      case DeleteMode::NONE:
        global->getEngine()->abortRace(race);
        break;
      case DeleteMode::INCOMPLETE:
        global->getEngine()->deleteOnAllSites(race, false, false);
        break;
      case DeleteMode::OWN:
        global->getEngine()->deleteOnAllSites(race, false, true);
        break;
      case DeleteMode::ALL:
        global->getEngine()->deleteOnAllSites(race, true, true);
        break;
    }
  }
  else {
    for (const std::string& site : sites) {
      switch (deletemode) {
        case DeleteMode::NONE:
          global->getEngine()->removeSiteFromRace(race, site);
          break;
        case DeleteMode::INCOMPLETE:
          global->getEngine()->removeSiteFromRaceDeleteFiles(race, site, false, false);
          break;
        case DeleteMode::OWN:
          global->getEngine()->removeSiteFromRaceDeleteFiles(race, site, false, true);
          break;
        case DeleteMode::ALL:
          global->getEngine()->removeSiteFromRaceDeleteFiles(race, site, true, true);
          break;
      }
    }
  }
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSiteSectionsGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sitename = path.level(1).toString();
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
  if (!site) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json sec = jsonSiteSections(site);
  http::Response response(200);
  std::string jsondump = sec.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSiteSectionGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sitename = path.level(1).toString();
  std::string section = path.level(3).toString();
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
  if (!site || !site->hasSection(section)) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json sec;
  sec["name"] = section;
  sec["path"] = site->getSectionPath(section).toString();
  http::Response response(200);
  std::string jsondump = sec.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSiteSectionPost(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sitename = path.level(1).toString();
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
  if (!site) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json jsondata = getJsonFromBody(request);
  std::string section = jsondata["name"];
  std::string sectionpath = jsondata["path"];
  if (site->hasSection(section)) {
    http::Response response(409);
    response.appendHeader("Content-Length", "0");
    cb->requestHandled(connrequestid, response);
    return;
  }
  if (sectionpath.empty()) {
    cb->requestHandled(connrequestid, emptySectionPathResponse());
    return;
  }
  site->addSection(section, sectionpath);
  http::Response response(201);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSiteSectionPatch(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sitename = path.level(1).toString();
  std::string section = path.level(3).toString();
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
  if (!site || !site->hasSection(section)) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json jsondata = getJsonFromBody(request);
  std::string sectionpath = jsondata["path"];
  if (sectionpath.empty()) {
    cb->requestHandled(connrequestid, emptySectionPathResponse());
    return;
  }
  site->removeSection(section);
  site->addSection(section, sectionpath);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSiteSectionDelete(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sitename = path.level(1).toString();
  std::string section = path.level(3).toString();
  std::shared_ptr<Site> site = global->getSiteManager()->getSite(sitename);
  if (!site) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  site->removeSection(section);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleTransferJobsGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json joblist = nlohmann::json::array();
  bool filterstatus = request.hasQueryParam("status");
  bool filtersite = request.hasQueryParam("site");
  bool filtersrcsite = request.hasQueryParam("src_site");
  bool filterdstsite = request.hasQueryParam("dst_site");
  bool filtertype = request.hasQueryParam("type");
  bool filtername = request.hasQueryParam("name");
  std::string statusstr = request.getQueryParamValue("status");
  std::string sitestr = request.getQueryParamValue("site");
  std::string srcsitestr = request.getQueryParamValue("src_site");
  std::string dstsitestr = request.getQueryParamValue("dst_site");
  std::string typestr = request.getQueryParamValue("type");
  std::string namestr = request.getQueryParamValue("name");
  bool showid = getQueryParamBoolValue(request, "id");
  TransferJobStatus status = TRANSFERJOB_QUEUED;
  if (filterstatus) {
    try {
      status = stringToTransferJobStatus(statusstr);
    }
    catch (std::range_error& e) {
      cb->requestHandled(connrequestid, badRequestResponse(e.what()));
      return;
    }
  }
  TransferJobType type = TRANSFERJOB_FXP;
  if (filtertype) {
    try {
      type = stringToTransferJobType(typestr);
    }
    catch (std::range_error& e) {
      cb->requestHandled(connrequestid, badRequestResponse(e.what()));
      return;
    }
  }
  if ((filtersite && !global->getSiteManager()->getSite(sitestr)) ||
      (filtersrcsite && !global->getSiteManager()->getSite(srcsitestr)) ||
      (filterdstsite && !global->getSiteManager()->getSite(dstsitestr)))
  {
    cb->requestHandled(connrequestid, badRequestResponse("No such site: " + sitestr));
    return;
  }
  std::list<std::shared_ptr<TransferJob>>::const_iterator it;
  for (it = global->getEngine()->getTransferJobsBegin(); it != global->getEngine()->getTransferJobsEnd(); ++it) {
    if ((filterstatus && (*it)->getStatus() != status) ||
        (filtersite && (!(*it)->getDst() || (*it)->getDst()->getSite()->getName() != sitestr) && (!(*it)->getSrc() || (*it)->getSrc()->getSite()->getName() != sitestr)) ||
        (filtersrcsite && (!(*it)->getSrc() || (*it)->getSrc()->getSite()->getName() != srcsitestr)) ||
        (filterdstsite && (!(*it)->getDst() || (*it)->getDst()->getSite()->getName() != dstsitestr)) ||
        (filtertype && (*it)->getType() != type) ||
        (filtername && !util::wildcmp(namestr.c_str(), (*it)->getName().c_str())))
    {
      continue;
    }
    if (showid) {
      nlohmann::json entry;
      entry["name"] = (*it)->getName();
      entry["id"] = (*it)->getId();
      joblist.push_back(entry);
    }
    else {
      joblist.push_back((*it)->getName());
    }
  }

  http::Response response(200);
  std::string jsondump = joblist.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleTransferJobPost(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  auto srcsiteit = jsondata.find("src_site");
  auto dstsiteit = jsondata.find("dst_site");
  std::shared_ptr<SiteLogic> srcsl;
  std::shared_ptr<SiteLogic> dstsl;
  if (srcsiteit != jsondata.end()) {
    srcsl = global->getSiteLogicManager()->getSiteLogic(*srcsiteit);
    if (!srcsl) {
      cb->requestHandled(connrequestid, badRequestResponse("Site not found: " + srcsiteit->get<std::string>()));
      return;
    }
  }
  if (dstsiteit != jsondata.end()) {
    dstsl = global->getSiteLogicManager()->getSiteLogic(*dstsiteit);
    if (!dstsl) {
      cb->requestHandled(connrequestid, badRequestResponse("Site not found: " + dstsiteit->get<std::string>()));
      return;
    }
  }
  if (!srcsl && !dstsl) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing keys: src_site || dst_site"));
    return;
  }
  std::string srcpath;
  std::string srcsection;
  auto srcsectionit = jsondata.find("src_section");
  if (srcsectionit != jsondata.end()) {
    srcsection = *srcsectionit;
    if (!srcsl) {
      cb->requestHandled(connrequestid, badRequestResponse("src_section can not be used without src_site"));
      return;
    }
    if (!srcsl->getSite()->hasSection(*srcsectionit)) {
      cb->requestHandled(connrequestid, badRequestResponse("Section " + srcsectionit->get<std::string>() + " is not defined on site " + srcsl->getSite()->getName()));
      return;
    }
    srcpath = srcsl->getSite()->getSectionPath(*srcsectionit).toString();
  }
  auto srcpathit = jsondata.find("src_path");
  if (srcpathit != jsondata.end()) {
    std::string section;
    Path path(*srcpathit);
    if (srcsl) {
      util::Result res = SectionUtil::useOrSectionTranslate(Path(*srcpathit), srcsl->getSite(), section, path);
      if (!res.success) {
        cb->requestHandled(connrequestid, badRequestResponse(res.error));
        return;
      }
    }
    srcpath = path.toString();
    if (!section.empty()) {
      srcsection = section;
    }
  }
  if (srcpath.empty()) {
    if (!srcsl) { // omitting src_path is ok for upload jobs
       srcpath = global->getLocalStorage()->getDownloadPath().toString();
    }
    else {
      cb->requestHandled(connrequestid, badRequestResponse("Missing keys: src_path || src_section"));
      return;
    }
  }
  std::string dstpath;
  std::string dstsection;
  auto dstsectionit = jsondata.find("dst_section");
  if (dstsectionit != jsondata.end()) {
    dstsection = *dstsectionit;
    if (!dstsl) {
      cb->requestHandled(connrequestid, badRequestResponse("dst_section can not be used without dst_site"));
      return;
    }
    if (!dstsl->getSite()->hasSection(*dstsectionit)) {
      cb->requestHandled(connrequestid, badRequestResponse("Section " + dstsectionit->get<std::string>() + " is not defined on site " + dstsl->getSite()->getName()));
      return;
    }
    dstpath = dstsl->getSite()->getSectionPath(*dstsectionit).toString();
  }
  auto dstpathit = jsondata.find("dst_path");
  if (dstpathit != jsondata.end()) {
    std::string section;
    Path path(*dstpathit);
    if (dstsl) {
      util::Result res = SectionUtil::useOrSectionTranslate(Path(*dstpathit), dstsl->getSite(), section, path);
      if (!res.success) {
        cb->requestHandled(connrequestid, badRequestResponse(res.error));
        return;
      }
    }
    dstpath = path.toString();
    if (!section.empty()) {
      dstsection = section;
    }
  }
  if (dstpath.empty()) {
    if (!dstsl) { // omitting dst_path is ok for download jobs
      dstpath = global->getLocalStorage()->getDownloadPath().toString();
    }
    else {
      cb->requestHandled(connrequestid, badRequestResponse("Missing key: dst_path || dst_section"));
      return;
    }
  }
  std::string name;
  auto nameit = jsondata.find("name");
  if (nameit != jsondata.end()) {
    name = *nameit;
  }
  else {
    cb->requestHandled(connrequestid, badRequestResponse("Missing key: name"));
    return;
  }

  unsigned int id = 0;
  if (srcsl && dstsl) {
    id = global->getEngine()->newTransferJobFXP(srcsl->getSite()->getName(), srcpath, srcsection, name, dstsl->getSite()->getName(), dstpath, dstsection, name);
  }
  else if (srcsl && !dstsl) {
    id = global->getEngine()->newTransferJobDownload(srcsl->getSite()->getName(), srcpath, srcsection, name, dstpath, name);
  }
  else if (!srcsl && dstsl) {
    id = global->getEngine()->newTransferJobUpload(srcpath, name, dstsl->getSite()->getName(), dstpath, dstsection, name);
  }
  http::Response response(201);
  if (id == 0) {
    response.setStatusCode(503);
    response.appendHeader("Content-Length", "0");
  }
  else {
    nlohmann::json j;
    j["id"] = id;
    std::string jsondump = j.dump(2);
    response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
    response.addHeader("Content-Type", "application/json");
  }
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleTransferJobGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  bool byid = getQueryParamBoolValue(request, "id");
  std::string name = Path(request.getPath()).baseName();
  std::shared_ptr<TransferJob> transferjob = byid ? global->getEngine()->getTransferJob(std::stol(name)) : global->getEngine()->getTransferJob(name);
  if (!transferjob) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json j;
  int type = transferjob->getType();
  j["type"] = transferJobTypeToString(transferjob->getType());
  if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_DOWNLOAD) {
    j["src_site"] = transferjob->getSrc()->getSite()->getName();
  }
  if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_UPLOAD) {
    j["dst_site"] = transferjob->getDst()->getSite()->getName();
  }
  j["name"] = transferjob->getName();
  j["id"] = transferjob->getId();
  j["status"] = transferJobStatusToString(transferjob->getStatus());
  j["src_path"] = transferjob->getSrcPath().toString();
  j["dst_path"] = transferjob->getDstPath().toString();
  j["time_started"] = transferjob->timeStarted();
  j["time_spent_seconds"] = transferjob->timeSpent();
  j["time_remaining_seconds"] = transferjob->timeRemaining();
  j["size_progress_bytes"] = transferjob->sizeProgress();
  j["size_estimated_bytes"] = transferjob->totalSize();
  j["files_progress"] = transferjob->filesProgress();
  j["files_total"] = transferjob->filesTotal();
  j["percentage_complete"] = transferjob->getProgress();

  http::Response response(200);
  std::string jsondump = j.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleTransferJobReset(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  bool byid = getQueryParamBoolValue(request, "id");
  Path path = request.getPath();
  std::string transferjob = path.level(1).toString();
  std::shared_ptr<TransferJob> tj = byid ? global->getEngine()->getTransferJob(std::stol(transferjob)) : global->getEngine()->getTransferJob(transferjob);
  if (!tj) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  global->getEngine()->resetTransferJob(tj);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleTransferJobAbort(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  bool byid = getQueryParamBoolValue(request, "id");
  Path path = request.getPath();
  std::string transferjob = path.level(1).toString();
  std::shared_ptr<TransferJob> tj = byid ? global->getEngine()->getTransferJob(std::stol(transferjob)) : global->getEngine()->getTransferJob(transferjob);
  if (!tj) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  global->getEngine()->abortTransferJob(tj);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSectionsGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json j = nlohmann::json::array();
  for (std::unordered_map<std::string, Section>::const_iterator it = global->getSectionManager()->begin(); it != global->getSectionManager()->end(); ++it) {
    j.push_back(jsonSection(it->second));
  }
  http::Response response(200);
  std::string jsondump = j.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSectionPost(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  nlohmann::json jsondata = getJsonFromBody(request);
  auto nameit = jsondata.find("name");
  if (nameit == jsondata.end()) {
    cb->requestHandled(connrequestid, badRequestResponse("Missing key: name"));
    return;
  }
  if (global->getSectionManager()->getSection(static_cast<std::string>(*nameit))) {
    http::Response response(409);
    response.appendHeader("Content-Length", "0");
    cb->requestHandled(connrequestid, response);
    return;
  }
  Section section;
  updateSection(&section, jsondata);
  global->getSectionManager()->addSection(section);
  http::Response response(201);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSectionGet(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sectionname = path.level(1).toString();
  Section* section = global->getSectionManager()->getSection(sectionname);
  if (!section) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  nlohmann::json sec = jsonSection(*section);
  http::Response response(200);
  std::string jsondump = sec.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  response.addHeader("Content-Type", "application/json");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSectionPatch(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sectionname = path.level(1).toString();
  Section* sectionptr = global->getSectionManager()->getSection(sectionname);
  if (!sectionptr) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  Section section = *sectionptr;
  nlohmann::json jsondata = getJsonFromBody(request);
  updateSection(&section, jsondata);
  if (!global->getSectionManager()->replaceSection(section, sectionname)) {
    http::Response response(409);
    response.appendHeader("Content-Length", "0");
    cb->requestHandled(connrequestid, response);
    return;
  }
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

void RestApi::handleSectionDelete(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  Path path = request.getPath();
  std::string sectionname = path.level(1).toString();
  global->getSectionManager()->removeSection(sectionname);
  http::Response response(204);
  response.appendHeader("Content-Length", "0");
  cb->requestHandled(connrequestid, response);
}

std::string RestApi::createTemporaryAuthToken() {
  srand(time(NULL));
  Core::BinaryData data;
  data.resize(16);
  for (int i = 0; i < 16; ++i) {
    data[i] = static_cast<unsigned char>(rand() % 256);
  }
  std::string token = Crypto::toHex(data);
  tempauthtokens.insert(token);
  return token;
}

void RestApi::removeTemporaryAuthToken(const std::string& token) {
  tempauthtokens.erase(token);
}

void RestApi::cancelOngoingSyncRequests(RestApiCallback* cb) {
  for (std::list<OngoingRequest>::iterator it = ongoingrequests.begin(); it != ongoingrequests.end();) {
    if (it->cb == cb && !it->async) {
      it = ongoingrequests.erase(it);
    }
    else {
      ++it;
    }
  }
}

void RestApi::handleRequest(RestApiCallback* cb, int connrequestid, const http::Request& request) {
  bool authorized = false;
  if (request.hasHeader("Authorization")) {
    std::string encoded = request.getHeaderValue("Authorization");
    if (!encoded.compare(0, 6, "Basic ")) {
      Core::BinaryData out;
      Crypto::base64Decode(Core::BinaryData(encoded.begin() + 6, encoded.end()), out);
      std::string decoded(out.begin(), out.end());
      size_t split = decoded.find(':');
      if (split != std::string::npos) {
        std::string password = decoded.substr(split + 1);
        if (password == global->getRemoteCommandHandler()->getPassword() || tempauthtokens.find(password) != tempauthtokens.end()) {
          authorized = true;
        }
      }
    }
  }
  if (!authorized) {
    http::Response response(401);
    response.addHeader("WWW-Authenticate", "Basic");
    response.addHeader("Content-Length", "0");
    cb->requestHandled(connrequestid, response);
    return;
  }
  Path path(request.getPath());
  int levels = path.levels();
  global->getEventLog()->log("RestApi", "Received request for: " + request.getMethod() + " " + path.toString());
  std::map<Path, std::map<std::string, EndpointPointer>>::iterator it;
  for (it = endpoints.begin(); it != endpoints.end(); it++) {
    const Path& endpointpath = it->first;
    if (endpointpath.levels() != levels) {
      continue;
    }
    bool mismatch = false;
    for (int i = 0; i < levels; i++) {
      if (endpointpath.level(i) != path.level(i) && endpointpath.level(i) != "*") {
        mismatch = true;
        break;
      }
    }
    if (!mismatch) {
      break;
    }
  }
  if (it == endpoints.end()) {
    cb->requestHandled(connrequestid, notFoundResponse());
    return;
  }
  std::map<std::string, EndpointPointer>::iterator it2 = it->second.find(request.getMethod());
  if (it2 == it->second.end()) {
    http::Response response(405);
    response.appendHeader("Content-Length", "0");
    cb->requestHandled(connrequestid, response);
    return;
  }
  try {
    (this->*(it2->second))(cb, connrequestid, request);
  }
  catch (nlohmann::json::exception& e) {
    cb->requestHandled(connrequestid, badRequestResponse(e.what()));
    return;
  }
  catch (std::range_error& e) {
    cb->requestHandled(connrequestid, badRequestResponse(e.what()));
    return;
  }
  catch (std::invalid_argument& e) {
    cb->requestHandled(connrequestid, badRequestResponse(e.what()));
    return;
  }
}

OngoingRequest* RestApi::findOngoingRequest(void* service, int servicerequestid) {
  std::pair<void*, int> pair = std::make_pair(service, servicerequestid);
  for (OngoingRequest& request : ongoingrequests) {
    if (request.ongoingservicerequests.find(pair) != request.ongoingservicerequests.end()) {
      return &request;
    }
  }
  return nullptr;
}

OngoingRequest* RestApi::findOngoingRequest(int apirequestid) {
  for (OngoingRequest& request : ongoingrequests) {
    if (request.apirequestid == apirequestid) {
      return &request;
    }
  }
  return nullptr;
}

void RestApi::requestReady(void* service, int servicerequestid) {
  OngoingRequest* request = findOngoingRequest(service, servicerequestid);
  if (!request) {
    return;
  }

  switch (request->type) {
    case OngoingRequestType::RAW_COMMAND: {
      SiteLogic* sl = static_cast<SiteLogic*>(service);
      bool status = sl->requestStatus(servicerequestid);
      if (!status) {
        request->failures.emplace_back(service, "failed");
      }
      else {
        std::string result = sl->getRawCommandResult(servicerequestid);
        request->successes.emplace_back(service, result);
      }
      sl->finishRequest(servicerequestid);
      request->ongoingservicerequests.erase(std::make_pair(service, servicerequestid));
      if (request->ongoingservicerequests.empty() && !request->async) {
        finalize(*request);
      }
      return;
    }
    case OngoingRequestType::FILE_LIST: {
      SiteLogic* sl = static_cast<SiteLogic*>(service);
      bool status = sl->requestStatus(servicerequestid);
      if (!status) {
        http::Response response(502);
        response.appendHeader("Content-Length", "0");
        request->cb->requestHandled(request->connrequestid, response);
      }
      else {
        FileListData* data = sl->getFileListData(servicerequestid);
        std::shared_ptr<FileList> fl = data->getFileList();
        nlohmann::json j = nlohmann::json::array();
        for (std::list<File*>::const_iterator it = fl->begin(); it != fl->end(); ++it) {
          File* f = *it;
          nlohmann::json jf;
          jf["name"] = f->getName();
          jf["size"] = f->getSize();
          jf["user"] = f->getOwner();
          jf["group"] = f->getGroup();
          jf["type"] = f->isDirectory() ? "DIR" : (f->isLink() ? "LINK" : "FILE");
          jf["last_modified"] = f->getLastModified();
          if (f->isLink()) {
            jf["link_target"] = f->getLinkTarget();
          }
          j.push_back(jf);
        }
        http::Response response(200);
        std::string jsondump = j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace);
        response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
        response.addHeader("Content-Type", "application/json");
        request->cb->requestHandled(request->connrequestid, response);
      }
      sl->finishRequest(servicerequestid);
      for (std::list<OngoingRequest>::iterator it = ongoingrequests.begin(); it != ongoingrequests.end(); ++it) {
        if (&*it == request) {
          ongoingrequests.erase(it);
          break;
        }
      }
      return;
    }
    case OngoingRequestType::DOWNLOAD_FILE: {
      SiteLogic* sl = static_cast<SiteLogic*>(service);
      bool status = sl->requestStatus(servicerequestid);
      if (!status) {
        http::Response response(502);
        response.appendHeader("Content-Length", "0");
        request->cb->requestHandled(request->connrequestid, response);
      }
      else {
        DownloadFileData* data = sl->getDownloadFileData(servicerequestid);
        http::Response response(200);
        response.setBody(std::vector<char>(data->data.begin(), data->data.end()));
        request->cb->requestHandled(request->connrequestid, response);
      }
      sl->finishRequest(servicerequestid);
      for (std::list<OngoingRequest>::iterator it = ongoingrequests.begin(); it != ongoingrequests.end(); ++it) {
        if (&*it == request) {
          ongoingrequests.erase(it);
          break;
        }
      }
      return;
    }
  }
}

void RestApi::finalize(OngoingRequest& request) {
  switch (request.type) {
    case OngoingRequestType::RAW_COMMAND: {
      nlohmann::json successlist = nlohmann::json::array();
      nlohmann::json failurelist = nlohmann::json::array();
      for (const std::pair<void*, std::string>& success : request.successes) {
        SiteLogic* sl = static_cast<SiteLogic*>(success.first);
        nlohmann::json site;
        site["name"] = sl->getSite()->getName();
        site["result"] = success.second;
        successlist.push_back(site);
      }
      for (const std::pair<void*, int>& ongoing : request.ongoingservicerequests) {
        request.failures.emplace_back(ongoing.first, "timeout");
      }
      for (const std::pair<void*, std::string>& failure : request.failures) {
        SiteLogic* sl = static_cast<SiteLogic*>(failure.first);
        nlohmann::json site;
        site["name"] = sl->getSite()->getName();
        site["reason"] = failure.second;
        failurelist.push_back(site);
      }
      nlohmann::json j;
      j["successes"] = successlist;
      j["failures"] = failurelist;
      http::Response response(200);
      std::string jsondump = j.dump(2, ' ', false, nlohmann::json::error_handler_t::replace);
      response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
      response.addHeader("Content-Type", "application/json");
      request.cb->requestHandled(request.connrequestid, response);
      break;
    }
    case OngoingRequestType::FILE_LIST:
    case OngoingRequestType::DOWNLOAD_FILE:
      http::Response response(504);
      response.appendHeader("Content-Length", "0");
      request.cb->requestHandled(request.connrequestid, response);
      break;
  }
  for (std::list<OngoingRequest>::iterator it = ongoingrequests.begin(); it != ongoingrequests.end(); ++it) {
    if (&*it == &request) {
      ongoingrequests.erase(it);
      break;
    }
  }
}

void RestApi::respondAsynced(OngoingRequest& request) {
  http::Response response(202);
  nlohmann::json j;
  j["request_id"] = request.apirequestid;
  response.addHeader("Content-Type", "application/json");
  std::string jsondump = j.dump(2);
  response.setBody(std::vector<char>(jsondump.begin(), jsondump.end()));
  request.cb->requestHandled(request.connrequestid, response);
}

void RestApi::tick(int message) {
  std::list<OngoingRequest*> finalizerequests;
  for (OngoingRequest& request : ongoingrequests) {
    request.timepassed += RESTAPI_TICK_INTERVAL_MS;
    if (request.timepassed > request.timeout * 1000) {
      finalizerequests.push_back(&request);
    }
  }
  for (OngoingRequest* request : finalizerequests) {
    finalize(*request);
  }
}
