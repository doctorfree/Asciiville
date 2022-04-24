#include "remotecommandhandler.h"

#include <vector>
#include <list>

#include "core/tickpoke.h"
#include "core/iomanager.h"
#include "core/types.h"
#include "crypto.h"
#include "globalcontext.h"
#include "engine.h"
#include "eventlog.h"
#include "util.h"
#include "sitelogicmanager.h"
#include "sitelogic.h"
#include "uibase.h"
#include "sitemanager.h"
#include "site.h"
#include "race.h"
#include "localstorage.h"
#include "httpserver.h"
#include "sectionutil.h"

#define DEFAULT_PASS "DEFAULT"
#define RETRY_DELAY 30000

namespace {

enum RaceType {
  RACE,
  DISTRIBUTE,
  PREPARE
};

std::list<std::shared_ptr<SiteLogic> > getSiteLogicList(const std::string & sitestring) {
  std::list<std::shared_ptr<SiteLogic> > sitelogics;
  std::list<std::string> sites;
  if (sitestring == "*") {
    std::vector<std::shared_ptr<Site> >::const_iterator it;
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      if (!(*it)->getDisabled()) {
        sites.push_back((*it)->getName());
      }
    }
  }
  else {
    sites = util::trim(util::split(sitestring, ","));
  }
  std::list<std::string> notfoundsites;
  for (std::list<std::string>::const_iterator it = sites.begin(); it != sites.end(); it++) {
    const std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(*it);
    if (!sl) {
      notfoundsites.push_back(*it);
      continue;
    }
    sitelogics.push_back(sl);
  }
  if (sitelogics.empty()) {
    for (std::vector<std::shared_ptr<Site> >::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); ++it) {
      if ((*it)->hasSection(sitestring) && !(*it)->getDisabled()) {
        std::shared_ptr<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic((*it)->getName());
        sitelogics.push_back(sl);
      }
    }
  }
  if (!notfoundsites.empty()) {
    std::string notfound = util::join(notfoundsites, ",");
    if (sites.size() == 1) {
      global->getEventLog()->log("RemoteCommandHandler", "Site or section not found: " + notfound);
    }
    else {
      global->getEventLog()->log("RemoteCommandHandler", "Sites not found: " + notfound);
    }
  }
  return sitelogics;
}

}

RemoteCommandHandler::RemoteCommandHandler() :
  enabled(false),
  encrypted(true),
  password(DEFAULT_PASS),
  port(DEFAULT_API_PORT),
  retrying(false),
  connected(false),
  notify(RemoteCommandNotify::DISABLED) {
}

bool RemoteCommandHandler::isEnabled() const {
  return enabled;
}

bool RemoteCommandHandler::isEncrypted() const {
  return encrypted;
}

int RemoteCommandHandler::getUDPPort() const {
  return port;
}

std::string RemoteCommandHandler::getPassword() const {
  return password;
}

void RemoteCommandHandler::setPassword(const std::string & newpass) {
  password = newpass;
}

void RemoteCommandHandler::setPort(int newport) {
  bool reopen = !(port == newport || !enabled);
  port = newport;
  if (reopen) {
    setEnabled(false);
    setEnabled(true);
  }
}

RemoteCommandNotify RemoteCommandHandler::getNotify() const {
  return notify;
}

void RemoteCommandHandler::setNotify(RemoteCommandNotify notify) {
  this->notify = notify;
}

void RemoteCommandHandler::connect() {
  int udpport = getUDPPort();
  sockid = global->getIOManager()->registerUDPServerSocket(this, udpport);
  if (sockid >= 0) {
    connected = true;
    global->getEventLog()->log("RemoteCommandHandler", "Listening on UDP port " + std::to_string(udpport));
  }
  else {
    int delay = RETRY_DELAY / 1000;
    global->getEventLog()->log("RemoteCommandHandler", "Retrying in " + std::to_string(delay) + " seconds.");
    retrying = true;
    global->getTickPoke()->startPoke(this, "RemoteCommandHandler", RETRY_DELAY, 0);
  }
}

void RemoteCommandHandler::FDData(int sockid, char * data, unsigned int datalen) {
  std::string message;
  if (encrypted) {
    Core::BinaryData encrypted(datalen);
    memcpy(encrypted.data(), data, datalen);
    Core::BinaryData decrypted;
    Core::BinaryData key(password.begin(), password.end());
    Crypto::decrypt(encrypted, key, decrypted);
    if (!Crypto::isMostlyASCII(decrypted)) {
      global->getEventLog()->log("RemoteCommandHandler", "Received " + std::to_string(datalen) + " bytes of garbage or wrongly encrypted data");
      return;
    }
    message = std::string(decrypted.begin(), decrypted.end());
  }
  else {
    message = std::string(data, datalen);
  }
  handleMessage(message);
}

