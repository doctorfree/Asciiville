#include "crypto.h"

#include <sstream>
#include <climits>
#include <ctime>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/evp.h>

#define SALT_LENGTH 8
#define SALT_STRING_LENGTH 16
#define DEFAULT_ITER 10000

namespace {

const EVP_CIPHER * cipher() {
  return EVP_aes_256_cbc();
}

const EVP_MD * digest() {
  return EVP_sha256();
}

int blockSize() {
  return EVP_CIPHER_block_size(cipher());
}

unsigned char hex2uc(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (tolower(c) >= 'a' && tolower(c) <= 'f') {
    return c - 'W';
  }
  return 0;
}

std::string uc2hex(unsigned char c) {
  std::string out;
  char major = c / 16;
  out += major >= 10 ? major + 'W' : major + '0';
  char minor = c % 16;
  out += minor >= 10 ? minor + 'W' : minor + '0';
  return out;
}

}

void Crypto::encrypt(const Core::BinaryData & indata, const Core::BinaryData & pass, Core::BinaryData & outdata) {
  if (indata.empty()) {
    outdata.resize(0);
    return;
  }
  EVP_CIPHER_CTX * ctx = EVP_CIPHER_CTX_new();
  const EVP_CIPHER * cipherp = cipher();

  int keylen = EVP_CIPHER_key_length(cipherp);
  unsigned char tmpkeyiv[EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH];
  unsigned char* key = tmpkeyiv;
  unsigned char* iv = tmpkeyiv + keylen;
  outdata.resize(SALT_STRING_LENGTH + indata.size() + blockSize());
  memcpy(&outdata[0], "Salted__", SALT_STRING_LENGTH - SALT_LENGTH);
  srand(time(NULL));
  for (int i = SALT_LENGTH; i < SALT_STRING_LENGTH; ++i) {
    outdata[i] = (unsigned char)(rand() % UCHAR_MAX);
  }
#if OPENSSL_VERSION_NUMBER < 0x10000000L
  EVP_BytesToKey(cipherp, digest(), &outdata[SALT_STRING_LENGTH - SALT_LENGTH], !pass.empty() ? pass.data() : NULL,
                 pass.size(), 1, key, iv);
#else
  int ivlen = EVP_CIPHER_iv_length(cipherp);
  PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(pass.data()), pass.size(),
                    &outdata[SALT_STRING_LENGTH - SALT_LENGTH], SALT_LENGTH,
                    DEFAULT_ITER, digest(), keylen + ivlen, tmpkeyiv);
#endif
  int resultlen;
  int finallen;
  EVP_EncryptInit_ex(ctx, cipherp, NULL, key, iv);
  EVP_EncryptUpdate(ctx, &outdata[SALT_STRING_LENGTH], &resultlen, &indata[0], indata.size());
  EVP_EncryptFinal_ex(ctx, &outdata[SALT_STRING_LENGTH + resultlen], &finallen);
  outdata.resize(SALT_STRING_LENGTH + resultlen + finallen);
  EVP_CIPHER_CTX_free(ctx);
}

void Crypto::decrypt(const Core::BinaryData & indata, const Core::BinaryData & pass, Core::BinaryData & outdata) {
  if (indata.empty()) {
    outdata.resize(0);
    return;
  }
  EVP_CIPHER_CTX * ctx = EVP_CIPHER_CTX_new();
  const EVP_CIPHER * cipherp = cipher();

  int keylen = EVP_CIPHER_key_length(cipherp);
  unsigned char tmpkeyiv[EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH];
  unsigned char* key = tmpkeyiv;
  unsigned char* iv = tmpkeyiv + keylen;
#if OPENSSL_VERSION_NUMBER < 0x10000000L
  EVP_BytesToKey(cipherp, digest(), &indata[SALT_STRING_LENGTH - SALT_LENGTH], !pass.empty() ? pass.data() : NULL,
                 pass.size(), 1, key, iv);
#else
  int ivlen = EVP_CIPHER_iv_length(cipherp);
  PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(pass.data()), pass.size(),
                    &indata[SALT_STRING_LENGTH - SALT_LENGTH], SALT_LENGTH,
                    DEFAULT_ITER, digest(), keylen + ivlen, tmpkeyiv);
