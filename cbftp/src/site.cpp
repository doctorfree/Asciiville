#include "site.h"

#include "path.h"
#include "util.h"
#include "globalcontext.h"
#include "sectionmanager.h"

namespace {

#define REPORT_LOGINS_IF_UNLIMITED 10

Address parseAddress(std::string address) {
  Address addr;
  addr.addrfam = Core::AddressFamily::IPV4_IPV6;
  addr.port = 21;
  addr.brackets = false;
  if (address.length() > 3) {
    std::string prefix = address.substr(0, 3);
    if (prefix == "(4)") {
      addr.addrfam = Core::AddressFamily::IPV4;
      address = address.substr(3);
    }
    else if (prefix == "(6)") {
      addr.addrfam = Core::AddressFamily::IPV6;
      address = address.substr(3);
    }
  }
  size_t portpos = std::string::npos;
  if (!address.empty() && address[0] == '[') {
    size_t bracketendpos = address.find("]");
    if (bracketendpos != std::string::npos) {
      addr.brackets = true;
      addr.host = address.substr(1, bracketendpos - 1);
      portpos = address.find(":", bracketendpos);
    }
  }
  if (!addr.brackets) {
    portpos = address.find(":");
    if (portpos != std::string::npos) {
      addr.host = address.substr(0, portpos);
    }
    else {
      addr.host = address;
    }
  }
  if (portpos != std::string::npos) {
    try {
      addr.port = std::stoi(address.substr(portpos + 1));
    }
    catch (std::exception&) {
    }
  }
  if (addr.port == 21) {
    addr.brackets = false;
  }
  return addr;
}

}

Site::Site() : skiplist(global->getSkipList()) {

}

Site::Site(const std::string& name) :
  name(name),
  user("anonymous"),
  pass("anonymous"),
  basepath("/"),
  logins(3),
  maxup(-1),
  maxdn(2),
  maxdnpre(-2),
  maxdncomplete(-2),
  maxdntransferjob(-2),
  maxidletime(60),
  pret(false),
  binary(false),
  listcommand(SITE_LIST_STAT),
  tlsmode(TLSMode::AUTH_TLS),
  ssltransfer(SITE_SSL_PREFER_OFF),
  transferprotocol(TransferProtocol::IPV4_ONLY),
  sscnsupported(true),
  cpsvsupported(true),
  ceprsupported(true),
  brokenpasv(false),
  disabled(false),
  allowupload(SITE_ALLOW_TRANSFER_YES),
  allowdownload(SITE_ALLOW_TRANSFER_YES),
  priority(SitePriority::NORMAL),
  refreshrate(RefreshRate::AUTO),
  xdupe(true),
  proxytype(SITE_PROXY_GLOBAL),
  dataproxytype(SITE_PROXY_GLOBAL),
  transfersourcepolicy(SITE_TRANSFER_POLICY_ALLOW),
  transfertargetpolicy(SITE_TRANSFER_POLICY_ALLOW),
  skiplist(global->getSkipList()),
  freeslot(false),
  stayloggedin(false),
  maxtransfertimeseconds(-1),
  maxspreadjobtimeseconds(-1)
{
  Address addr;
  addr.host = "ftp.sunet.se";
  addresses.push_back(addr);
}

