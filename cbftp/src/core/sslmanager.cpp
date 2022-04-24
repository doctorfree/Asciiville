#include "sslmanager.h"

#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <list>
#include <map>
#include <mutex>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <vector>

#include <openssl/asn1.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/sslerr.h>
#endif
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include "util.h"
#include "logger.h"


namespace Core {

// These can be supplied as -D parameters while compiling to change default
// built-in certificate values.
#ifndef CERT_COUNTRY
#define CERT_COUNTRY "AQ"
#endif

#ifndef CERT_CORP
#define CERT_CORP "localhost"
#endif

#ifndef CERT_FQDN
#define CERT_FQDN "localhost"
#endif

namespace {

const int ELLIPTIC_CURVE = NID_X9_62_prime256v1;

std::recursive_mutex g_lock;

std::string SSLErrors() {
  std::string errstr;
  unsigned long err = 0;
  std::string previouserr;
  while ((err = ERR_get_error())) {
    errstr += (errstr.length() ? "," : "") + previouserr;
    previouserr = ERR_error_string(err, nullptr);
  }
  return errstr;
}

struct SSLContextInfo {
  SSLContextInfo() : ctx(nullptr), pkey(nullptr), cert(nullptr) {}
  SSLContextInfo(SSL_CTX* ctx, EVP_PKEY* pkey, X509* cert) : ctx(ctx), pkey(pkey) , cert(cert) {}
  SSL_CTX* ctx;
  EVP_PKEY* pkey;
  X509* cert;
};

bool g_initialized = false;
SSL_CTX* g_clientcontext;
std::list<SSLContextInfo> g_contexts;
std::map<std::string, SSLContextInfo&> g_contexthosts;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
std::vector<std::mutex> g_ssllocks;
void sslLockingCallback(int mode, int n, const char*, int) {
  if (mode & CRYPTO_LOCK) {
    g_ssllocks[n].lock();
  }
  else {
    g_ssllocks[n].unlock();
  }
}

pthread_t sslThreadIdCallback() {
  return pthread_self();
}
#endif

EVP_PKEY* createPrivateKey() {
  assert(g_initialized);
  EVP_PKEY* pkey = nullptr;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  EC_KEY* eckey = EC_KEY_new_by_curve_name(ELLIPTIC_CURVE);
  if (!eckey) {
    getLogger()->log("SSLManager", "Creating EC KEY failed: " + SSLErrors(), LogLevel::ERROR);
    return nullptr;
  }
  EC_KEY_set_asn1_flag(eckey, OPENSSL_EC_NAMED_CURVE);
  if (!EC_KEY_generate_key(eckey)) {
    getLogger()->log("SSLManager", "Generating EC KEY failed: " + SSLErrors(), LogLevel::ERROR);
    EC_KEY_free(eckey);
    return nullptr;
  }
  pkey = EVP_PKEY_new();
  if (!pkey) {
    getLogger()->log("SSLManager", "Creating EVP PKEY failed: " + SSLErrors(), LogLevel::ERROR);
    EVP_PKEY_free(pkey);
    EC_KEY_free(eckey);
    return nullptr;
  }
  if (!EVP_PKEY_assign_EC_KEY(pkey, eckey)) {
    getLogger()->log("SSLManager", "Assigning EC KEY to EVP KEY failed: " + SSLErrors(), LogLevel::ERROR);
    EVP_PKEY_free(pkey);
    EC_KEY_free(eckey);
    return nullptr;
  }
#else
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
  if (!ctx) {
    getLogger()->log("SSLManager", "EVP_PKEY_CTX_new_id failed: " + SSLErrors(), LogLevel::ERROR);
    return nullptr;
  }
  if (EVP_PKEY_paramgen_init(ctx) < 1) {
    getLogger()->log("SSLManager", "EVP_PKEY_paramgen_init failed: " + SSLErrors(), LogLevel::ERROR);
    EVP_PKEY_CTX_free(ctx);
  }
  if (EVP_PKEY_keygen_init(ctx) < 1) {
    getLogger()->log("SSLManager", "EVP_PKEY_keygen_init failed: " + SSLErrors(), LogLevel::ERROR);
    EVP_PKEY_CTX_free(ctx);
    return nullptr;
  }
  if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, ELLIPTIC_CURVE) < 1) {
    getLogger()->log("SSLManager", "EVP_PKEY_CTX_set_ec_paramgen_curve_nid failed: " + SSLErrors(), LogLevel::ERROR);
    EVP_PKEY_CTX_free(ctx);
    return nullptr;
  }
  if (EVP_PKEY_keygen(ctx, &pkey) < 1) {
    getLogger()->log("SSLManager", "EVP_PKEY_keygen failed: " + SSLErrors(), LogLevel::ERROR);
    EVP_PKEY_CTX_free(ctx);
    return nullptr;
  }
  EVP_PKEY_CTX_free(ctx);
#endif
  return pkey;
}

