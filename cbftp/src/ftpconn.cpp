#include "ftpconn.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "core/workmanager.h"
#include "core/iomanager.h"
#include "core/tickpoke.h"
#include "ftpconnect.h"
#include "filelist.h"
#include "site.h"
#include "globalcontext.h"
#include "rawbuffer.h"
#include "sitelogic.h"
#include "eventlog.h"
#include "proxymanager.h"
#include "util.h"

#define FTPCONN_TICK_INTERVAL 1000

const char * xdupematch = "X-DUPE: ";
const char * dupematch1 = "uploaded by";
const char * dupematch2 = "ile exists";
const size_t xdupematchlen = strlen(xdupematch);

void fromPASVString(std::string pasv, std::string & host, int & port) {
  size_t sep1 = pasv.find(",");
  size_t sep2 = pasv.find(",", sep1 + 1);
  size_t sep3 = pasv.find(",", sep2 + 1);
  size_t sep4 = pasv.find(",", sep3 + 1);
  size_t sep5 = pasv.find(",", sep4 + 1);
  pasv[sep1] = '.';
  pasv[sep2] = '.';
  pasv[sep3] = '.';
  host = pasv.substr(0, sep4);
  int major = std::stoi(pasv.substr(sep4 + 1, sep5 - sep4 + 1));
  int minor = std::stoi(pasv.substr(sep5 + 1));
  port = major * 256 + minor;
}

std::string toPASVString(const std::string & addr, int port) {
  std::string pasv = addr;
  size_t pos;
  while ((pos = pasv.find(".")) != std::string::npos) {
    pasv[pos] = ',';
  }
  int portfirst = port / 256;
  int portsecond = port % 256;
  return pasv + "," + std::to_string(portfirst) + "," + std::to_string(portsecond);
}

FTPConn::FTPConn(SiteLogic * sl, int id) :
  nextconnectorid(0),
  iom(global->getIOManager()),
  databuflen(DATA_BUF_SIZE),
  databuf((char *) malloc(databuflen)),
  databufpos(0),
  id(id),
  processing(false),
  allconnectattempted(false),
  sl(sl),
  status("disconnected"),
  site(sl->getSite()),
  sockid(-1),
  state(FTPConnState::DISCONNECTED),
  aborted(false),
  currentfl(NULL),
  currentco(NULL),
  currentpath("/"),
  protectedmode(ProtMode::UNSET),
  sscnmode(false),
  ceprenabled(false),
  mkdtarget(false),
  rawbuf(new RawBuffer(site->getName(), std::to_string(id))),
  aggregatedrawbuf(sl->getAggregatedRawBuffer()),
  cwdrawbuf(new RawBuffer(site->getName())),
  xduperun(false),
  typeirun(false),
  cleanlyclosed(false) {

}

FTPConn::~FTPConn() {
  global->getTickPoke()->stopPoke(this, 0);
  if (isConnected()) {
    iom->closeSocket(sockid);
  }
  delete rawbuf;
  delete databuf;
  clearConnectors();
}

int FTPConn::getId() const {
  return id;
}

void FTPConn::setId(int id) {
  this->id = id;
  rawbuf->setId(id);
}

std::string FTPConn::getStatus() const {
  return status;
}

void FTPConn::login() {
  if (state != FTPConnState::DISCONNECTED && state != FTPConnState::QUIT) {
    return;
  }
  protectedmode = ProtMode::UNSET;
  sscnmode = false;
  ceprenabled = false;
  mkdtarget = false;
  databufpos = 0;
  processing = true;
  allconnectattempted = false;
  sockid = -1;
  xduperun = false;
  typeirun = false;
  cleanlyclosed = false;
  cwdrawbuf->clear();
  currentpath = "/";
  state = FTPConnState::CONNECTING;
  clearConnectors();
  connectors.push_back(std::make_shared<FTPConnect>(nextconnectorid++, this, site->getAddress(), getProxy(), true, site->getTLSMode() == TLSMode::IMPLICIT));
  ticker = 0;
  global->getTickPoke()->startPoke(this, "FTPConn", FTPCONN_TICK_INTERVAL, 0);
}

void FTPConn::connectAllAddresses() {
  allconnectattempted = true;
  std::list<Address> addresses = site->getAddresses();
  Proxy * proxy = getProxy();
  for (std::list<Address>::const_iterator it = addresses.begin(); it != addresses.end(); it++) {
    if (it == addresses.begin()) continue; // first one is already connected
    connectors.push_back(std::make_shared<FTPConnect>(nextconnectorid++, this, *it, proxy, false, site->getTLSMode() == TLSMode::IMPLICIT));
  }
}

Proxy* FTPConn::getProxy() const {
  Proxy* proxy = nullptr;
  int proxytype = site->getProxyType();
  if (proxytype == SITE_PROXY_USE) {
    proxy = global->getProxyManager()->getProxy(site->getProxy());
  }
  else if (proxytype == SITE_PROXY_GLOBAL) {
    proxy = global->getProxyManager()->getDefaultProxy();
  }
  return proxy;
}

