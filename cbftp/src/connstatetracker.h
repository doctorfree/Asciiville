#pragma once

#include <memory>
#include <string>
#include <list>

#include "delayedcommand.h"

#define CST_DOWNLOAD 981
#define CST_UPLOAD 982
#define CST_LIST 983

class CommandOwner;
class SiteRace;
class FileList;
class TransferMonitor;
class DelayedCommand;
class RecursiveCommandLogic;
class SiteLogicRequest;

class ConnStateTracker {
private:
  unsigned long long int time;
  int idletime;
  int lastcheckedcount;
  std::shared_ptr<SiteRace> lastchecked;
  DelayedCommand delayedcommand;
  bool transfer;
  bool initialized;
  TransferMonitor* tm;
  std::shared_ptr<FileList> fl;
  std::string file;
  int type;
  bool passive;
  bool ssl;
  bool sslclient;
  bool ipv6;
  bool aborted;
  bool transferlocked;
  bool loggedin;
  bool fxp;
  bool listtransfer;
  bool listpassive;
  bool listssl;
  bool listipv6;
  bool listinitialized;
  bool quitting;
  bool handling;
  std::string listhost;
  int listport;
  TransferMonitor * listtm;
  std::shared_ptr<CommandOwner> co;
  std::string host;
  int port;
  std::shared_ptr<RecursiveCommandLogic> recursivelogic;
  std::shared_ptr<SiteLogicRequest> request;
  bool refreshtoken;
  std::string lastrefreshpath;
  void setTransfer(const std::string& file, bool fxp, bool ipv6, bool passive, const std::string& host, int port, bool ssl, bool sslclient);
  void setList(TransferMonitor* tm, bool ipv6, bool listpassive, const std::string& host, int port, bool ssl);
public:
  ConnStateTracker();
  ~ConnStateTracker();
  void delayedCommand(const std::string & command, int delay, bool persisting = false, const std::shared_ptr<CommandOwner> & co = nullptr);
  void timePassed(int);
  int getTimePassed() const;
  void check();
  void setLastChecked(const std::shared_ptr<SiteRace>& sr);
  void resetLastChecked();
  const std::shared_ptr<SiteRace> & lastChecked() const;
  int checkCount() const;
  void purgeSiteRace(const std::shared_ptr<SiteRace> & sr);
  DelayedCommand & getCommand();
  void setDisconnected();
  void setTransfer(const std::string& file, bool fxp, bool ipv6, bool ssl, bool sslclient);
  void setTransfer(const std::string& file, bool fxp, bool ipv6, const std::string& host, int port, bool ssl, bool sslclient);
  void setList(TransferMonitor* tm, bool ipv6, bool ssl);
  void setList(TransferMonitor* tm, bool ipv6, const std::string & host, int port, bool ssl);
  bool hasTransfer() const;
  bool hasFileTransfer() const;
  void finishTransfer();
  void finishFileTransfer();
  void abortTransfer();
  bool getTransferAborted() const;
  void lockForTransfer(TransferMonitor *, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner> &, bool);
  bool isListLocked() const;
  bool isTransferLocked() const;
  bool hasRequest() const;
  bool isLocked() const;
  bool isListOrTransferLocked() const;
  bool isHardLocked() const;
  const std::shared_ptr<SiteLogicRequest> & getRequest() const;
  void setRequest(SiteLogicRequest);
  void finishRequest();
  bool isLoggedIn() const;
  void setLoggedIn();
  void use();
  void resetIdleTime();
  TransferMonitor* getTransferMonitor() const;
  std::shared_ptr<FileList> getTransferFileList() const;
  std::string getTransferFile() const;
  int getTransferType() const;
  bool getTransferPassive() const;
  bool getTransferSSL() const;
  bool getTransferIPv6() const;
  bool getTransferSSLClient() const;
  bool getTransferFXP() const;
  std::string getTransferHost() const;
  int getTransferPort() const;
  const std::shared_ptr<RecursiveCommandLogic>& getRecursiveLogic() const;
  bool transferInitialized() const;
  const std::shared_ptr<CommandOwner>& getCommandOwner() const;
  void initializeTransfer();
  bool isQuitting() const;
  void setQuitting();
  bool hasRefreshToken() const;
  void setRefreshToken();
  void useRefreshTokenFor(const std::string& refreshpath);
  bool isHandlingConnection() const;
  void setHandlingConnection(bool handling);
  std::string getLastRefreshPath() const;
};