X509* createSelfSignedCertificate(EVP_PKEY* pkey) {
  assert(g_initialized);
  X509* x509 = X509_new();
  if (!x509) {
    getLogger()->log("SSLManager", "X509_new failed: " + SSLErrors(), LogLevel::ERROR);
    return nullptr;
  }
  ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);
  X509_NAME* name = X509_get_subject_name(x509);
  if (!name) {
    getLogger()->log("SSLManager", "X509_get_subject_name failed: " + SSLErrors(), LogLevel::ERROR);
    X509_free(x509);
    return nullptr;
  }
  if (!X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)CERT_COUNTRY, -1, -1, 0) ||
      !X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)CERT_CORP, -1, -1, 0) ||
      !X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)CERT_FQDN, -1, -1, 0))
  {
    getLogger()->log("SSLManager", "X509_NAME_add_entry_by_txt failed: " + SSLErrors(), LogLevel::ERROR);
    X509_free(x509);
    return nullptr;
  }
  if (!X509_set_issuer_name(x509, name)) {
    getLogger()->log("SSLManager", "X509_set_issuer_name failed: " + SSLErrors(), LogLevel::ERROR);
    X509_free(x509);
    return nullptr;
  }
  if (!X509_set_pubkey(x509, pkey)) {
    getLogger()->log("SSLManager", "X509_set_pubkey failed: " + SSLErrors(), LogLevel::ERROR);
    X509_free(x509);
    return nullptr;
  }
  if (!X509_sign(x509, pkey, EVP_sha256())) {
    getLogger()->log("SSLManager", "X509_sign failed: " + SSLErrors(), LogLevel::ERROR);
    X509_free(x509);
    return nullptr;
  }
  return x509;
}

std::string ip2Str(ASN1_OCTET_STRING* ip) {
  socklen_t addrlen = ip->length == 4 ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
  char buf[INET6_ADDRSTRLEN];
  if (!inet_ntop(ip->length == 4 ? AF_INET : AF_INET6, ip->data, buf, addrlen)) {
    getLogger()->log("SSLManager", "inet_ntop failed: " + util::getStrError(errno), LogLevel::ERROR);
    return "";
  }
  return buf;
}

std::list<std::string> getCertSubjectAltNames(X509* cert) {
  std::list<std::string> hostNames;
  STACK_OF(GENERAL_NAME)* sans = (STACK_OF(GENERAL_NAME)*)X509_get_ext_d2i(cert, NID_subject_alt_name, nullptr, nullptr);
  if (sans) {
    int numSANNames = sk_GENERAL_NAME_num(sans);
    for (int i = 0; i < numSANNames; i++) {
      const GENERAL_NAME* currentName = sk_GENERAL_NAME_value(sans, i);
      if (currentName->type == GEN_DNS) {
        unsigned char* host;
        int len = ASN1_STRING_to_UTF8(&host, currentName->d.dNSName);
        if (len >= 0) {
          if (len > 0) {
            hostNames.push_back((char*)host);
          }
          OPENSSL_free(host);
        }
      }
      else if (currentName->type == GEN_IPADD) {
        std::string ip = ip2Str(currentName->d.iPAddress);
        if (ip.length())
        {
            hostNames.push_back(ip);
        }
      }
    }
  }
  return hostNames;
}