Proxy* FTPConn::getDataProxy() const {
  Proxy* proxy = nullptr;
  int proxytype = site->getDataProxyType();
  if (proxytype == SITE_PROXY_USE) {
    proxy = global->getProxyManager()->getProxy(site->getDataProxy());
  }
  else if (proxytype == SITE_PROXY_GLOBAL) {
    proxy = global->getProxyManager()->getDefaultDataProxy();
  }
  return proxy;
}

void FTPConn::clearConnectors() {
  std::list<std::shared_ptr<FTPConnect> >::const_iterator it;
  for (std::list<std::shared_ptr<FTPConnect> >::const_iterator it = connectors.begin(); it != connectors.end(); it++) {
    (*it)->disengage();
    global->getWorkManager()->deferDelete(*it);
  }
  connectors.clear();
}

void FTPConn::FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  if (this->sockid != sockid) {
    return;
  }
  if (state != FTPConnState::DISCONNECTED) {
    rawBufWriteLine("[Disconnected: " + details + "]");
    this->status = "disconnected";
    state = FTPConnState::DISCONNECTED;
    sl->disconnected(id);
  }
  if (reason == Core::DisconnectType::ERROR) {
    global->getEventLog()->log("FTPConn", site->getName() + " " + std::to_string(id) + ": " + details, Core::LogLevel::ERROR);
  }
}

void FTPConn::FDSSLSuccess(int sockid, const std::string & cipher) {
  printCipher(cipher);
  if (state == FTPConnState::AUTH_TLS) {
    doUSER(false);
  }
  else {
    state = FTPConnState::IDLE;
  }
}

void FTPConn::printCipher(const std::string& cipher) {
  rawBufWriteLine("[Cipher: " + cipher + "]");
}

void FTPConn::printLocalError(const std::string& info) {
  rawBufWriteLine("[Error: " + info + "]");
}

bool FTPConn::parseData(char * data, unsigned int datalen, char ** databuf, unsigned int & databuflen, unsigned int & databufpos, int & databufcode) {
  if (databufpos + datalen > databuflen) {
    while (databufpos + datalen > databuflen) {
      databuflen *= 2;
    }
    char * newdatabuf = (char *) malloc(databuflen);
    memcpy(newdatabuf, *databuf, databufpos);
    delete *databuf;
    *databuf = newdatabuf;
  }
  memcpy(*databuf + databufpos, data, datalen);
  databufpos += datalen;
  bool messagecomplete = false;
  char * loc = 0;
  if((*databuf)[databufpos - 1] == '\n') {
    loc = *databuf + databufpos - 5;
    while (loc >= *databuf) {
      if (isdigit(*loc) && isdigit(*(loc+1)) && isdigit(*(loc+2))) {
        if ((*(loc+3) == ' ' || *(loc+3) == '\n') && (loc == *databuf || *(loc-1) == '\n')) {
          messagecomplete = true;
          databufcode = atoi(std::string(loc, 3).data());
          break;
        }
      }
      --loc;
    }
  }
  if (messagecomplete) {
    if (databufcode == 550) {
      // workaround for a glftpd bug causing an extra row '550 Unable to load your own user file!.' on retr/stor
      if (*(loc+4) == 'U' && *(loc+5) == 'n' && *(loc+28) == 'u' && *(loc+33) == 'f') {
        databufpos = 0;
        return false;
      }
    }
  }
  else if (datalen == databufpos && datalen >= 3 && (!isdigit(**databuf) ||
           !isdigit(*(*databuf+1)) || !isdigit(*(*databuf+2))))
  {
    databufcode = 0;
    return true;
  }
  return messagecomplete;
}

