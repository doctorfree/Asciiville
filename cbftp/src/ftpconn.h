#pragma once

#include <list>
#include <memory>
#include <string>

#include "core/eventreceiver.h"
#include "core/types.h"
#include "address.h"
#include "ftpconnectowner.h"
#include "path.h"

enum class FTPConnState {
  DISCONNECTED,
  CONNECTING,
  AUTH_TLS,
  USER,
  PASS,
  LOGIN,
  STAT,
  PWD,
  PROT_P,
  PROT_C,
  RAW,
  CPSV,
  PASV,
  PORT,
  EPSV,
  EPRT,
  CEPR_ON,
  CWD,
  MKD,
  PRET_RETR,
  PRET_STOR,
  RETR,
  RETR_COMPLETE,
  STOR,
  STOR_COMPLETE,
  ABOR,
  QUIT,
  USER_LOGINKILL,
  PASS_LOGINKILL,
  WIPE,
  DELE,
  RMD,
  NUKE,
  LIST,
  PRET_LIST,
  LIST_COMPLETE,
  SSCN_ON,
  SSCN_OFF,
  SSL_HANDSHAKE,
  PASV_ABORT,
  PBSZ,
  TYPEI,
  XDUPE,
  RNFR,
  RNTO,
  IDLE
};

enum class ProtMode {
  UNSET,
  C,
  P
};

enum class FailureType {
  UNDEFINED,
  DUPE
};

namespace Core {
class IOManager;
}

class FTPConnect;
class SiteRace;
class FileList;
class SiteLogic;
class RawBuffer;
class Site;
class ProxySession;
class CommandOwner;
class Proxy;
class RawBufferCallback;

#define DATA_BUF_SIZE 2048