std::string getCertCommonName(X509* cert) {
  X509_NAME* subjectName = X509_get_subject_name(cert);
  if (!subjectName) {
    getLogger()->log("SSLManager", "X509_get_subject_name failed.", LogLevel::ERROR);
    return "";
  }
  int idx = X509_NAME_get_index_by_NID(subjectName, NID_commonName, -1);
  X509_NAME_ENTRY* entry = X509_NAME_get_entry(subjectName, idx);
  if (entry) {
    ASN1_STRING* entryData = X509_NAME_ENTRY_get_data(entry);
    if (entryData) {
      unsigned char* host;
      int length = ASN1_STRING_to_UTF8(&host, entryData);
      if (length >= 0) {
        std::string hostString((char*)host);
        OPENSSL_free(host);
        return hostString;
      }
    }
  }
  return "";
}


std::list<std::string> getHostNamesFromCertificate(X509* cert) {
  std::list<std::string> names = getCertSubjectAltNames(cert);
  if (!names.size()) {
    std::string commonName = getCertCommonName(cert);
    if (commonName.length()) {
      names.push_back(commonName);
    }
  }
  return names;
}

} // namespace

int earlyCallback(SSL* ssl, int* alarm, void* arg) {
  const char* name = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  std::map<std::string, SSLContextInfo&>::const_iterator it;
  if (name != nullptr) {
    getLogger()->log("SSLManager", std::string("TLS SNI: ") + name, LogLevel::DEBUG);
    if ((it = g_contexthosts.find(name)) != g_contexthosts.end()) {
      getLogger()->log("SSLManager", std::string("Host string ") + name + " matches certificate string " + name +
                             ", switching context", LogLevel::DEBUG);
      if (!SSL_set_SSL_CTX(ssl, it->second.ctx)) {
        getLogger()->log("SSLManager", "SSL_set_SSL_CTX failed: " + SSLErrors(), LogLevel::ERROR);
        return 0;
      }
      return 1;
    }
    else {
      for (auto& host : g_contexthosts) {
        getLogger()->log("SSLManager", "Comparing cert string: " + host.first, LogLevel::DEBUG);
        if (util::wildcmp(host.first.c_str(), name)) {
          getLogger()->log("SSLManager", std::string("Host string ") + name + " matches certificate string " +
                                 host.first.c_str() + ", switching context", LogLevel::DEBUG);
          if (!SSL_set_SSL_CTX(ssl, host.second.ctx)) {
            getLogger()->log("SSLManager", "SSL_set_SSL_CTX failed: " + SSLErrors(), LogLevel::ERROR);
            return 0;
          }
          return 1;
        }
      }
    }
    getLogger()->log("SSLManager", "No certificate matches the requested host, using default cert.", LogLevel::DEBUG);
  }
  return 1;
}

SSL_CTX* createContext() {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  SSL_CTX* ctx = SSL_CTX_new(SSLv23_method());
#else
  SSL_CTX* ctx = SSL_CTX_new(TLS_method());
#endif
  if (!ctx) {
    getLogger()->log("SSLManager", "SSL_CTX_new failed: " + SSLErrors(), LogLevel::ERROR);
    return nullptr;
  }
/*  if (!SSL_CTX_set_cipher_list(ctx, "DEFAULT:!SEED")) {
    getLogger()->log("SSLManager", "SSL_CTX_set_cipher_list failed: " + SSLErrors(), LogLevel::ERROR);
    SSL_CTX_free(ctx);
    return nullptr;
  }*/
  return ctx;
}

SSL_CTX* createServerContext() {
  SSL_CTX* ctx = createContext();
  if (!ctx) {
    return nullptr;
  }
#if OPENSSL_VERSION_NUMBER < 0x10101000L
  SSL_CTX_set_tlsext_servername_callback(ctx, earlyCallback);
#else
  SSL_CTX_set_client_hello_cb(ctx, earlyCallback, nullptr);
#endif
#if OPENSSL_VERSION_NUMBER < 0x10002000L
  EC_KEY* eckey = EC_KEY_new_by_curve_name(ELLIPTIC_CURVE);
  if (!eckey) {
    getLogger()->log("SSLManager", "Creating EC KEY failed: " + SSLErrors(), LogLevel::ERROR);
    return ctx;
  }
  SSL_CTX_set_tmp_ecdh(ctx, eckey);
  EC_KEY_free(eckey);
#elif OPENSSL_VERSION_NUMBER < 0x10100000L
  SSL_CTX_set_ecdh_auto(ctx, 1);
#endif
  return ctx;
}