Site::Site(const Site& other) {
  name = other.name;
  addresses = other.addresses;
  user = other.user;
  pass = other.pass;
  basepath = other.basepath;
  logins = other.logins;
  maxup = other.maxup;
  maxdn = other.maxdn;
  maxdnpre = other.maxdnpre;
  maxdncomplete = other.maxdncomplete;
  maxdntransferjob = other.maxdntransferjob;
  freeslot = other.freeslot;
  stayloggedin = other.stayloggedin;
  maxidletime = other.maxidletime;
  pret = other.pret;
  binary = other.binary;
  listcommand = other.listcommand;
  tlsmode = other.tlsmode;
  ssltransfer = other.ssltransfer;
  transferprotocol = other.transferprotocol;
  sscnsupported = other.sscnsupported;
  cpsvsupported = other.cpsvsupported;
  ceprsupported = other.ceprsupported;
  brokenpasv = other.brokenpasv;
  disabled = other.disabled;
  allowupload = other.allowupload;
  allowdownload = other.allowdownload;
  priority = other.priority;
  refreshrate = other.refreshrate;
  xdupe = other.xdupe;
  proxytype = other.proxytype;
  proxyname = other.proxyname;
  dataproxytype = other.dataproxytype;
  dataproxyname = other.dataproxyname;
  sections = other.sections;
  affils = other.affils;
  transfersourcepolicy = other.transfersourcepolicy;
  transfertargetpolicy = other.transfertargetpolicy;
  skiplist = other.skiplist;
  transferpairing = other.transferpairing;
  maxtransfertimeseconds = other.maxtransfertimeseconds;
  maxspreadjobtimeseconds = other.maxspreadjobtimeseconds;
}

std::map<std::string, Path>::const_iterator Site::sectionsBegin() const {
  return sections.begin();
}

std::map<std::string, Path>::const_iterator Site::sectionsEnd() const {
  return sections.end();
}

std::map<std::string, int>::const_iterator Site::avgspeedBegin() const {
  return avgspeed.begin();
}

std::map<std::string, int>::const_iterator Site::avgspeedEnd() const {
  return avgspeed.end();
}

unsigned int Site::getMaxLogins() const {
  if (logins == -1) {
    return REPORT_LOGINS_IF_UNLIMITED;
  }
  return logins;
}

unsigned int Site::getMaxUp() const {
  if (maxup == -1) {
    return getMaxLogins();
  }
  return maxup;
}

unsigned int Site::getMaxDown() const {
  if (maxdn == -1) {
    return getMaxLogins();
  }
  return maxdn;
}

unsigned int Site::getMaxDownPre() const {
  if (maxdnpre == -1) {
    return getMaxLogins();
  }
  if (maxdnpre == -2) {
    return getMaxDown();
  }
  return maxdnpre;
}

unsigned int Site::getMaxDownComplete() const {
  if (maxdncomplete == -1) {
    return getMaxLogins();
  }
  if (maxdncomplete == -2) {
    return getMaxDown();
  }
  return maxdncomplete;
}

unsigned int Site::getMaxDownTransferJob() const {
  if (maxdntransferjob == -1) {
    return getMaxLogins();
  }
  if (maxdntransferjob == -2) {
    return getMaxDown();
  }
  return maxdntransferjob;
}

unsigned int Site::getMaxIdleTime() const {
  return maxidletime;
}

int Site::getInternMaxLogins() const {
  return logins;
}

int Site::getInternMaxUp() const {
  return maxup;
}

int Site::getInternMaxDown() const {
  return maxdn;
}

int Site::getInternMaxDownPre() const {
  return maxdnpre;
}

int Site::getInternMaxDownComplete() const {
  return maxdncomplete;
}

int Site::getInternMaxDownTransferJob() const {
  return maxdntransferjob;
}

bool Site::getLeaveFreeSlot() const {
  return freeslot;
}

const Path & Site::getBasePath() const {
  return basepath;
}

bool Site::unlimitedLogins() const {
  return logins == -1;
}

bool Site::unlimitedUp() const {
  return maxup == -1;
}

bool Site::unlimitedDown() const {
  return maxdn == -1;
}

int Site::getAverageSpeed(const std::string& target) const {
  std::map<std::string, int>::const_iterator it = avgspeed.find(target);
  if (it == avgspeed.end()) return 1024;
  return it->second;
}

void Site::setAverageSpeed(const std::string& target, int speed) {
  std::map<std::string, int>::iterator it = avgspeed.find(target);
  if (it != avgspeed.end()) avgspeed.erase(it);
  avgspeed[target] = speed;
}