class FTPConn : private Core::EventReceiver, public FTPConnectOwner {
  private:
    std::list<std::shared_ptr<FTPConnect> > connectors;
    int nextconnectorid;
    Core::IOManager * iom;
    ProxySession * proxysession;
    unsigned int databuflen;
    char * databuf;
    unsigned int databufpos;
    int databufcode;
    int id;
    bool processing;
    bool allconnectattempted;
    SiteLogic * sl;
    std::string status;
    std::shared_ptr<Site> site;
    int transferstatus;
    int sockid;
    FTPConnState state;
    bool aborted;
    std::shared_ptr<FileList> currentfl;
    std::shared_ptr<CommandOwner> currentco;
    Path currentpath;
    ProtMode protectedmode;
    bool sscnmode;
    bool ceprenabled;
    Path targetpath;
    bool mkdtarget;
    Path mkdsect;
    Path mkdpath;
    std::list<std::string> mkdsubdirs;
    RawBuffer * rawbuf;
    RawBuffer * aggregatedrawbuf;
    RawBuffer * cwdrawbuf;
    int ticker;
    std::list<std::string> xdupelist;
    bool xduperun;
    bool typeirun;
    bool cleanlyclosed;
    Address connectedaddr;
    void AUTHTLSResponse();
    void USERResponse();
    void PASSResponse();
    void STATResponse();
    void PWDResponse();
    void PROTPResponse();
    void PROTCResponse();
    void RawResponse();
    void CPSVResponse();
    void PASVResponse();
    void PORTResponse();
    void EPSVResponse();
    void EPRTResponse();
    void CEPRONResponse();
    void CWDResponse();
    void MKDResponse();
    void PRETRETRResponse();
    void PRETSTORResponse();
    void PRETLISTResponse();
    void RETRResponse();
    void RETRComplete();
    void STORResponse();
    void STORComplete();
    void ABORResponse();
    void QUITResponse();
    void WIPEResponse();
    void DELEResponse();
    void RMDResponse();
    void NUKEResponse();
    void LISTResponse();
    void LISTComplete();
    void SSCNONResponse();
    void SSCNOFFResponse();
    void PASVAbortResponse();
    void PBSZ0Response();
    void TYPEIResponse();
    void XDUPEResponse();
    void RNFRResponse();
    void RNTOResponse();
    void proxySessionInit(bool);
    void sendEcho(const std::string &);
    void connectAllAddresses();
    Proxy* getProxy() const;
    void clearConnectors();
    void rawBufWrite(const std::string &);
    void rawBufWriteLine(const std::string &);
    void doCWD(const Path& path, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co);
    void doMKD(const Path& path, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co);
    void parseXDUPEData();
    void finishLogin();
    void FDData(int sockid, char* data, unsigned int datalen) override;
    void FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
    void FDSSLSuccess(int sockid, const std::string& cipher) override;
    void ftpConnectInfo(int id, const std::string& info) override;
    void ftpConnectSuccess(int id, const Address& addr) override;
    void ftpConnectFail(int id) override;
    void tick(int message) override;
  public:
    int getId() const;
    void setId(int);
    std::string getStatus() const;
    void login();
    void reconnect();
    bool isProcessing() const;
    FTPConn(SiteLogic *, int);
    ~FTPConn();
    void updateName();
    const Path & getCurrentPath() const;
    void doUSER(bool);
    void doAUTHTLS();
    void doPWD();
    void doPROTP();
    void doPROTC();
    void doRaw(const std::string &);
    void doWipe(const Path &, bool);
    void doNuke(const Path &, int, const std::string &);
    void doDELE(const Path &);
    void doRMD(const Path &);
    void doSTAT();
    void doSTAT(const std::shared_ptr<CommandOwner>& co, const std::shared_ptr<FileList>& fl);
    void doLIST();
    void doLISTa();
    std::shared_ptr<FileList> newFileList() const;
    void setListData(const std::shared_ptr<CommandOwner>& co, const std::shared_ptr<FileList>& fl);
    void doSTATla();
    void doSSCN(bool);
    void doCPSV();
    void doPASV();
    void doEPSV();
    void doEPRT(const std::string& host, int port);
    void doPORT(const std::string& host, int port);
    void doCEPRON();
    void doCWD(const Path &, const std::shared_ptr<CommandOwner> & co = std::shared_ptr<CommandOwner>());
    void doCWD(const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co = std::shared_ptr<CommandOwner>());
    void doMKD(const Path &, const std::shared_ptr<CommandOwner> & co = std::shared_ptr<CommandOwner>());
    void doMKD(const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co);
    void doPRETRETR(const std::string &);
    void doRETR(const std::string &);
    void doPRETSTOR(const std::string &);
    void doSTOR(const std::string &);
    void doPRETLIST();
    void abortTransfer();
    void abortTransferPASV();
    void doPBSZ0();
    void doTYPEI();
    void doXDUPE();
    void doQUIT();
    void doRNFR(const std::string& from);
    void doRNTO(const std::string& to);
    void disconnect();
    FTPConnState getState() const;
    std::string getConnectedAddress() const;
    std::string getInterfaceAddress() const;
    bool isIPv6() const;
    ProtMode getProtectedMode() const;
    bool getSSCNMode() const;
    bool getCEPREnabled() const;
    void setMKDCWDTarget(const Path &, const Path &);
    bool hasMKDCWDTarget() const;
    const Path & getTargetPath() const;
    const Path & getMKDCWDTargetSection() const;
    const Path & getMKDCWDTargetPath() const;
    void finishMKDCWDTarget();
    const std::list<std::string> & getMKDSubdirs();
    RawBuffer * getRawBuffer() const;
    RawBuffer * getCwdRawBuffer() const;
    int getSockId() const;
    bool isCleanlyClosed() const;
    static bool parseData(char*, unsigned int, char**, unsigned int&, unsigned int&, int&);
    void printCipher(const std::string& cipher);
    void printLocalError(const std::string& info);
    std::shared_ptr<FileList> currentFileList() const;
    const std::shared_ptr<CommandOwner> & currentCommandOwner() const;
    void resetCurrentCommandOwner();
    void parseFileList(char *, unsigned int);
    bool isConnected() const;
    void setRawBufferCallback(RawBufferCallback *);
    void unsetRawBufferCallback();
    const std::list<std::string> & getXDUPEList() const;
    Proxy* getDataProxy() const;
};