void SSLManager::init() {
  std::lock_guard<std::recursive_mutex> lock(g_lock);
  if (g_initialized) {
    return;
  }
  g_initialized = true;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  g_ssllocks = std::vector<std::mutex>(CRYPTO_num_locks());
  CRYPTO_set_locking_callback(sslLockingCallback);
  CRYPTO_set_id_callback(sslThreadIdCallback);
  SSL_library_init();
  SSL_load_error_strings();
#endif
  g_clientcontext = createContext();
}

void SSLManager::cleanupThread() {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
  OPENSSL_thread_stop();
#endif
}

SSL_CTX* SSLManager::getClientSSLCTX() {
  std::lock_guard<std::recursive_mutex> lock(g_lock);
  assert(g_initialized);
  return g_clientcontext;
}

SSL_CTX* SSLManager::getServerSSLCTX()
{
  std::lock_guard<std::recursive_mutex> lock(g_lock);
  assert(g_initialized);
  if (!g_contexts.size()) {
    getLogger()->log("SSLManager", "Creating private key and self-signed certificate...", LogLevel::INFO);
    EVP_PKEY* pkey = createPrivateKey();
    X509* x509 = createSelfSignedCertificate(pkey);
    if (!SSLManager::addCertKeyPair(pkey, x509)) {
      return nullptr;
    }
  }
  return g_contexts.front().ctx;
}

std::list<std::pair<BinaryData, BinaryData>> SSLManager::certKeyPairs() {
  std::lock_guard<std::recursive_mutex> lock(g_lock);
  std::list<std::pair<BinaryData, BinaryData>> pairs;
  for (auto& context : g_contexts) {
    BinaryData keydata, certdata;
    int bufsize = i2d_PrivateKey(context.pkey, nullptr);
    if (bufsize < 0) {
      getLogger()->log("SSLManager", "i2d_PrivateKey failed: " + SSLErrors(), LogLevel::ERROR);
      continue;
    }
    keydata.resize(bufsize);
    unsigned char* buf = &keydata[0];
    if (i2d_PrivateKey(context.pkey, &buf) < 0) {
      getLogger()->log("SSLManager", "i2d_PrivateKey failed: " + SSLErrors(), LogLevel::ERROR);
      continue;
    }
    bufsize = i2d_X509(context.cert, nullptr);
    if (bufsize < 0) {
      getLogger()->log("SSLManager", "i2d_X509 failed: " + SSLErrors(), LogLevel::ERROR);
      continue;
    }
    certdata.resize(bufsize);
    buf = &certdata[0];
    if (i2d_X509(context.cert, &buf) < 0) {
      getLogger()->log("SSLManager", "i2d_X509 failed: " + SSLErrors(), LogLevel::ERROR);
      continue;
    }
    pairs.emplace_back(keydata, certdata);
  }
  return pairs;
}

bool SSLManager::addCertKeyPair(const BinaryData& key, const BinaryData& cert) {
  std::lock_guard<std::recursive_mutex> lock(g_lock);
  assert(g_initialized);
  const unsigned char* bufStart = &cert[0];
  X509* x509 = d2i_X509(nullptr, &bufStart, cert.size());
  if (!x509) { // PEM format?
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
      getLogger()->log("SSLManager", "BIO_new failed.", LogLevel::ERROR);
      return false;
    }
    if (!(BIO_write(bio, &cert[0], cert.size()) > 0)) {
      getLogger()->log("SSLManager", "BIO_write failed.", LogLevel::ERROR);
      BIO_free(bio);
      return false;
    }
    x509 = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!x509) {
      getLogger()->log("SSLManager", "Error while parsing certificate: " + SSLErrors(), LogLevel::ERROR);
      return false;
    }
  }
  bufStart = &key[0];
  EVP_PKEY* pkey = d2i_AutoPrivateKey(nullptr, &bufStart, key.size());
  if (!pkey) { // PEM format?
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
      getLogger()->log("SSLManager", "BIO_new failed.", LogLevel::ERROR);
      return false;
    }
    if (!(BIO_write(bio, &key[0], key.size()) > 0)) {
      getLogger()->log("SSLManager", "BIO_write failed.", LogLevel::ERROR);
      BIO_free(bio);
      return false;
    }
    pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) {
      getLogger()->log("SSLManager", "Error while parsing private key: " + SSLErrors(), LogLevel::ERROR);
      return false;
    }
  }
  return addCertKeyPair(pkey, x509);
}

