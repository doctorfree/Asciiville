#pragma once

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "address.h"
#include "path.h"
#include "skiplist.h"
#include "statistics.h"
#include "hourlyalltracking.h"
#include "transferpairing.h"
#include "transferprotocol.h"
#include "util.h"

#define SITE_PROXY_GLOBAL 820
#define SITE_PROXY_NONE 821
#define SITE_PROXY_USE 822

#define SITE_SSL_ALWAYS_OFF 830
#define SITE_SSL_PREFER_OFF 831
#define SITE_SSL_PREFER_ON 832
#define SITE_SSL_ALWAYS_ON 833

#define SITE_LIST_STAT 840
#define SITE_LIST_LIST 841

enum class SitePriority {
  VERY_LOW = 711,
  LOW = 712,
  NORMAL = 713,
  HIGH = 714,
  VERY_HIGH = 715
};

enum class RefreshRate {
  VERY_LOW = 751,
  FIXED_LOW = 752,
  FIXED_AVERAGE = 753,
  FIXED_HIGH = 754,
  FIXED_VERY_HIGH = 755,
  AUTO = 756,
  DYNAMIC_LOW = 757,
  DYNAMIC_AVERAGE = 758,
  DYNAMIC_HIGH = 759,
  DYNAMIC_VERY_HIGH = 760
};

#define SITE_TRANSFER_POLICY_ALLOW 817
#define SITE_TRANSFER_POLICY_BLOCK 818

enum SiteAllowTransfer {
  SITE_ALLOW_TRANSFER_NO = 821,
  SITE_ALLOW_TRANSFER_YES = 822,
  SITE_ALLOW_DOWNLOAD_MATCH_ONLY = 823
};

enum class TLSMode {
  NONE = 0,
  AUTH_TLS = 1,
  IMPLICIT = 2
};