void Site::pushTransferSpeed(const std::string& target, int speed, unsigned long long int size) {
  std::map<std::string, int>::iterator it = avgspeed.find(target);
  int oldspeed;
  if (it == avgspeed.end()) {
    oldspeed = 1024;
    avgspeedsamples[target] = std::pair<int, unsigned long long int>(0, 0);
  }
  else {
    oldspeed = it->second;
    avgspeed.erase(it);
  }
  avgspeed[target] = (int) ((speed / 5) + (oldspeed * 0.8));
  ++avgspeedsamples[target].first;
  avgspeedsamples[target].second += size;
}

std::pair<int, unsigned long long int> Site::getAverageSpeedSamples(const std::string& target) const {
  std::map<std::string, std::pair<int, unsigned long long int> >::const_iterator it = avgspeedsamples.find(target);
  if (it != avgspeedsamples.end()) {
    return it->second;
  }
  return std::pair<int, unsigned long long int>(0, 0);
}

void Site::resetAverageSpeedSamples(const std::string& target) {
  std::map<std::string, std::pair<int, unsigned long long int> >::iterator it = avgspeedsamples.find(target);
  if (it != avgspeedsamples.end()) {
    it->second.first = 0;
    it->second.second = 0;
  }
}

bool Site::needsPRET() const {
  return pret;
}

void Site::setPRET(bool val) {
  pret = val;
}

bool Site::forceBinaryMode() const {
  return binary;
}

void Site::setForceBinaryMode(bool val) {
  binary = val;
}

TLSMode Site::getTLSMode() const {
  return tlsmode;
}

TransferProtocol Site::getTransferProtocol() const {
  return transferprotocol;
}

int Site::getSSLTransferPolicy() const {
  return ssltransfer;
}

void Site::setSSLTransferPolicy(int policy) {
  ssltransfer = policy;
}

SitePriority Site::getPriority() const {
  return priority;
}

RefreshRate Site::getRefreshRate() const {
  return refreshrate;
}

std::string Site::getPriorityText(SitePriority priority) {
  switch (priority) {
    case SitePriority::VERY_LOW:
      return "Very low";
    case SitePriority::LOW:
      return "Low";
    case SitePriority::NORMAL:
      return "Normal";
    case SitePriority::HIGH:
      return "High";
    case SitePriority::VERY_HIGH:
      return "Very high";
  }
  return "Unknown";
}

std::string Site::getRefreshRateText(RefreshRate rate) {
  switch (rate) {
    case RefreshRate::VERY_LOW:
      return "Very low";
    case RefreshRate::FIXED_LOW:
      return "Fixed low";
    case RefreshRate::FIXED_AVERAGE:
      return "Fixed average";
    case RefreshRate::FIXED_HIGH:
      return "Fixed high";
    case RefreshRate::FIXED_VERY_HIGH:
      return "Fixed very high";
    case RefreshRate::AUTO:
      return "Auto";
    case RefreshRate::DYNAMIC_LOW:
      return "Dynamic low";
    case RefreshRate::DYNAMIC_AVERAGE:
      return "Dynamic average";
    case RefreshRate::DYNAMIC_HIGH:
      return "Dynamic high";
    case RefreshRate::DYNAMIC_VERY_HIGH:
      return "Dynamic very high";
  }
  return "Unknown";
}

void Site::setPriority(SitePriority priority) {
  this->priority = priority;
}

void Site::setRefreshRate(RefreshRate rate) {
  refreshrate = rate;
}

bool Site::supportsSSCN() const {
  return sscnsupported;
}

bool Site::supportsCPSV() const {
  return cpsvsupported;
}

bool Site::supportsCEPR() const {
  return ceprsupported;
}

void Site::setSupportsSSCN(bool supported) {
  sscnsupported = supported;
}

void Site::setSupportsCPSV(bool supported) {
  cpsvsupported = supported;
}

void Site::setSupportsCEPR(bool supported) {
  ceprsupported = supported;
}

int Site::getListCommand() const {
  return listcommand;
}

void Site::setListCommand(int command) {
  listcommand = command;
}

void Site::setTLSMode(TLSMode mode) {
  tlsmode = mode;
}

