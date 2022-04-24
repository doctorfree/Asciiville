#pragma once

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "core/eventreceiver.h"
#include "refreshgovernor.h"

class TransferMonitor;
class ConnStateTracker;
class FTPConn;
class RawBuffer;
class SiteRace;
class FileList;
class Site;
class Race;
class SiteLogicRequest;
class SiteLogicRequestReady;
class PotentialTracker;
class FileStore;
class TransferJob;
class CommandOwner;
class Path;
class SiteTransferJob;
class FileListData;
class DelayedCommand;
class RequestCallback;
struct DownloadFileData;
enum class FTPConnState;
enum class FailureType;

enum class TransferType {
  REGULAR,
  PRE,
  COMPLETE,
  TRANSFERJOB
};

enum class Exists;

class SiteLogic : public Core::EventReceiver {
  private:
    std::shared_ptr<Site> site;
    std::vector<FTPConn *> conns;
    std::vector<ConnStateTracker> connstatetracker;
    std::vector<std::shared_ptr<SiteRace>> currentraces;
    std::list<std::shared_ptr<SiteRace>> recentlylistedraces;
    std::list<std::shared_ptr<SiteTransferJob> > transferjobs;
    RawBuffer * rawcommandrawbuf;
    RawBuffer * aggregatedrawbuf;
    int slotsdn;
    int slotsup;
    int available;
    PotentialTracker * ptrack;
    unsigned int loggedin;
    std::list<SiteLogicRequest> requests;
    std::list<SiteLogicRequestReady> requestsready;
    int requestidcounter;
    bool poke;
    int currtime;
    int timesincelastrequestready;
    std::list<DelayedCommand> delayedcommands;
    RefreshGovernor refreshgovernor;
    std::string lastlistpath;
    void handleConnection(int id);
    bool handleLockCheck(int id);
    bool handleSpreadJob(int id);
    bool handleSpreadJobs(int id, bool requestaction = true);
    bool handleTransferJobs(int id);
    bool handleRequest(int);
    void handleRecursiveLogic(int id, const std::shared_ptr<FileList>& fl = nullptr);
    void addRecentList(const std::shared_ptr<SiteRace> & sr);
    bool wasRecentlyListed(const std::shared_ptr<SiteRace> & sr) const;
    void refreshChangePath(int, const std::shared_ptr<SiteRace> & race, bool);
    void initTransfer(int);
    void handleFail(int);
    void handleTransferFail(int, int);
    void handleTransferFail(int, int, int);
    void reportTransferErrorAndFinish(int, int);
    void reportTransferErrorAndFinish(int, int, int);
    void getFileListConn(int id, bool hiddenfiles = false);
    void getFileListConn(int, const std::shared_ptr<CommandOwner>& co, const std::shared_ptr<FileList>& fl);
    void passiveModeCommand(int);
    static void * run(void *);
    bool lockTransferConn(const std::shared_ptr<FileList>& fl, int* ret, TransferMonitor* tm, const std::shared_ptr<CommandOwner>& co, bool isdownload);
    void setRequestReady(unsigned int, void *, bool, bool returnslot = true);
    void cleanupConnection(int);
    void checkFailListRequest(int);
    void clearExpiredReadyRequests();
    void clearReadyRequest(SiteLogicRequestReady &);
    bool setPathExists(int id, Exists exists, bool refreshtime);
    bool handlePreTransfer(int);
    bool handleCommandDelete(int, bool);
    bool makeTargetDirectory(int, bool, const std::shared_ptr<CommandOwner> & co);
    std::shared_ptr<SiteRace> getRace(unsigned int id) const;
    void antiAntiIdle(int id);
    void transferComplete(int, bool isdownload, bool returntransferslot = true);
    bool getSlot(bool isdownload, TransferType type);
    void handlePostUpload(int id, const std::shared_ptr<FileList>& fl);
    void handlePostDownload(int id);
    std::string expandVariables(const std::string& text) const;
  public:
    SiteLogic(const std::string &);
    ~SiteLogic();
    void runInstance();
    std::shared_ptr<SiteRace> addRace(const std::shared_ptr<Race>&, const std::string&, const std::string&, bool downloadonly);
    void resetRace(const std::shared_ptr<SiteRace> & race);
    void addTransferJob(std::shared_ptr<SiteTransferJob> & tj);
    void tick(int);
    void connectFailed(int);
    void userDenied(int);
    void userDeniedSiteFull(int);
    void userDeniedSimultaneousLogins(int);
    void loginKillFailed(int);
    void passDenied(int);
    void TLSFailed(int);
    void listRefreshed(int);
    void commandSuccess(int id, FTPConnState state);
    void commandFail(int id);
    void commandFail(int id, FailureType failuretype);
    void gotPath(int, const std::string &);
    void rawCommandResultRetrieved(int, const std::string &);
    void gotPassiveAddress(int, const std::string &, int);
    void timedout(int);
    void disconnected(int);
    void activateOne();
    void activateAll();
    void disconnectAll(bool hard = false);
    void haveConnectedActivate(unsigned int);
    const std::shared_ptr<Site> & getSite() const;
    std::shared_ptr<SiteRace> getRace(const std::string & race) const;
    void lockConnList(int);
    bool lockDownloadConn(const std::shared_ptr<FileList>& fl, int* ret, const std::shared_ptr<CommandOwner>& co, TransferMonitor* tm);
    bool lockUploadConn(const std::shared_ptr<FileList>& fl, int* ret, const std::shared_ptr<CommandOwner>& co, TransferMonitor* tm);
    void returnConn(int, bool);
    void registerDownloadLock(int id, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co, TransferMonitor* tm);
    void setNumConnections(unsigned int);
    bool downloadSlotAvailable(TransferType type = TransferType::REGULAR) const;
    bool uploadSlotAvailable() const;
    int slotsAvailable() const;
    int getCurrLogins() const;
    int getCurrDown() const;
    int getCurrUp() const;
    int getCleanlyClosedConnectionsCount() const;
    void connectConn(int);
    void disconnectConn(int id, bool hard = false);
    void finishTransferGracefully(int);
    void listCompleted(int id, int storeid, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co);
    void downloadCompleted(int id, int storeid, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co);
    void issueRawCommand(unsigned int, const std::string &);
    RawBuffer * getRawCommandBuffer() const;
    RawBuffer * getAggregatedRawBuffer() const;
    void raceGlobalComplete(const std::shared_ptr<SiteRace> & sr);
    void raceLocalComplete(const std::shared_ptr<SiteRace> & sr, int uploadslotsleft, bool reportdone = true);
    int requestFileList(RequestCallback* cb, const Path &);
    int requestDownloadFile(RequestCallback* cb, const Path& path, const std::string& file, bool inmemory);
    int requestDownloadFile(RequestCallback* cb, const std::shared_ptr<FileList>& fl, const std::string& file, bool inmemory);
    int requestRawCommand(RequestCallback* cb, const std::string &);
    int requestRawCommand(RequestCallback* cb, const Path &, const std::string &);
    int requestWipe(RequestCallback* cb, const Path &, bool);
    int requestDelete(RequestCallback* cb, const Path& path, bool recursive,  bool allfiles);
    int requestNuke(RequestCallback* cb, const Path &, int, const std::string &);
    int requestOneIdle(RequestCallback* cb);
    int requestAllIdle(RequestCallback* cb, const Path & path, int);
    int requestAllIdle(RequestCallback* cb, int);
    int requestMakeDirectory(RequestCallback* cb, const Path& path, const std::string & dirname);
    int requestMove(RequestCallback* cb, const Path& srcpath, const Path& dstpath);
    bool requestReady(int) const;
    bool requestStatus(int) const;
    void abortRace(const std::shared_ptr<SiteRace> & race);
    void removeRace(const std::shared_ptr<SiteRace> & race);
    void abortTransfers(const std::shared_ptr<CommandOwner> & co);
    FileListData* getFileListData(int requestid) const;
    DownloadFileData* getDownloadFileData(int requestid) const;
    void* getOngoingRequestData(int requestid) const;
    std::string getRawCommandResult(int requestid);
    bool finishRequest(int);
    void pushPotential(int, const std::string &, const std::shared_ptr<SiteLogic> &);
    bool potentialCheck(int);
    int getPotential();
    void siteUpdated();
    void updateName();
    const std::vector<FTPConn*>* getConns() const;
    FTPConn* getConn(int) const;
    std::string getStatus(int) const;
    void preparePassiveTransfer(int id, const std::string& file, bool fxp, bool ipv6, bool ssl, bool sslclient = false);
    void prepareActiveTransfer(int id, const std::string& file , bool fxp, bool ipv6, const std::string& host, int port, bool ssl, bool sslclient = false);
    void preparePassiveList(int id, TransferMonitor* tmb, bool ipv6, bool ssl);
    void prepareActiveList(int id, TransferMonitor* tmb, bool ipv6, const std::string & host, int port, bool ssl);
    void download(int);
    void upload(int);
    void list(int);
    void listAll(int);
    const ConnStateTracker* getConnStateTracker(int) const;
};