#endif
  outdata.resize(indata.size() + blockSize());
  int writelen;
  int finalwritelen;
  EVP_DecryptInit_ex(ctx, cipherp, NULL, key, iv);
  EVP_DecryptUpdate(ctx, &outdata[0], &writelen, &indata[SALT_STRING_LENGTH],
                    indata.size() - SALT_STRING_LENGTH);
  EVP_DecryptFinal_ex(ctx, &outdata[writelen], &finalwritelen);
  outdata.resize(writelen + finalwritelen);
  EVP_CIPHER_CTX_free(ctx);
}

void Crypto::decryptOld(const Core::BinaryData & indata, const Core::BinaryData & pass, Core::BinaryData & outdata) {
  if (indata.empty()) {
    outdata.resize(0);
    return;
  }
  EVP_CIPHER_CTX * ctx = EVP_CIPHER_CTX_new();
  const EVP_CIPHER * cipherp = cipher();
  int keylen = EVP_CIPHER_key_length(cipherp);
  unsigned char tmpkeyiv[EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH];
  unsigned char* key = tmpkeyiv;
  unsigned char* iv = tmpkeyiv + keylen;
  EVP_BytesToKey(cipherp, digest(), &indata[SALT_STRING_LENGTH - SALT_LENGTH], !pass.empty() ? pass.data() : NULL,
                 pass.size(), 1, key, iv);
  outdata.resize(indata.size() + blockSize());
  int writelen;
  int finalwritelen;
  EVP_DecryptInit_ex(ctx, cipherp, NULL, key, iv);
  EVP_DecryptUpdate(ctx, &outdata[0], &writelen, &indata[SALT_STRING_LENGTH],
                    indata.size() - SALT_STRING_LENGTH);
  EVP_DecryptFinal_ex(ctx, &outdata[writelen], &finalwritelen);
  outdata.resize(writelen + finalwritelen);
  EVP_CIPHER_CTX_free(ctx);
}

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#define EVP_MD_CTX_new EVP_MD_CTX_create
#define EVP_MD_CTX_free EVP_MD_CTX_destroy
#endif

void Crypto::sha256(const Core::BinaryData & indata, Core::BinaryData & outdata) {
  if (indata.empty()) {
    outdata.resize(0);
    return;
  }
  outdata.resize(EVP_MD_size(digest()));
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  EVP_DigestInit(ctx, digest());
  EVP_DigestUpdate(ctx, &indata[0], indata.size());
  EVP_DigestFinal(ctx, &outdata[0], nullptr);
  EVP_MD_CTX_free(ctx);
}

void Crypto::base64Encode(const Core::BinaryData & indata, Core::BinaryData & outdata) {
  if (indata.empty()) {
    outdata.resize(0);
    return;
  }
  outdata.resize((indata.size() / 3 + ((indata.size() % 3) ? 1 : 0)) * 4 + 1);
  int bytes = EVP_EncodeBlock(&outdata[0], &indata[0], indata.size());
  outdata.resize(bytes);
}

void Crypto::base64Decode(const Core::BinaryData & indata, Core::BinaryData & outdata) {
  if (indata.empty()) {
    outdata.resize(0);
    return;
  }
  int pos = indata.size();
  int padding = 0;
  while (--pos >= 0 && indata[pos] == '=') {
    ++padding;
  }
  unsigned int outsize = indata.size() / 4 * 3;
  outdata.resize(outsize);
  EVP_DecodeBlock(&outdata[0], &indata[0], indata.size());
  outdata.resize(outsize - padding);
}

bool Crypto::isMostlyASCII(const Core::BinaryData& data) {
  unsigned int asciicount = 0;
  for (unsigned int i = 0; i < data.size(); ++i) {
    if (data[i] < 128) {
      ++asciicount;
    }
  }
  return asciicount > data.size() * 0.9;
}

std::string Crypto::toHex(const Core::BinaryData& indata) {
  std::stringstream sstream;
  for (size_t i = 0; i < indata.size(); ++i) {
    sstream << uc2hex(indata[i]);
  }
  return sstream.str();
}

void Crypto::fromHex(const std::string& indata, Core::BinaryData& outdata) {
  outdata.resize(indata.size() / 2);
  for (size_t i = 0; i < outdata.size(); ++i) {
    outdata[i] = hex2uc(indata[i * 2]) * 16 + hex2uc(indata[i * 2 + 1]);
  }
}