bool SSLManager::addCertKeyPair(EVP_PKEY* pkey, X509* x509) {
  std::lock_guard<std::recursive_mutex> lock(g_lock);
  assert(g_initialized);
  if (x509 != nullptr && pkey != nullptr) {
    SSL_CTX* ctx = createServerContext();
    if (!ctx) {
      return false;
    }
    if (SSL_CTX_use_certificate(ctx, x509) != 1) {
      getLogger()->log("SSLManager", "Error while applying certificate: " + SSLErrors(), LogLevel::ERROR);
      SSL_CTX_free(ctx);
      return false;
    }
    if (SSL_CTX_use_PrivateKey(ctx, pkey) != 1) {
      getLogger()->log("SSLManager", "Error while applying private key: " + SSLErrors(), LogLevel::ERROR);
      SSL_CTX_free(ctx);
      return false;
    }
    std::list<std::string> hostNames = getHostNamesFromCertificate(x509);
    g_contexts.push_back(SSLContextInfo(ctx, pkey, x509));
    for (auto& hostName : hostNames) {
      std::pair<std::string, SSLContextInfo&> pair(hostName, g_contexts.back());
      g_contexthosts.insert(pair);
    }
    return true;
  }
  return false;
}

void SSLManager::clearCertKeyPairs() {
  std::lock_guard<std::recursive_mutex> lock(g_lock);
  for (auto& context : g_contexts) {
    SSL_CTX_free(context.ctx);
    X509_free(context.cert);
    EVP_PKEY_free(context.pkey);
  }
  g_contexthosts.clear();
  g_contexts.clear();
}

const char* SSLManager::getCipher(SSL* ssl) {
  return SSL_CIPHER_get_name(SSL_get_current_cipher(ssl));
}

std::string SSLManager::version() {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  return SSLeay_version(SSLEAY_VERSION);
#else
  return OpenSSL_version(OPENSSL_VERSION);
#endif
}

std::string SSLManager::sslErrorToString(int error) {
  switch (error) {
    case SSL_ERROR_NONE:
      return "SSL_ERROR_NONE";
    case SSL_ERROR_SSL:
      return "SSL_ERROR_SSL";
    case SSL_ERROR_WANT_READ:
      return "SSL_ERROR_WANT_READ";
    case SSL_ERROR_WANT_WRITE:
      return "SSL_ERROR_WANT_WRITE";
    case SSL_ERROR_WANT_X509_LOOKUP:
      return "SSL_ERROR_WANT_X509_LOOKUP";
    case SSL_ERROR_SYSCALL:
      return "SSL_ERROR_SYSCALL";
    case SSL_ERROR_ZERO_RETURN:
      return "SSL_ERROR_ZERO_RETURN";
    case SSL_ERROR_WANT_CONNECT:
      return "SSL_ERROR_WANT_CONNECT";
    case SSL_ERROR_WANT_ACCEPT:
      return "SSL_ERROR_WANT_ACCEPT";
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
    case SSL_ERROR_WANT_ASYNC:
      return "SSL_ERROR_WANT_ASYNC";
    case SSL_ERROR_WANT_ASYNC_JOB:
      return "SSL_ERROR_WANT_ASYNC_JOB";
#endif
#if OPENSSL_VERSION_NUMBER >= 0x10101000L
    case SSL_ERROR_WANT_CLIENT_HELLO_CB:
      return "SSL_ERROR_WANT_CLIENT_HELLO_CB";
#endif
  }
  return std::to_string(error);
}

bool SSLManager::isAbruptDisconnectError(unsigned long e) {
#if OPENSSL_VERSION_NUMBER < 0x30000000L
#else
  if ((e & ERR_REASON_MASK) == SSL_R_UNEXPECTED_EOF_WHILE_READING) {
    return true;
  }
#endif
  return false;
}

} // namespace Core