void Site::setTransferProtocol(TransferProtocol protocol) {
  transferprotocol = protocol;
}

bool Site::getDisabled() const {
  return disabled;
}

SiteAllowTransfer Site::getAllowUpload() const {
  return allowupload;
}

SiteAllowTransfer Site::getAllowDownload() const {
  return allowdownload;
}

int Site::getProxyType() const {
  return proxytype;
}

std::string Site::getProxy() const {
  return proxyname;
}

int Site::getDataProxyType() const {
  return dataproxytype;
}

std::string Site::getDataProxy() const {
  return dataproxyname;
}

void Site::setDisabled(bool val) {
  disabled = val;
}

void Site::setAllowUpload(SiteAllowTransfer val) {
  allowupload = val;
}

void Site::setAllowDownload(SiteAllowTransfer val) {
  allowdownload = val;
}

bool Site::hasBrokenPASV() const {
  return brokenpasv;
}

void Site::setBrokenPASV(bool val) {
  brokenpasv = val;
}

std::string Site::getName() const {
  return name;
}

unsigned int Site::sectionsSize() const {
  return sections.size();
}

const Path Site::getSectionPath(const std::string& sectionname) const {
  std::map<std::string, Path>::const_iterator it = sections.find(sectionname);
  if (it == sections.end()) return "/";
  return it->second;
}

bool Site::hasSection(const std::string& sectionname) const {
  std::map<std::string, Path>::const_iterator it = sections.find(sectionname);
  if (it == sections.end()) return false;
  return true;
}

Address Site::getAddress() const {
  if (addresses.empty()) {
    return Address();
  }
  return addresses.front();
}

std::list<Address> Site::getAddresses() const {
  return addresses;
}

std::string Site::getAddressesAsString() const {
  std::string addressesconcat;
  for (const Address& addr : addresses) {
    if (addressesconcat.length()) {
      addressesconcat += " ";
    }
    addressesconcat += addr.toString();
  }
  return addressesconcat;
}

std::string Site::getUser() const {
  return user;
}

std::string Site::getPass() const {
  return pass;
}

int Site::getTransferSourcePolicy() const {
  return transfersourcepolicy;
}

int Site::getTransferTargetPolicy() const {
  return transfertargetpolicy;
}

bool Site::useXDUPE() const {
  return xdupe;
}

SkipList & Site::getSkipList() {
  return skiplist;
}

TransferPairing& Site::getTransferPairing() {
  return transferpairing;
}

const std::map<std::string, Path>& Site::getSections() const {
  return sections;
}

bool Site::getStayLoggedIn() const {
  return stayloggedin;
}

void Site::setName(const std::string& name) {
  this->name = name;
}

void Site::setAddresses(std::string addrports) {
  addresses.clear();
  size_t pos;
  while ((pos = addrports.find(";")) != std::string::npos) addrports[pos] = ' ';
  while ((pos = addrports.find(",")) != std::string::npos) addrports[pos] = ' ';
  std::list<std::string> addrlist = util::trim(util::split(addrports));
  for (const std::string& address : addrlist) {
    addresses.push_back(parseAddress(address));
  }
}

void Site::setPrimaryAddress(const Address& addr) {
  std::list<Address>::iterator it;
  for (it = addresses.begin(); it != addresses.end(); it++) {
    if (*it == addr) {
      addresses.erase(it);
      break;
    }
  }
  addresses.push_front(addr);
}

void Site::setUser(const std::string & user) {
  this->user = user;
}

void Site::setPass(const std::string & pass) {
  this->pass = pass;
}

void Site::setBasePath(const std::string & basepath) {
  this->basepath = basepath.empty() ? "/" : basepath;
}

void Site::setMaxLogins(int num) {
  logins = num;
  if (num > 0 && maxdn > num) {
    maxdn = num;
  }
  if (num > 0 && maxdnpre > num) {
    maxdnpre = num;
  }
  if (num > 0 && maxdncomplete > num) {
    maxdncomplete = num;
  }
  if (num > 0 && maxdntransferjob > num) {
    maxdntransferjob = num;
  }
  if (num > 0 && maxup > num) {
    maxup = num;
  }
}