void RemoteCommandHandler::handleMessage(const std::string & message) {
  std::string trimmedmessage = util::trim(message);
  std::vector<std::string> tokens = util::splitVec(trimmedmessage);
  if (tokens.size() < 2) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad message format: " + trimmedmessage);
    return;
  }
  std::string & pass = tokens[0];
  bool passok = pass == password;
  if (passok) {
    for (unsigned int i = 0; i < pass.length(); i++) {
      pass[i] = '*';
    }
  }
  global->getEventLog()->log("RemoteCommandHandler", "Received: " + util::join(tokens));
  if (!passok) {
    global->getEventLog()->log("RemoteCommandHandler", "Invalid password.");
    return;
  }
  std::string command = tokens[1];
  std::vector<std::string> remainder(tokens.begin() + 2, tokens.end());
  bool notification = notify == RemoteCommandNotify::ALL_COMMANDS;
  if (command == "race") {
    bool started = commandRace(remainder);
    if (started && notify >= RemoteCommandNotify::JOBS_ADDED) {
      notification = true;
    }
  }
  else if (command == "distribute") {
    bool started = commandDistribute(remainder) && notify >= RemoteCommandNotify::JOBS_ADDED;
    if (started && notify >= RemoteCommandNotify::JOBS_ADDED) {
      notification = true;
    }
  }
  else if (command == "prepare") {
    bool created = commandPrepare(remainder);
    if (created && notify >= RemoteCommandNotify::ACTION_REQUESTED) {
      notification = true;
    }
  }
  else if (command == "raw") {
    commandRaw(remainder);
  }
  else if (command == "rawwithpath") {
    commandRawWithPath(remainder);
  }
  else if (command == "fxp") {
    bool started = commandFXP(remainder);
    if (started && notify >= RemoteCommandNotify::JOBS_ADDED) {
      notification = true;
    }
  }
  else if (command == "download") {
    bool started = commandDownload(remainder);
    if (started && notify >= RemoteCommandNotify::JOBS_ADDED) {
      notification = true;
    }
  }
  else if (command == "upload") {
    bool started = commandUpload(remainder);
    if (started && notify >= RemoteCommandNotify::JOBS_ADDED) {
      notification = true;
    }
  }
  else if (command == "idle") {
    commandIdle(remainder);
  }
  else if (command == "abort") {
    commandAbort(remainder);
  }
  else if (command == "delete") {
    commandDelete(remainder);
  }
  else if (command == "abortdeleteincomplete") {
    commandAbortDeleteIncomplete(remainder);
  }
  else if(command == "reset") {
    commandReset(remainder, false);
  }
  else if(command == "hardreset") {
    commandReset(remainder, true);
  }
  else {
    global->getEventLog()->log("RemoteCommandHandler", "Invalid remote command: " + util::join(tokens));
    return;
  }
  if (notification) {
    global->getUIBase()->notify();
  }
}

bool RemoteCommandHandler::commandRace(const std::vector<std::string> & message) {
  return parseRace(message, RACE);
}

bool RemoteCommandHandler::commandDistribute(const std::vector<std::string> & message) {
  return parseRace(message, DISTRIBUTE);
}

bool RemoteCommandHandler::commandPrepare(const std::vector<std::string> & message) {
  return parseRace(message, PREPARE);
}

void RemoteCommandHandler::commandRaw(const std::vector<std::string> & message) {
  if (message.size() < 2) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote raw command format: " + util::join(message));
    return;
  }
  std::string sitestring = message[0];
  std::string rawcommand = util::join(std::vector<std::string>(message.begin() + 1, message.end()));

  std::list<std::shared_ptr<SiteLogic> > sites = getSiteLogicList(sitestring);

  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
    (*it)->requestRawCommand(nullptr, rawcommand);
  }
}