void FTPConn::FDData(int sockid, char * data, unsigned int datalen) {
  if (this->sockid != sockid) {
    return;
  }
  if (state != FTPConnState::STAT && state != FTPConnState::STOR && state != FTPConnState::PRET_STOR) {
    rawBufWrite(std::string(data, datalen));
  }
  if (state == FTPConnState::CWD) {
    cwdrawbuf->write(std::string(data, datalen));
  }
  if (parseData(data, datalen, &databuf, databuflen, databufpos, databufcode)) {
    switch(state) {
      case FTPConnState::AUTH_TLS: // awaiting AUTH TLS response
        AUTHTLSResponse();
        break;
      case FTPConnState::USER: // awaiting USER response
        USERResponse();
        break;
      case FTPConnState::PASS: // awaiting PASS response
        PASSResponse();
        break;
      case FTPConnState::STAT: // awaiting STAT response
        STATResponse();
        break;
      case FTPConnState::PWD: // awaiting PWD response
        PWDResponse();
        break;
      case FTPConnState::PROT_P: // awaiting PROT P response
        PROTPResponse();
        break;
      case FTPConnState::PROT_C:  // awaiting PROT C response
        PROTCResponse();
        break;
      case FTPConnState::RAW: // awaiting raw response
        RawResponse();
        break;
      case FTPConnState::CPSV: // awaiting CPSV response
        CPSVResponse();
        break;
      case FTPConnState::PASV: // awaiting PASV response
        PASVResponse();
        break;
      case FTPConnState::PORT: // awaiting PORT response
        PORTResponse();
        break;
      case FTPConnState::EPSV: // awaiting EPSV response
        EPSVResponse();
        break;
      case FTPConnState::EPRT: // awaiting EPRT response
        EPRTResponse();
        break;
      case FTPConnState::CEPR_ON: // awaiting CEPR ON response
        CEPRONResponse();
        break;
      case FTPConnState::CWD: // awaiting CWD response
        CWDResponse();
        break;
      case FTPConnState::MKD: // awaiting MKD response
        MKDResponse();
        break;
      case FTPConnState::PRET_RETR: // awaiting PRET RETR response
        PRETRETRResponse();
        break;
      case FTPConnState::PRET_STOR: // awaiting PRET STOR response
        PRETSTORResponse();
        break;
      case FTPConnState::RETR: // awaiting RETR response
        RETRResponse();
        break;
      case FTPConnState::RETR_COMPLETE: // awaiting RETR complete
        RETRComplete();
        break;
      case FTPConnState::STOR: // awaiting STOR response
        STORResponse();
        break;
      case FTPConnState::STOR_COMPLETE: // awaiting STOR complete
        STORComplete();
        break;
      case FTPConnState::ABOR: // awaiting ABOR response
        ABORResponse();
        break;
      case FTPConnState::QUIT: // awaiting QUIT response
        QUITResponse();
        break;
      case FTPConnState::USER_LOGINKILL: // awaiting loginkilling USER response
        USERResponse();
        break;
      case FTPConnState::PASS_LOGINKILL: // awaiting loginkilling PASS response
        PASSResponse();
        break;
      case FTPConnState::WIPE: // awaiting WIPE response
        WIPEResponse();
        break;
      case FTPConnState::DELE: // awaiting DELE response
        DELEResponse();
        break;
      case FTPConnState::RMD: // awaiting RMD response
        RMDResponse();
        break;
      case FTPConnState::NUKE: // awaiting SITE NUKE response
        NUKEResponse();
        break;
      case FTPConnState::LIST: // awaiting LIST response
        LISTResponse();
        break;
      case FTPConnState::PRET_LIST: // awaiting PRET LIST response
        PRETLISTResponse();
        break;
      case FTPConnState::LIST_COMPLETE: // awaiting LIST complete
        LISTComplete();
        break;
      case FTPConnState::SSCN_ON: // awaiting SSCN ON response
        SSCNONResponse();
        break;
      case FTPConnState::SSCN_OFF: // awaiting SSCN OFF response
        SSCNOFFResponse();
        break;
      case FTPConnState::PASV_ABORT: // awaiting aborting PASV
        PASVAbortResponse();
        break;
      case FTPConnState::PBSZ: // awaiting PBSZ 0 response
        PBSZ0Response();
        break;
      case FTPConnState::TYPEI: // awaiting TYPE I response
        TYPEIResponse();
        break;
      case FTPConnState::XDUPE: // awaiting XDUPE response
        XDUPEResponse();
        break;
      case FTPConnState::RNFR: // awaiting RNFR response
        RNFRResponse();
        break;
      case FTPConnState::RNTO: // awaiting RNTO response
        RNTOResponse();
        break;
      default: // nothing expected at this time, discard
        break;
    }
    if (!isProcessing()) {
      currentco.reset();
      currentfl = NULL;
      if (isConnected()) {
        state = FTPConnState::IDLE;
      }
    }
    databufpos = 0;
  }
}

void FTPConn::ftpConnectInfo(int connectorid, const std::string& info) {
  rawBufWriteLine(info);
}

void FTPConn::ftpConnectSuccess(int connectorid, const Address& addr) {
  if (state != FTPConnState::CONNECTING) {
    return;
  }
  std::shared_ptr<FTPConnect> connector;
  for (const std::shared_ptr<FTPConnect>& it : connectors) {
    if (it->getId() == connectorid) {
      connector = it;
      break;
    }
  }
  assert(connector);
  sockid = connector->handedOver();
  connectedaddr = addr;
  iom->adopt(this, sockid);
  global->getTickPoke()->stopPoke(this, 0);
  if (!connector->isPrimary()) {
    site->setPrimaryAddress(connector->getAddress());
    rawBufWriteLine("[Setting " + connector->getAddress().toString() + " as primary address]");
  }
  if (connectors.size() > 1) {
    rawBufWriteLine("[Disconnecting other attempts]");
  }
  clearConnectors();
  if (site->getTLSMode() == TLSMode::AUTH_TLS) {
    state = FTPConnState::AUTH_TLS;
    sendEcho("AUTH TLS");
  }
  else {
    doUSER(false);
  }
}

void FTPConn::ftpConnectFail(int connectorid) {
  if (state != FTPConnState::CONNECTING) {
    return;
  }
  std::list<std::shared_ptr<FTPConnect> >::iterator it;
  for (it = connectors.begin(); it != connectors.end(); it++) {
    if ((*it)->getId() == connectorid) {
      break;
    }
  }
  assert(it != connectors.end());
  bool primary = (*it)->isPrimary();
  global->getWorkManager()->deferDelete(*it);
  connectors.erase(it);
  if (primary) {
    if (site->getAddresses().size() > 1 && !allconnectattempted) {
      connectAllAddresses();
    }
  }
  if (!connectors.size()) {
    global->getTickPoke()->stopPoke(this, 0);
    state = FTPConnState::DISCONNECTED;
    sl->connectFailed(id);
  }
}