class Site {
private:
  std::string name;
  std::list<Address> addresses;
  std::string user;
  std::string pass;
  Path basepath;
  int logins;
  int maxup;
  int maxdn;
  int maxdnpre;
  int maxdncomplete;
  int maxdntransferjob;
  unsigned int maxidletime;
  bool pret;
  bool binary;
  int listcommand;
  TLSMode tlsmode;
  int ssltransfer;
  TransferProtocol transferprotocol;
  bool sscnsupported;
  bool cpsvsupported;
  bool ceprsupported;
  bool brokenpasv;
  bool disabled;
  SiteAllowTransfer allowupload;
  SiteAllowTransfer allowdownload;
  SitePriority priority;
  RefreshRate refreshrate;
  bool xdupe;
  std::map<std::string, Path> sections;
  std::map<std::string, int> avgspeed;
  std::map<std::string, std::pair<int, unsigned long long int>> avgspeedsamples;
  std::set<std::string, util::naturalComparator> affils;
  std::set<std::shared_ptr<Site>> exceptsourcesites;
  std::set<std::shared_ptr<Site>> excepttargetsites;
  int proxytype;
  std::string proxyname;
  int dataproxytype;
  std::string dataproxyname;
  int transfersourcepolicy;
  int transfertargetpolicy;
  SkipList skiplist;
  std::map<std::string, HourlyAllTracking> sitessizeup;
  std::map<std::string, HourlyAllTracking> sitesfilesup;
  std::map<std::string, HourlyAllTracking> sitessizedown;
  std::map<std::string, HourlyAllTracking> sitesfilesdown;
  HourlyAllTracking sizeup;
  HourlyAllTracking filesup;
  HourlyAllTracking sizedown;
  HourlyAllTracking filesdown;
  bool freeslot;
  bool stayloggedin;
  TransferPairing transferpairing;
  int maxtransfertimeseconds;
  int maxspreadjobtimeseconds;
public:
  Site();
  Site(const std::string &);
  Site(const Site &);
  std::map<std::string, Path, util::naturalComparator>::const_iterator sectionsBegin() const;
  std::map<std::string, Path, util::naturalComparator>::const_iterator sectionsEnd() const;
  std::map<std::string, int>::const_iterator avgspeedBegin() const;
  std::map<std::string, int>::const_iterator avgspeedEnd() const;
  unsigned int getMaxLogins() const;
  unsigned int getMaxUp() const;
  unsigned int getMaxDown() const;
  unsigned int getMaxDownPre() const;
  unsigned int getMaxDownComplete() const;
  unsigned int getMaxDownTransferJob() const;
  int getInternMaxLogins() const;
  int getInternMaxUp() const;
  int getInternMaxDown() const;
  int getInternMaxDownPre() const;
  int getInternMaxDownComplete() const;
  int getInternMaxDownTransferJob() const;
  bool getLeaveFreeSlot() const;
  const Path & getBasePath() const;
  bool unlimitedLogins() const;
  bool unlimitedUp() const;
  bool unlimitedDown() const;
  int getAverageSpeed(const std::string &) const;
  void setAverageSpeed(const std::string &, int);
  void pushTransferSpeed(const std::string &, int, unsigned long long int);
  std::pair<int, unsigned long long int> getAverageSpeedSamples(const std::string &) const;
  void resetAverageSpeedSamples(const std::string &);
  bool needsPRET() const;
  void setPRET(bool);
  bool forceBinaryMode() const;
  void setForceBinaryMode(bool);
  int getSSLTransferPolicy() const;
  int getListCommand() const;
  TLSMode getTLSMode() const;
  TransferProtocol getTransferProtocol() const;
  void setSSLTransferPolicy(int);
  void setTransferProtocol(TransferProtocol protocol);
  void setListCommand(int);
  SitePriority getPriority() const;
  RefreshRate getRefreshRate() const;
  static std::string getPriorityText(SitePriority priority);
  static std::string getRefreshRateText(RefreshRate rate);
  void setPriority(SitePriority priority);
  void setRefreshRate(RefreshRate rate);
  bool hasBrokenPASV() const;
  void setBrokenPASV(bool val);
  bool supportsSSCN() const;
  bool supportsCPSV() const;
  bool supportsCEPR() const;
  void setSupportsSSCN(bool supported);
  void setSupportsCPSV(bool supported);
  void setSupportsCEPR(bool supported);
  bool getDisabled() const;
  SiteAllowTransfer getAllowUpload() const;
  SiteAllowTransfer getAllowDownload() const;
  int getProxyType() const;
  std::string getProxy() const;
  int getDataProxyType() const;
  std::string getDataProxy() const;
  unsigned int getMaxIdleTime() const;
  std::string getName() const;
  unsigned int sectionsSize() const;
  const Path getSectionPath(const std::string &) const;
  bool hasSection(const std::string &) const;
  Address getAddress() const;
  std::list<Address> getAddresses() const;
  std::string getAddressesAsString() const;
  std::string getUser() const;
  std::string getPass() const;
  int getTransferSourcePolicy() const;
  int getTransferTargetPolicy() const;
  bool useXDUPE() const;
  SkipList& getSkipList();
  TransferPairing& getTransferPairing();
  const std::map<std::string, Path> & getSections() const;
  bool getStayLoggedIn() const;
  void setName(const std::string &);
  void setAddresses(std::string addrports);
  void setPrimaryAddress(const Address& addr);
  void setBasePath(const std::string &);
  void setUser(const std::string &);
  void setPass(const std::string &);
  void setMaxLogins(int num);
  void setMaxDn(int num);
  void setMaxUp(int num);
  void setMaxDnPre(int num);
  void setMaxDnComplete(int num);
  void setMaxDnTransferJob(int num);
  void setLeaveFreeSlot(bool free);
  void setTLSMode(TLSMode mode);
  void setDisabled(bool);
  void setAllowUpload(SiteAllowTransfer);
  void setAllowDownload(SiteAllowTransfer);
  void setMaxIdleTime(unsigned int);
  void setProxyType(int proxytype);
  void setProxy(const std::string& proxyname);
  void setDataProxyType(int proxytype);
  void setDataProxy(const std::string& proxyname);
  void clearSections();
  bool isAffiliated(const std::string& affil) const;
  void addAffil(const std::string& affil);
  void clearAffils();
  void setTransferSourcePolicy(int);
  void setTransferTargetPolicy(int);
  void setUseXDUPE(bool);
  void setStayLoggedIn(bool loggedin);
  void addAllowedSourceSite(const std::shared_ptr<Site>& site);
  void addBlockedSourceSite(const std::shared_ptr<Site>& site);
  void addExceptSourceSite(const std::shared_ptr<Site>& site);
  void addAllowedTargetSite(const std::shared_ptr<Site>& site);
  void addBlockedTargetSite(const std::shared_ptr<Site>& site);
  void addExceptTargetSite(const std::shared_ptr<Site>& site);
  void purgeOtherSite(const std::shared_ptr<Site>& site);
  void clearExceptSites();
  bool isAllowedTargetSite(const std::shared_ptr<Site> &) const;
  std::set<std::string, util::naturalComparator>::const_iterator affilsBegin() const;
  std::set<std::string, util::naturalComparator>::const_iterator affilsEnd() const;
  std::set<std::shared_ptr<Site> >::const_iterator exceptSourceSitesBegin() const;
  std::set<std::shared_ptr<Site> >::const_iterator exceptSourceSitesEnd() const;
  std::set<std::shared_ptr<Site> >::const_iterator exceptTargetSitesBegin() const;
  std::set<std::shared_ptr<Site> >::const_iterator exceptTargetSitesEnd() const;
  void addSection(const std::string &, const std::string &);
  void renameSection(const std::string & oldname, const std::string & newname);
  void removeSection(const std::string & name);
  std::list<std::string> getSectionsForPath(const Path &) const;
  std::list<std::string> getSectionsForPartialPath(const Path &) const;
  std::pair<Path, Path> splitPathInSectionAndSubpath(const Path &) const;
  void addTransferStatsFile(StatsDirection, const std::string &, unsigned long long int);
  void addTransferStatsFile(StatsDirection, unsigned long long int);
  void tickMinute();
  const HourlyAllTracking & getSizeUp() const;
  const HourlyAllTracking & getSizeDown() const;
  const HourlyAllTracking & getFilesUp() const;
  const HourlyAllTracking & getFilesDown() const;
  HourlyAllTracking & getSizeUp();
  HourlyAllTracking & getSizeDown();
  HourlyAllTracking & getFilesUp();
  HourlyAllTracking & getFilesDown();
  std::map<std::string, HourlyAllTracking>::const_iterator sizeUpBegin() const;
  std::map<std::string, HourlyAllTracking>::const_iterator filesUpBegin() const;
  std::map<std::string, HourlyAllTracking>::const_iterator sizeDownBegin() const;
  std::map<std::string, HourlyAllTracking>::const_iterator filesDownBegin() const;
  std::map<std::string, HourlyAllTracking>::const_iterator sizeUpEnd() const;
  std::map<std::string, HourlyAllTracking>::const_iterator filesUpEnd() const;
  std::map<std::string, HourlyAllTracking>::const_iterator sizeDownEnd() const;
  std::map<std::string, HourlyAllTracking>::const_iterator filesDownEnd() const;
  HourlyAllTracking & getSiteSizeUp(const std::string & site);
  HourlyAllTracking & getSiteSizeDown(const std::string & site);
  HourlyAllTracking & getSiteFilesUp(const std::string & site);
  HourlyAllTracking & getSiteFilesDown(const std::string & site);
  void setSkipList(const SkipList & skiplist);
  void setSections(const std::map<std::string, Path> & sections);
  void setTransferPairing(const TransferPairing& transferpairing);
  void resetHourlyStats();
  void resetAllStats();
  int getMaxTransferTimeSeconds() const;
  void setMaxTransferTimeSeconds(int seconds);
  int getMaxSpreadJobTimeSeconds() const;
  void setMaxSpreadJobTimeSeconds(int seconds);
};