void RemoteCommandHandler::commandRawWithPath(const std::vector<std::string> & message) {
  if (message.size() < 3) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote rawwithpath command format: " + util::join(message));
    return;
  }
  std::string sitestring = message[0];
  std::string pathstr = message[1];
  std::string rawcommand = util::join(std::vector<std::string>(message.begin() + 2, message.end()));

  std::list<std::shared_ptr<SiteLogic> > sites = getSiteLogicList(sitestring);

  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
    Path path;
    util::Result res = SectionUtil::useOrSectionTranslate(Path(pathstr), (*it)->getSite(), path);
    if (!res.success) {
      global->getEventLog()->log("RemoteCommandHandler", res.error);
      continue;
    }
    (*it)->requestRawCommand(nullptr, path, rawcommand);
  }
}

bool RemoteCommandHandler::commandFXP(const std::vector<std::string> & message) {
  if (message.size() < 5) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote fxp command format: " + util::join(message));
    return false;
  }
  std::shared_ptr<SiteLogic> srcsl = global->getSiteLogicManager()->getSiteLogic(message[0]);
  std::shared_ptr<SiteLogic> dstsl = global->getSiteLogicManager()->getSiteLogic(message[3]);
  if (!srcsl) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad site name: " + message[0]);
    return false;
  }
  if (!dstsl) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad site name: " + message[3]);
    return false;
  }
  std::string dstfile = message.size() > 5 ? message[5] : message[2];

  Path srcpath;
  std::string srcsection;
  util::Result res = SectionUtil::useOrSectionTranslate(message[1], srcsl->getSite(), srcsection, srcpath);
  if (!res.success) {
    global->getEventLog()->log("RemoteCommandHandler", res.error);
    return false;
  }
  Path dstpath;
  std::string dstsection;
  res = SectionUtil::useOrSectionTranslate(message[4], dstsl->getSite(), dstsection, dstpath);
  if (!res.success) {
    global->getEventLog()->log("RemoteCommandHandler", res.error);
    return false;
  }
  global->getEngine()->newTransferJobFXP(message[0], srcpath, srcsection, message[2], message[3], dstpath, dstsection, dstfile);
  return true;
}

bool RemoteCommandHandler::commandDownload(const std::vector<std::string> & message) {
  if (message.size() < 2) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad download command format: " + util::join(message));
    return false;
  }
  std::shared_ptr<SiteLogic> srcsl = global->getSiteLogicManager()->getSiteLogic(message[0]);
  if (!srcsl) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad site name: " + message[0]);
    return false;
  }
  Path srcpath;
  std::string srcsection;
  util::Result res = SectionUtil::useOrSectionTranslate(message[1], srcsl->getSite(), srcsection, srcpath);
  if (!res.success) {
    global->getEventLog()->log("RemoteCommandHandler", res.error);
    return false;
  }
  std::string file = srcpath.baseName();
  if (message.size() == 2) {
    srcpath = srcpath.dirName();
  }
  else {
    file = message[2];
  }
  global->getEngine()->newTransferJobDownload(message[0], srcpath, srcsection, file, global->getLocalStorage()->getDownloadPath(), file);
  return true;
}

bool RemoteCommandHandler::commandUpload(const std::vector<std::string> & message) {
  if (message.size() < 3) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad upload command format: " + util::join(message));
    return false;
  }
  Path srcpath = message[0];
  std::string file = srcpath.baseName();
  std::string dstsite;
  std::string dstpathstr;
  if (message.size() == 3) {
    srcpath = srcpath.dirName();
    dstsite = message[1];
    dstpathstr = message[2];
  }
  else {
    file = message[1];
    dstsite = message[2];
    dstpathstr = message[3];
  }
  std::shared_ptr<SiteLogic> dstsl = global->getSiteLogicManager()->getSiteLogic(dstsite);
  if (!dstsl) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad site name: " + dstsite);
    return false;
  }
  Path dstpath;
  std::string dstsection;
  util::Result res = SectionUtil::useOrSectionTranslate(dstpathstr, dstsl->getSite(), dstsection, dstpath);
  if (!res.success) {
    global->getEventLog()->log("RemoteCommandHandler", res.error);
    return false;
  }
  global->getEngine()->newTransferJobUpload(srcpath, file, dstsite, dstpath, dstsection, file);
  return true;
}

void RemoteCommandHandler::commandIdle(const std::vector<std::string> & message) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad idle command format: " + util::join(message));
    return;
  }
  int idletime;
  std::string sitestring;
  if (message.size() < 2) {
    sitestring = message[0];
    idletime = 0;
  }
  else {
    sitestring = message[0];
    idletime = std::stoi(message[1]);
  }

  std::list<std::shared_ptr<SiteLogic> > sites = getSiteLogicList(sitestring);
  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sites.begin(); it != sites.end(); it++) {
    (*it)->requestAllIdle(nullptr, idletime);
  }
}