void FTPConn::tick(int message) {
  ticker += FTPCONN_TICK_INTERVAL;
  std::list<std::shared_ptr<FTPConnect> > ticklist = connectors;
  for (std::list<std::shared_ptr<FTPConnect> >::const_iterator it = ticklist.begin(); it != ticklist.end(); it++) {
    (*it)->tickIntern();
  }
  if (ticker >= 1000) {
    if (state == FTPConnState::CONNECTING && !allconnectattempted) {
      connectAllAddresses();
    }
  }
  if (!connectors.size()) {
    global->getTickPoke()->stopPoke(this, 0);
  }
}

void FTPConn::sendEcho(const std::string & data) {
  rawBufWriteLine(data);
  processing = true;
  status = data;
  iom->sendData(sockid, data + "\r\n");
}

void FTPConn::AUTHTLSResponse() {
  if (databufcode == 234) {
    iom->negotiateSSLConnect(sockid);
  }
  else {
    rawBufWriteLine("[Unknown response]");
    processing = false;
    sl->TLSFailed(id);
  }
}

void FTPConn::doUSER(bool killer) {
  if (!killer) {
    state = FTPConnState::USER;
  }
  else {
    state = FTPConnState::USER_LOGINKILL;
  }
  sendEcho((std::string("USER ") + (killer ? "!" : "") + site->getUser()).data());
}

void FTPConn::USERResponse() {
  if (databufcode == 331) {
    std::string pass = site->getPass();
    std::string passc = "";
    for (unsigned int i = 0; i < pass.length(); i++) passc.append("*");
    std::string output = "PASS " + std::string(passc);
    rawBufWriteLine(output);
    status = output;
    if (state == FTPConnState::USER) {
      state = FTPConnState::PASS;
    }
    else {
      state = FTPConnState::PASS_LOGINKILL;
    }
    iom->sendData(sockid, std::string("PASS ") + site->getPass()  + "\r\n");
  }
  else {
    processing = false;
    bool sitefull = false;
    bool simultaneous = false;
    std::string reply = std::string(databuf, databufpos);
    if (databufcode == 530 || databufcode == 550) {
      if (reply.find("site") != std::string::npos && reply.find("full") != std::string::npos) {
        sitefull = true;
      }
      else if (reply.find("simultaneous") != std::string::npos) {
        simultaneous = true;
      }
    }
    if (sitefull) {
      sl->userDeniedSiteFull(id);
    }
    else if (state == FTPConnState::USER) {
      if (simultaneous) {
        sl->userDeniedSimultaneousLogins(id);
      }
      else {
        sl->userDenied(id);
      }
    }
    else {
      sl->loginKillFailed(id);
    }
  }
}

void FTPConn::PASSResponse() {
  processing = false;
  this->status = "connected";
  if (databufcode == 230) {
    finishLogin();
  }
  else {
    bool sitefull = false;
    bool simultaneous = false;
    std::string reply = std::string(databuf, databufpos);
    if (databufcode == 530 || databufcode == 550) {
      if (reply.find("site") != std::string::npos && reply.find("full") != std::string::npos) {
        sitefull = true;
      }
      else if (reply.find("simultaneous") != std::string::npos) {
        simultaneous = true;
      }
    }
    if (sitefull) {
      sl->userDeniedSiteFull(id);
    }
    else if (state == FTPConnState::PASS) {
      if (simultaneous) {
        sl->userDeniedSimultaneousLogins(id);
      }
      else {
        sl->passDenied(id);
      }
    }
    else {
      sl->loginKillFailed(id);
    }
  }
}

void FTPConn::finishLogin() {
  if (site->forceBinaryMode() && !typeirun) {
    doTYPEI();
  }
  else if (site->useXDUPE() && !xduperun) {
    doXDUPE();
  }
  else {
    state = FTPConnState::LOGIN;
    sl->commandSuccess(id, state);
  }
}

