#include "globalcontext.h"

GlobalContext* global = new GlobalContext();

void GlobalContext::linkCore(Core::WorkManager* wm, Core::TickPoke* tp,
                             Core::IOManager* iom,
                             std::shared_ptr<EventLog>& el)
{
  this->wm = wm;
  this->tp = tp;
  this->iom = iom;
  this->el = el;
}

void GlobalContext::linkComponents(SettingsLoaderSaver* sls, Engine* e,
    UIBase* uib, SiteManager* sm, SiteLogicManager* slm,
    TransferManager* tm, RemoteCommandHandler* rch,
    SkipList* sl, ProxyManager* pm, LocalStorage* ls,
    TimeReference* tr, Statistics* s,
    SectionManager* secm, HTTPServer* httpsrv, RestApi* ra,
    LoadMonitor* lm, ExternalScriptsManager* esm, SubProcessManager* spm,
    LogManager* logm)
{
  this->sls = sls;
  this->e = e;
  this->uib = uib;
  this->sm = sm;
  this->slm = slm;
  this->tm = tm;
  this->rch = rch;
  this->sl = sl;
  this->pm = pm;
  this->ls = ls;
  this->tr = tr;
  this->s = s;
  this->secm = secm;
  this->httpsrv = httpsrv;
  this->ra = ra;
  this->lm = lm;
  this->esm = esm;
  this->spm = spm;
  this->logm = logm;
}

Engine* GlobalContext::getEngine() const {
  return e;
}

SettingsLoaderSaver* GlobalContext::getSettingsLoaderSaver() const {
  return sls;
}

Core::IOManager* GlobalContext::getIOManager() const {
  return iom;
}

Core::WorkManager* GlobalContext::getWorkManager() const {
  return wm;
}

UIBase* GlobalContext::getUIBase() const {
  return uib;
}

SiteManager* GlobalContext::getSiteManager() const {
  return sm;
}

SiteLogicManager* GlobalContext::getSiteLogicManager() const {
  return slm;
}

TransferManager* GlobalContext::getTransferManager() const {
  return tm;
}

Core::TickPoke* GlobalContext::getTickPoke() const {
  return tp;
}

RemoteCommandHandler* GlobalContext::getRemoteCommandHandler() const {
  return rch;
}

SkipList* GlobalContext::getSkipList() const {
  return sl;
}

std::shared_ptr<EventLog>& GlobalContext::getEventLog() {
  return el;
}

ProxyManager* GlobalContext::getProxyManager() const {
  return pm;
}

LocalStorage* GlobalContext::getLocalStorage() const {
  return ls;
}

TimeReference* GlobalContext::getTimeReference() const {
  return tr;
}

Statistics* GlobalContext::getStatistics() const {
  return s;
}

SectionManager* GlobalContext::getSectionManager() const {
  return secm;
}

HTTPServer* GlobalContext::getHTTPServer() const {
  return httpsrv;
}

RestApi* GlobalContext::getRestApi() const {
  return ra;
}

LoadMonitor* GlobalContext::getLoadMonitor() const {
  return lm;
}

ExternalScriptsManager* GlobalContext::getExternalScriptsManager() const {
  return esm;
}

SubProcessManager* GlobalContext::getSubProcessManager() const {
  return spm;
}

LogManager* GlobalContext::getLogManager() const {
  return logm;
}