void RemoteCommandHandler::commandAbort(const std::vector<std::string> & message) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad abort command format: " + util::join(message));
    return;
  }
  std::shared_ptr<Race> race = global->getEngine()->getRace(message[0]);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + message[0]);
    return;
  }
  global->getEngine()->abortRace(race);
}

void RemoteCommandHandler::commandDelete(const std::vector<std::string> & message) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad delete command format: " + util::join(message));
    return;
  }
  std::string release = message[0];
  std::string sitestring;
  if (message.size() >= 2) {
    sitestring = message[1];
  }
  std::shared_ptr<Race> race = global->getEngine()->getRace(release);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + release);
    return;
  }
  if (!sitestring.length()) {
    global->getEngine()->deleteOnAllSites(race, false, true);
    return;
  }
  std::list<std::shared_ptr<SiteLogic> > sitelogics = getSiteLogicList(sitestring);
  std::list<std::shared_ptr<Site> > sites;
  for (std::list<std::shared_ptr<SiteLogic> >::const_iterator it = sitelogics.begin(); it != sitelogics.end(); it++) {
    sites.push_back((*it)->getSite());
  }
  global->getEngine()->deleteOnSites(race, sites, false);
}

void RemoteCommandHandler::commandAbortDeleteIncomplete(const std::vector<std::string> & message) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad abortdeleteincomplete command format: " + util::join(message));
    return;
  }
  std::string release = message[0];
  std::shared_ptr<Race> race = global->getEngine()->getRace(release);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + release);
    return;
  }
  global->getEngine()->deleteOnAllSites(race, false, false);
}

void RemoteCommandHandler::commandReset(const std::vector<std::string> & message, bool hard) {
  if (message.empty()) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad reset command format: " + util::join(message));
    return;
  }
  std::shared_ptr<Race> race = global->getEngine()->getRace(message[0]);
  if (!race) {
    global->getEventLog()->log("RemoteCommandHandler", "No matching race: " + message[0]);
    return;
  }
  global->getEngine()->resetRace(race, hard);
}

bool RemoteCommandHandler::parseRace(const std::vector<std::string> & message, int type) {
  if (message.size() < 3) {
    global->getEventLog()->log("RemoteCommandHandler", "Bad remote race command format: " + util::join(message));
    return false;
  }
  std::string section = message[0];
  std::string release = message[1];
  std::string sitestring = message[2];
  std::list<std::string> sites;
  if (sitestring == "*") {
    for (std::vector<std::shared_ptr<Site> >::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      if ((*it)->hasSection(section) && !(*it)->getDisabled()) {
        sites.push_back((*it)->getName());
      }
    }
  }
  else {
    sites = util::trim(util::split(sitestring, ","));
  }
  std::list<std::string> dlonlysites;
  if (message.size() >= 4) {
    std::string dlonlysitestring = message[3];
    dlonlysites = util::trim(util::split(dlonlysitestring, ","));
  }
  if (type == RACE) {
    return !!global->getEngine()->newRace(release, section, sites, false, dlonlysites);
  }
  else if (type == DISTRIBUTE){
    return !!global->getEngine()->newDistribute(release, section, sites, false, dlonlysites);
  }
  else {
    return global->getEngine()->prepareRace(release, section, sites, false, dlonlysites);
  }
}

void RemoteCommandHandler::FDFail(int sockid, const std::string & message) {
  global->getEventLog()->log("RemoteCommandHandler", "UDP binding on port " +
      std::to_string(getUDPPort()) + " failed: " + message);
}

void RemoteCommandHandler::disconnect() {
  if (connected) {
    global->getIOManager()->closeSocket(sockid);
    global->getEventLog()->log("RemoteCommandHandler", "Closing UDP socket");
    connected = false;
  }
}

void RemoteCommandHandler::setEnabled(bool enabled) {
  if (this->enabled == enabled) {
    return;
  }
  if (retrying) {
    stopRetry();
  }
  if (enabled) {
    connect();
  }
  else {
    disconnect();
  }
  this->enabled = enabled;
}

void RemoteCommandHandler::setEncrypted(bool encrypted) {
  this->encrypted = encrypted;
}

void RemoteCommandHandler::stopRetry() {
  if (retrying) {
    global->getTickPoke()->stopPoke(this, 0);
    retrying = false;
  }
}

void RemoteCommandHandler::tick(int) {
  stopRetry();
  if (enabled) {
    connect();
  }
}