void FTPConn::TYPEIResponse() {
  processing = false;
  if (databufcode == 200) {
    typeirun = true;
    finishLogin();
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::XDUPEResponse() {
  processing = false;
  xduperun = true;
  finishLogin();
}

void FTPConn::RNFRResponse() {
  processing = false;
  if (databufcode == 350) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::RNTOResponse() {
  processing = false;
  if (databufcode == 250) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::reconnect() {
  disconnect();
  login();
}

void FTPConn::doSTAT() {
  state = FTPConnState::STAT;
  currentco.reset();
  currentfl = newFileList();
  sendEcho("STAT -l");
}

void FTPConn::doSTAT(const std::shared_ptr<CommandOwner> & co, const std::shared_ptr<FileList>& fl) {
  state = FTPConnState::STAT;
  currentco = co;
  currentfl = fl;
  sendEcho("STAT -l");
}

void FTPConn::doSTATla() {
  state = FTPConnState::STAT;
  currentco.reset();
  currentfl = newFileList();
  sendEcho("STAT -la");
}

std::shared_ptr<FileList> FTPConn::newFileList() const {
  return std::make_shared<FileList>(site->getUser(), currentpath);
}

void FTPConn::setListData(const std::shared_ptr<CommandOwner> & co, const std::shared_ptr<FileList>& fl) {
  currentco = co;
  currentfl = fl;
}

void FTPConn::doLIST() {
  state = FTPConnState::LIST;
  sendEcho("LIST");
}

void FTPConn::doLISTa() {
  state = FTPConnState::LIST;
  sendEcho("LIST -a");
}

void FTPConn::STATResponse() {
  processing = false;
  if (databufcode == 211 || databufcode == 212 || databufcode == 213) {
    parseFileList(databuf, databufpos);
    std::string output = "[File list retrieved]";
    rawBufWriteLine(output);
    sl->listRefreshed(id);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::LISTResponse() {
  if (databufcode == 150 || databufcode == 125) {
    state = FTPConnState::LIST_COMPLETE;
    sl->commandSuccess(id, FTPConnState::LIST);
  }
  else {
    processing = false;
    sl->commandFail(id);
  }
}

void FTPConn::LISTComplete() {
  processing = false;
  if (databufcode == 226) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::updateName() {
  rawbuf->rename(site->getName());
}

const Path & FTPConn::getCurrentPath() const {
  return currentpath;
}

void FTPConn::doPWD() {
  state = FTPConnState::PWD;
  sendEcho("PWD");
}

void FTPConn::PWDResponse() {
  processing = false;
  if (databufcode == 257) {
    std::string line(databuf, databufpos);
    int loc = 0;
    while(line[++loc] != '"');
    int start = loc + 1;
    while(line[++loc] != '"');
    sl->gotPath(id, line.substr(start, loc - start));
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doPROTP() {
  state = FTPConnState::PROT_P;
  sendEcho("PROT P");
}

void FTPConn::PROTPResponse() {
  processing = false;
  if (databufcode == 200) {
    protectedmode = ProtMode::P;
    sl->commandSuccess(id, state);
  }
  else {
    if (databufcode == 503) {
      std::string line(databuf, databufpos);
      if (line.find("PBSZ") != std::string::npos) {
        doPBSZ0();
        return;
      }
    }
    sl->commandFail(id);
  }
}

void FTPConn::doPROTC() {
  state = FTPConnState::PROT_C;
  sendEcho("PROT C");
}

void FTPConn::PROTCResponse() {
  processing = false;
  if (databufcode == 200) {
    protectedmode = ProtMode::C;
    sl->commandSuccess(id, state);
  }
  else {
    if (databufcode == 503) {
      std::string line(databuf, databufpos);
      if (line.find("PBSZ") != std::string::npos) {
        doPBSZ0();
        return;
      }
    }
    sl->commandFail(id);
  }
}

void FTPConn::doSSCN(bool on) {
  if (on) {
    state = FTPConnState::SSCN_ON;
    sendEcho("SSCN ON");
  }
  else {
    state = FTPConnState::SSCN_OFF;
    sendEcho("SSCN OFF");
  }
}

void FTPConn::SSCNONResponse() {
  processing = false;
  if (databufcode == 200) {
    sscnmode = true;
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::SSCNOFFResponse() {
  processing = false;
  if (databufcode == 200) {
    sscnmode = false;
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doRaw(const std::string & command) {
  state = FTPConnState::RAW;
  sendEcho(command.c_str());
}

void FTPConn::doWipe(const Path & path, bool recursive) {
  state = FTPConnState::WIPE;
  sendEcho(std::string("SITE WIPE ") + (recursive ? "-r " : "") + path.toString());
}

void FTPConn::doNuke(const Path & path, int multiplier, const std::string & reason) {
  state = FTPConnState::NUKE;
  sendEcho("SITE NUKE " + path.toString() + " " + std::to_string(multiplier) + " " + reason);
}

void FTPConn::doDELE(const Path & path) {
  state = FTPConnState::DELE;
  sendEcho("DELE " + path.toString());
}

void FTPConn::doRMD(const Path & path) {
  state = FTPConnState::RMD;
  sendEcho("RMD " + path.toString());
}

void FTPConn::doPBSZ0() {
  state = FTPConnState::PBSZ;
  sendEcho("PBSZ 0");
}

void FTPConn::doTYPEI() {
  state = FTPConnState::TYPEI;
  sendEcho("TYPE I");
}

void FTPConn::doXDUPE() {
  state = FTPConnState::XDUPE;
  sendEcho("SITE XDUPE 3");
}

void FTPConn::doRNFR(const std::string& from) {
  state = FTPConnState::RNFR;
  sendEcho("RNFR " + from);
}

void FTPConn::doRNTO(const std::string& to) {
  state = FTPConnState::RNTO;
  sendEcho("RNTO " + to);
}

void FTPConn::PBSZ0Response() {
  processing = false;
  if (databufcode == 200) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::RawResponse() {
  processing = false;
  std::string ret(databuf, databufpos);
  sl->rawCommandResultRetrieved(id, ret);
}

void FTPConn::WIPEResponse() {
  processing = false;
  if (databufcode == 200) {
    std::string data = std::string(databuf, databufpos);
    if (data.find("successfully") != std::string::npos ||
        data.find("okay") != std::string::npos)
    {
      sl->commandSuccess(id, state);
      return;
    }
  }
  sl->commandFail(id);
}

void FTPConn::DELEResponse() {
  processing = false;
  if (databufcode == 250) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::RMDResponse() {
  processing = false;
  if (databufcode == 250) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::NUKEResponse() {
  processing = false;
  if (databufcode == 200) {
    std::string data = std::string(databuf, databufpos);
    if (data.find("uccess") != std::string::npos ||
        data.find("succeeded") != std::string::npos)
    {
      sl->commandSuccess(id, state);
    }
    else {
      sl->commandFail(id);
    }
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doCPSV() {
  state = FTPConnState::CPSV;
  sendEcho("CPSV");
}

void FTPConn::CPSVResponse() {
  PASVResponse();
}

void FTPConn::doPASV() {
  state = FTPConnState::PASV;
  sendEcho("PASV");
}

void FTPConn::PASVResponse() {
  processing = false;
  if (databufcode == 227) {
    std::string data = std::string(databuf, databufpos);
    size_t start = data.find('(') + 1;
    size_t end = data.find(')');
    std::string addr = data.substr(start, end - start);
    int count = 0;
    for (unsigned int i = 0; i < addr.length(); i++) {
      if (addr[i] == ',') count++;
    }
    if (count == 2 && addr.substr(0, 2) == "1,") {
      std::string connaddr = getConnectedAddress();
      for (unsigned int i = 0; i < connaddr.length(); i++) {
        if (connaddr[i] == '.') connaddr[i] = ',';
      }
      addr = connaddr + "," + addr.substr(2);
    }
    std::string host;
    int port;
    fromPASVString(addr, host, port);
    sl->gotPassiveAddress(id, host, port);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doPORT(const std::string& host, int port) {
  state = FTPConnState::PORT;
  std::string addr = toPASVString(host, port);
  sendEcho(("PORT " + addr).c_str());
}

void FTPConn::PORTResponse() {
  processing = false;
  if (databufcode == 200) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doEPSV() {
  state = FTPConnState::EPSV;
  sendEcho("EPSV 2");
}

void FTPConn::EPSVResponse() {
  processing = false;
  if (databufcode == 229) {
    std::string data = std::string(databuf, databufpos);
    size_t tok1 = data.find("(|");
    if (tok1 == std::string::npos) {
      sl->commandFail(id);
      return;
    }
    size_t tok2 = data.find('|', tok1 + 2);
    if (tok2 == std::string::npos) {
      sl->commandFail(id);
      return;
    }
    size_t tok3 = data.find('|', tok2 + 1);
    if (tok3 == std::string::npos) {
      sl->commandFail(id);
      return;
    }
    size_t tok4 = data.find("|)", tok3 + 1);
    if (tok4 == std::string::npos) {
      sl->commandFail(id);
      return;
    }
    if (data.substr(tok1 + 2, tok2 - (tok1 + 2)) != "2") {
      sl->commandFail(id);
      return;
    }
    std::string host = data.substr(tok2 + 1, tok3 - (tok2 + 1));
    if (host.empty()) {
      host = connectedaddr.host;
    }
    if (host.find(":") == std::string::npos) {
      sl->commandFail(id);
      return;
    }
    host = Core::IOManager::compactIPv6Address(host);
    int port = 0;
    try {
      port = std::stoi(data.substr(tok3 + 1, tok4 - (tok3 + 1)));
    }
    catch (std::exception&) {
      sl->commandFail(id);
      return;
    }
    sl->gotPassiveAddress(id, host, port);
    return;
  }
  sl->commandFail(id);
}

void FTPConn::doEPRT(const std::string& host, int port) {
  state = FTPConnState::EPRT;
  sendEcho("EPRT |2|" + host + "|" + std::to_string(port) + "|");
}

void FTPConn::EPRTResponse() {
  processing = false;
  if (databufcode == 200) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doCEPRON() {
  state = FTPConnState::CEPR_ON;
  sendEcho("CEPR ON");
}

void FTPConn::CEPRONResponse() {
  processing = false;
  ceprenabled = true;
  sl->commandSuccess(id, state);
}

void FTPConn::doCWD(const Path & path, const std::shared_ptr<CommandOwner> & co) {
  doCWD(path, nullptr, co);
}

void FTPConn::doCWD(const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co) {
  doCWD(fl->getPath(), fl, co);
}

void FTPConn::doCWD(const Path& path, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co) {
  assert(path != "");
  currentfl = fl;
  currentco = co;
  targetpath = path;
  if (targetpath == currentpath) {
    global->getEventLog()->log("FTPConn " + site->getName() + std::to_string(id),
        "WARNING: Noop CWD requested: " + path.toString());
    return;
  }
  state = FTPConnState::CWD;
  std::string command = ("CWD " + path.toString()).c_str();
  cwdrawbuf->clear();
  cwdrawbuf->writeLine(command);
  sendEcho(command);
}

void FTPConn::CWDResponse() {
  processing = false;
  if (databufcode == 250) {
    currentpath = targetpath;
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doMKD(const Path & dir, const std::shared_ptr<CommandOwner> & co) {
  doMKD(dir, nullptr, co);
}

void FTPConn::doMKD(const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co) {
  doMKD(fl->getPath(), fl, co);
}

void FTPConn::doMKD(const Path& dir, const std::shared_ptr<FileList>& fl, const std::shared_ptr<CommandOwner>& co) {
  assert(dir != "");
  currentfl = fl;
  currentco = co;
  targetpath = dir;
  state = FTPConnState::MKD;
  sendEcho(("MKD " + targetpath.toString()).c_str());
}

void FTPConn::MKDResponse() {
  processing = false;
  if (databufcode == 257) {
    sl->commandSuccess(id, state);
  }
  else {
    if (databufcode == 550) {
      std::string message(databuf, databufpos);
      if (message.find("File exist") != std::string::npos ||
          message.find("already exists") != std::string::npos)
      {
        sl->commandSuccess(id, state);
        return;
      }
    }
    sl->commandFail(id);
  }
}

void FTPConn::doPRETRETR(const std::string & file) {
  state = FTPConnState::PRET_RETR;
  sendEcho(("PRET RETR " + file).c_str());
}

void FTPConn::PRETRETRResponse() {
  processing = false;
  if (databufcode == 200) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doPRETSTOR(const std::string & file) {
  state = FTPConnState::PRET_STOR;
  sendEcho(("PRET STOR " + file).c_str());
}

void FTPConn::PRETSTORResponse() {
  processing = false;
  std::string response = std::string(databuf, databufpos);
  if (databufcode == 553) {
    bool dupe = false;
    if (response.find(xdupematch) != std::string::npos) {
      parseXDUPEData();
      dupe = true;
    }
    else if (response.find(dupematch1) != std::string::npos ||
             response.find(dupematch2) != std::string::npos)
    {
      dupe = true;
    }
    if (dupe) {
      rawBufWrite(std::string(databuf, databufpos));
      sl->commandFail(id, FailureType::DUPE);
      return;
    }
  }
  rawBufWrite(response);
  if (databufcode == 200) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doPRETLIST() {
  state = FTPConnState::PRET_LIST;
  sendEcho("PRET LIST");
}

void FTPConn::PRETLISTResponse() {
  processing = false;
  if (databufcode == 200) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doRETR(const std::string & file) {
  state = FTPConnState::RETR;
  sendEcho(("RETR " + file).c_str());
}

void FTPConn::RETRResponse() {
  if (databufcode == 150 || databufcode == 125) {
    state = FTPConnState::RETR_COMPLETE;
    sl->commandSuccess(id, FTPConnState::RETR);
  }
  else {
    processing = false;
    sl->commandFail(id);
  }
}

void FTPConn::RETRComplete() {
  processing = false;
  if (databufcode == 226) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::doSTOR(const std::string & file) {
  state = FTPConnState::STOR;
  sendEcho(("STOR " + file).c_str());
}

void FTPConn::STORResponse() {
  std::string response = std::string(databuf, databufpos);
  if (databufcode == 550 || databufcode == 553) {
    processing = false;
    bool dupe = false;
    if (response.find(xdupematch) != std::string::npos) {
      parseXDUPEData();
      dupe = true;
    }
    else if (response.find(dupematch1) != std::string::npos ||
             response.find(dupematch2) != std::string::npos)
    {
      dupe = true;
    }
    if (dupe) {
      rawBufWrite(std::string(databuf, databufpos));
      sl->commandFail(id, FailureType::DUPE);
      return;
    }
  }
  rawBufWrite(response);
  if (databufcode == 150 || databufcode == 125) {
    state = FTPConnState::STOR_COMPLETE;
    sl->commandSuccess(id, FTPConnState::STOR);
  }
  else {
    processing = false;
    sl->commandFail(id);
  }
}

void FTPConn::STORComplete() {
  processing = false;
  if (databufcode == 226) {
    sl->commandSuccess(id, state);
  }
  else {
    sl->commandFail(id);
  }
}

void FTPConn::abortTransfer() {
  state = FTPConnState::ABOR;
  sendEcho("ABOR");
}

void FTPConn::abortTransferPASV() {
  state = FTPConnState::PASV_ABORT;
  sendEcho("PASV");
}

void FTPConn::ABORResponse() {
  processing = false;
  sl->commandSuccess(id, state);
}

void FTPConn::PASVAbortResponse() {
  processing = false;
  sl->commandSuccess(id, state);
}

void FTPConn::doQUIT() {
  if (state != FTPConnState::DISCONNECTED) {
    state = FTPConnState::QUIT;
    sendEcho("QUIT");
    cleanlyclosed = true;
  }
}

void FTPConn::QUITResponse() {
  processing = false;
  disconnect();
  state = FTPConnState::DISCONNECTED;
  sl->commandSuccess(id, FTPConnState::QUIT);
}

void FTPConn::disconnect() {
  if (state == FTPConnState::CONNECTING) {
    global->getTickPoke()->stopPoke(this, 0);
  }
  if (state != FTPConnState::DISCONNECTED) {
    state = FTPConnState::DISCONNECTED;
    processing = false;
    cleanlyclosed = true;
    iom->closeSocket(sockid);
    clearConnectors();
    this->status = "disconnected";
    rawBufWriteLine("[Disconnected]");
  }
}

RawBuffer * FTPConn::getRawBuffer() const {
  return rawbuf;
}

RawBuffer * FTPConn::getCwdRawBuffer() const {
  return cwdrawbuf;
}

int FTPConn::getSockId() const {
  return sockid;
}

bool FTPConn::isCleanlyClosed() const {
  return cleanlyclosed;
}

FTPConnState FTPConn::getState() const {
  return state;
}

std::string FTPConn::getConnectedAddress() const {
  return iom->getSocketAddress(sockid);
}

std::string FTPConn::getInterfaceAddress() const {
  return iom->getInterfaceAddress(sockid);
}

bool FTPConn::isIPv6() const {
  return iom->getAddressFamily(sockid) == Core::AddressFamily::IPV6;
}

ProtMode FTPConn::getProtectedMode() const {
  return protectedmode;
}

bool FTPConn::getSSCNMode() const {
  return sscnmode;
}

bool FTPConn::getCEPREnabled() const {
  return ceprenabled;
}

void FTPConn::setMKDCWDTarget(const Path & section, const Path & subpath) {
  mkdtarget = true;
  mkdsect = section;
  mkdpath = subpath;
  mkdsubdirs = mkdpath.split();
}

void FTPConn::finishMKDCWDTarget() {
  mkdtarget = false;
}

bool FTPConn::hasMKDCWDTarget() const {
  return mkdtarget;
}

const Path & FTPConn::getTargetPath() const {
  return targetpath;
}

const Path & FTPConn::getMKDCWDTargetSection() const {
  return mkdsect;
}

const Path & FTPConn::getMKDCWDTargetPath() const {
  return mkdpath;
}

const std::list<std::string> & FTPConn::getMKDSubdirs() {
  return mkdsubdirs;
}

std::shared_ptr<FileList> FTPConn::currentFileList() const {
  return currentfl;
}

const std::shared_ptr<CommandOwner> & FTPConn::currentCommandOwner() const {
  return currentco;
}

void FTPConn::resetCurrentCommandOwner() {
  currentco.reset();
}

bool FTPConn::isProcessing() const {
  return processing;
}

void FTPConn::parseFileList(char * buf, unsigned int buflen) {
  char * loc = buf, * start;
  unsigned int files = 0;
  int touch = rand();
  while (loc + 4 < buf + buflen && !(*loc == '2' && *(loc + 1) == '1' && *(loc + 3) == ' ')) {
    if (*loc == '2' && *(loc + 1) == '1' && *(loc + 3) == '-') {
      loc += 4;
    }
    start = loc;
    while(loc < buf + buflen && *loc++ != '\n');
    if (loc - start >= 40) {
      if (currentfl->updateFile(std::string(start, loc - start), touch)) {
        files++;
      }
    }
  }
  if (currentfl->getSize() > files) {
    currentfl->cleanSweep(touch);
  }
  if (currentfl->getState() != FileListState::LISTED) {
    currentfl->setFilled();
  }
  currentfl->setRefreshed();
}

void FTPConn::parseXDUPEData() {
  xdupelist.clear();
  char * loc = databuf;
  int lineendpos;
  int xdupestart = -1;
  int xdupeend = -1;
  while ((lineendpos = util::chrfind(loc, databuf + databufpos - loc, '\n')) != -1) {
    int lastpos = lineendpos;
    if (lineendpos > 0 && loc[lineendpos - 1] == '\r') {
      --lastpos;
    }
    int xdupepos = util::chrstrfind(loc, lastpos, xdupematch, xdupematchlen);
    if (xdupepos != -1) {
      if (xdupestart == -1) {
        xdupestart = loc - databuf;
      }
      xdupelist.emplace_back(loc + xdupepos + xdupematchlen, lastpos - (xdupepos + xdupematchlen));
    }
    else if (xdupestart != -1 && xdupeend == -1) {
      xdupeend = loc - databuf;
    }
    loc += lineendpos + 1;
  }
  if (xdupestart != -1) {
    if (xdupeend == -1) {
      xdupeend = databufpos - 2;
    }
    memmove(databuf + xdupestart + 24, databuf + xdupeend, databufpos - xdupeend);
    memcpy(databuf + xdupestart, "[XDUPE data retrieved]\r\n", 24);
    databufpos -= xdupeend - xdupestart - 24;
  }

}

bool FTPConn::isConnected() const {
  return state != FTPConnState::DISCONNECTED;
}

void FTPConn::rawBufWrite(const std::string & data) {
  rawbuf->write(data);
  aggregatedrawbuf->write(rawbuf->getTag(), data);
}

void FTPConn::rawBufWriteLine(const std::string & data) {
  rawbuf->writeLine(data);
  aggregatedrawbuf->writeLine(rawbuf->getTag(), data);
}

void FTPConn::setRawBufferCallback(RawBufferCallback * callback) {
  rawbuf->setCallback(callback);
}

void FTPConn::unsetRawBufferCallback() {
  rawbuf->unsetCallback();
}

const std::list<std::string> & FTPConn::getXDUPEList() const {
  return xdupelist;
}