void Site::setMaxDn(int num) {
  maxdn = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxUp(int num) {
  maxup = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxDnPre(int num) {
  maxdnpre = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxDnComplete(int num) {
  maxdncomplete = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxDnTransferJob(int num) {
  maxdntransferjob = num > logins && logins > 0 ? logins : num;
}

void Site::setLeaveFreeSlot(bool free) {
  freeslot = free;
}

void Site::setMaxIdleTime(unsigned int idletime) {
  maxidletime = idletime;
}

void Site::setProxyType(int proxytype) {
  this->proxytype = proxytype;
}

void Site::setProxy(const std::string & proxyname) {
  this->proxyname = proxyname;
}

void Site::setDataProxyType(int proxytype) {
  this->dataproxytype = proxytype;
}

void Site::setDataProxy(const std::string& proxyname) {
  this->dataproxyname = proxyname;
}

void Site::clearSections() {
  sections.clear();
}

void Site::addSection(const std::string& name, const std::string& path) {
  sections[name] = path;
  global->getSectionManager()->addSection(name);
}

void Site::renameSection(const std::string& oldname, const std::string& newname) {
  sections[newname] = sections[oldname];
  sections.erase(oldname);
}

void Site::removeSection(const std::string& name) {
  sections.erase(name);
}

bool Site::isAffiliated(const std::string& affil) const {
  return (affils.find(affil) != affils.end());
}

void Site::addAffil(const std::string& affil) {
  affils.insert(affil);
}

void Site::clearAffils() {
  affils.clear();
}

void Site::setTransferSourcePolicy(int policy) {
  transfersourcepolicy = policy;
}

void Site::setTransferTargetPolicy(int policy) {
  transfertargetpolicy = policy;
}

void Site::setUseXDUPE(bool xdupe) {
  this->xdupe = xdupe;
}

void Site::setStayLoggedIn(bool loggedin) {
  stayloggedin = loggedin;
}

void Site::addAllowedSourceSite(const std::shared_ptr<Site>& site) {
  if (transfersourcepolicy == SITE_TRANSFER_POLICY_BLOCK) {
    exceptsourcesites.insert(site);
  }
  else {
    exceptsourcesites.erase(site);
  }
}

void Site::addBlockedSourceSite(const std::shared_ptr<Site>& site) {
  if (transfersourcepolicy == SITE_TRANSFER_POLICY_ALLOW) {
    exceptsourcesites.insert(site);
  }
  else {
    exceptsourcesites.erase(site);
  }
}

void Site::addAllowedTargetSite(const std::shared_ptr<Site>& site) {
  if (transfertargetpolicy == SITE_TRANSFER_POLICY_BLOCK) {
    excepttargetsites.insert(site);
  }
  else {
    excepttargetsites.erase(site);
  }
}

void Site::addBlockedTargetSite(const std::shared_ptr<Site>& site) {
  if (transfertargetpolicy == SITE_TRANSFER_POLICY_ALLOW) {
    excepttargetsites.insert(site);
  }
  else {
    excepttargetsites.erase(site);
  }
}

void Site::addExceptSourceSite(const std::shared_ptr<Site>& site) {
  exceptsourcesites.insert(site);
}

void Site::addExceptTargetSite(const std::shared_ptr<Site>& site) {
  excepttargetsites.insert(site);
}

void Site::purgeOtherSite(const std::shared_ptr<Site>& site) {
  std::string name = site->getName();
  exceptsourcesites.erase(site);
  excepttargetsites.erase(site);
  avgspeed.erase(name);
  avgspeedsamples.erase(name);
  sitessizedown.erase(name);
  sitessizeup.erase(name);
  sitesfilesup.erase(name);
  sitesfilesdown.erase(name);
}

void Site::clearExceptSites() {
  exceptsourcesites.clear();
  excepttargetsites.clear();
}

bool Site::isAllowedTargetSite(const std::shared_ptr<Site>& site) const {
  std::set<std::shared_ptr<Site> >::const_iterator it = excepttargetsites.find(site);
  if (it != excepttargetsites.end()) {
    return transfertargetpolicy == SITE_TRANSFER_POLICY_BLOCK;
  }
  return transfertargetpolicy == SITE_TRANSFER_POLICY_ALLOW;
}

std::set<std::string, util::naturalComparator>::const_iterator Site::affilsBegin() const {
  return affils.begin();
}

std::set<std::string, util::naturalComparator>::const_iterator Site::affilsEnd() const {
  return affils.end();
}

std::set<std::shared_ptr<Site> >::const_iterator Site::exceptSourceSitesBegin() const {
  return exceptsourcesites.begin();
}

std::set<std::shared_ptr<Site> >::const_iterator Site::exceptSourceSitesEnd() const {
  return exceptsourcesites.end();
}

std::set<std::shared_ptr<Site> >::const_iterator Site::exceptTargetSitesBegin() const {
  return excepttargetsites.begin();
}

std::set<std::shared_ptr<Site> >::const_iterator Site::exceptTargetSitesEnd() const {
  return excepttargetsites.end();
}

std::list<std::string> Site::getSectionsForPath(const Path& path) const {
  std::map<std::string, Path>::const_iterator it;
  std::list<std::string> retsections;
  for (it = sections.begin(); it!= sections.end(); it++) {
    if (it->second == path) {
      retsections.push_back(it->first);
    }
  }
  return retsections;
}

std::list<std::string> Site::getSectionsForPartialPath(const Path& path) const {
  std::map<std::string, Path>::const_iterator it;
  std::list<std::string> retsections;
  for (it = sections.begin(); it!= sections.end(); it++) {
    if (path.contains(it->second)) {
      retsections.push_back(it->first);
    }
  }
  return retsections;
}

std::pair<Path, Path> Site::splitPathInSectionAndSubpath(const Path& path) const {
  Path sectionpath("/");
  std::list<std::string> sections = getSectionsForPartialPath(path);
  for (std::list<std::string>::const_iterator it = sections.begin(); it != sections.end(); ++it) {
    Path currentsectionpath = getSectionPath(*it);
    if (currentsectionpath.toString().length() > sectionpath.toString().length()) {
      sectionpath = currentsectionpath;
    }
  }
  Path subpath = path - sectionpath;
  return std::pair<Path, Path>(sectionpath, subpath);
}

void Site::addTransferStatsFile(StatsDirection direction, const std::string& other, unsigned long long int size) {
  if (direction == STATS_DOWN) {
    if (sitessizedown.find(other) == sitessizedown.end()) {
      sitessizedown[other] = HourlyAllTracking();
      sitesfilesdown[other] = HourlyAllTracking();
    }
    sitessizedown[other].add(size);
    sitesfilesdown[other].add(1);
  }
  else {
    if (sitessizeup.find(other) == sitessizeup.end()) {
      sitessizeup[other] = HourlyAllTracking();
      sitesfilesup[other] = HourlyAllTracking();
    }
    sitessizeup[other].add(size);
    sitesfilesup[other].add(1);
  }
  addTransferStatsFile(direction, size);
}

void Site::addTransferStatsFile(StatsDirection direction, unsigned long long int size) {
  if (direction == STATS_DOWN) {
    sizedown.add(size);
    filesdown.add(1);
  }
  else {
    sizeup.add(size);
    filesup.add(1);
  }
}

void Site::tickMinute() {
  sizeup.tickMinute();
  filesup.tickMinute();
  sizedown.tickMinute();
  filesdown.tickMinute();
  for (std::map<std::string, HourlyAllTracking>::iterator it = sitessizeup.begin(); it != sitessizeup.end(); it++) {
    it->second.tickMinute();
  }
  for (std::map<std::string, HourlyAllTracking>::iterator it = sitesfilesup.begin(); it != sitesfilesup.end(); it++) {
    it->second.tickMinute();
  }
  for (std::map<std::string, HourlyAllTracking>::iterator it = sitessizedown.begin(); it != sitessizedown.end(); it++) {
    it->second.tickMinute();
  }
  for (std::map<std::string, HourlyAllTracking>::iterator it = sitesfilesdown.begin(); it != sitesfilesdown.end(); it++) {
    it->second.tickMinute();
  }
}

const HourlyAllTracking& Site::getSizeUp() const {
  return sizeup;
}

const HourlyAllTracking& Site::getSizeDown() const {
  return sizedown;
}

const HourlyAllTracking& Site::getFilesUp() const {
  return filesup;
}

const HourlyAllTracking& Site::getFilesDown() const {
  return filesdown;
}

HourlyAllTracking& Site::getSizeUp() {
  return sizeup;
}

HourlyAllTracking& Site::getSizeDown() {
  return sizedown;
}

HourlyAllTracking& Site::getFilesUp() {
  return filesup;
}

HourlyAllTracking& Site::getFilesDown() {
  return filesdown;
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::sizeUpBegin() const {
  return sitessizeup.begin();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::filesUpBegin() const {
  return sitesfilesup.begin();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::sizeDownBegin() const {
  return sitessizedown.begin();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::filesDownBegin() const {
  return sitesfilesdown.begin();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::sizeUpEnd() const {
  return sitessizeup.end();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::filesUpEnd() const {
  return sitesfilesup.end();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::sizeDownEnd() const {
  return sitessizedown.end();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::filesDownEnd() const {
  return sitesfilesdown.end();
}

HourlyAllTracking& Site::getSiteSizeUp(const std::string& site) {
  return sitessizeup[site];
}

HourlyAllTracking& Site::getSiteSizeDown(const std::string& site) {
  return sitessizedown[site];
}

HourlyAllTracking& Site::getSiteFilesUp(const std::string& site) {
  return sitesfilesup[site];
}

HourlyAllTracking& Site::getSiteFilesDown(const std::string& site) {
  return sitesfilesdown[site];
}

void Site::setSkipList(const SkipList& skiplist) {
  this->skiplist = skiplist;
}

void Site::setSections(const std::map<std::string, Path>& sections) {
  this->sections = sections;
}

void Site::setTransferPairing(const TransferPairing& transferpairing) {
  this->transferpairing = transferpairing;
}

void Site::resetHourlyStats() {
  filesdown.resetHours();
  filesup.resetHours();
  sizedown.resetHours();
  sizeup.resetHours();
  for (std::pair<const std::string, HourlyAllTracking>& site : sitessizeup) {
    site.second.resetHours();
  }
  for (std::pair<const std::string, HourlyAllTracking>& site : sitessizedown) {
    site.second.resetHours();
  }
  for (std::pair<const std::string, HourlyAllTracking>& site : sitesfilesup) {
    site.second.resetHours();
  }
  for (std::pair<const std::string, HourlyAllTracking>& site : sitesfilesdown) {
    site.second.resetHours();
  }
}

void Site::resetAllStats() {
  filesdown.reset();
  filesup.reset();
  sizedown.reset();
  sizeup.reset();
  for (std::pair<const std::string, HourlyAllTracking>& site : sitessizeup) {
    site.second.reset();
  }
  for (std::pair<const std::string, HourlyAllTracking>& site : sitessizedown) {
    site.second.reset();
  }
  for (std::pair<const std::string, HourlyAllTracking>& site : sitesfilesup) {
    site.second.reset();
  }
  for (std::pair<const std::string, HourlyAllTracking>& site : sitesfilesdown) {
    site.second.reset();
  }
}

int Site::getMaxTransferTimeSeconds() const {
  return maxtransfertimeseconds;
}

void Site::setMaxTransferTimeSeconds(int seconds) {
  maxtransfertimeseconds = seconds;
}

int Site::getMaxSpreadJobTimeSeconds() const {
  return maxspreadjobtimeseconds;
}

void Site::setMaxSpreadJobTimeSeconds(int seconds) {
  maxspreadjobtimeseconds = seconds;
}

