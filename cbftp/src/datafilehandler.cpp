#include "datafilehandler.h"

#include <unistd.h>
#include <cstdlib>
#include <cstdio>

#include "crypto.h"
#include "filesystem.h"
#include "datafilehandlermethod.h"
#include "eventlog.h"
#include "globalcontext.h"
#include "path.h"
#include "util.h"

DataFileHandler::DataFileHandler() : state(DataFileState::NOT_EXISTING) {
  datafile = DataFileHandlerMethod::getDataFile();
  if (FileSystem::directoryExists(datafile.dirName())) {
    if (!FileSystem::directoryExistsReadable(datafile.dirName())) {
      std::string errorstring = "Error: no read access to " + datafile.dirName();
      perror(errorstring.c_str());
      exit(1);
    }
    if (!FileSystem::directoryExistsWritable(datafile.dirName())) {
      std::string errorstring = "Error: no write access to " + datafile.dirName();
      perror(errorstring.c_str());
      exit(1);
    }
  }
  else {
    util::Result res = FileSystem::createDirectory(datafile.dirName(), true);
    if (!res.success) {
      perror(std::string("Error: could not create " + datafile.dirName() + ": " + res.error).c_str());
      exit(1);
    }
  }
  global->getEventLog()->log("DataFileHandler", "Using data file: " + datafile.toString());
  if (!FileSystem::fileExists(datafile)) {
    return;
  }
  if (!FileSystem::fileExistsReadable(datafile)) {
    perror(std::string("There was an error accessing " + datafile.toString()).c_str());
    exit(1);
  }
  if (!FileSystem::fileExistsWritable(datafile)) {
    perror(std::string("There was an error accessing " + datafile.toString()).c_str());
    exit(1);
  }
  util::Result res = FileSystem::readFile(datafile, rawdata);
  if (!res.success) {
    perror(std::string("Error: failed to read " + datafile.toString() + ": " + res.error).c_str());
    exit(1);
  }
  if (!Crypto::isMostlyASCII(rawdata)) {
    state = DataFileState::EXISTS_ENCRYPTED;
    return;
  }
  state = DataFileState::EXISTS_PLAIN;
  parseDecryptedFile(rawdata);
}

Path DataFileHandler::getDataDir() {
  return DataFileHandlerMethod::getDataFile().dirName();
}

DataFileState DataFileHandler::getState() const {
  return state;
}

bool DataFileHandler::tryDecrypt(const std::string& key) {
  if (state != DataFileState::EXISTS_ENCRYPTED) {
    return false;
  }
  this->key = key;
  Core::BinaryData keydata(key.begin(), key.end());
  Core::BinaryData decryptedtext;
  if (!DataFileHandlerMethod::decrypt(rawdata, keydata, decryptedtext)) {
    return false;
  }
  state = DataFileState::EXISTS_DECRYPTED;
  parseDecryptedFile(decryptedtext);
  return true;
}

void DataFileHandler::parseDecryptedFile(const Core::BinaryData& data) {
  int lastbreakpos = 0;
  for (unsigned int currentpos = 0; currentpos <= data.size(); currentpos++) {
    if (data[currentpos] == '\n' || currentpos == data.size()) {
      decryptedlines.push_back(std::string((const char *)&data[lastbreakpos], currentpos - lastbreakpos));
      lastbreakpos = currentpos + 1;
    }
  }
  Crypto::sha256(data, filehash);
}

bool DataFileHandler::setEncrypted(const std::string& key) {
  if (state == DataFileState::EXISTS_ENCRYPTED || state == DataFileState::EXISTS_DECRYPTED) {
    return false;
  }
  this->key = key;
  state = DataFileState::EXISTS_DECRYPTED;
  filehash.clear();
  writeFile();
  return true;
}

bool DataFileHandler::setPlain(const std::string& key) {
  if (state != DataFileState::EXISTS_DECRYPTED || this->key != key) {
    return false;
  }
  state = DataFileState::EXISTS_PLAIN;
  filehash.clear();
  writeFile();
  return true;
}

void DataFileHandler::writeFile() {
  if (state == DataFileState::EXISTS_ENCRYPTED) {
    return;
  }
  std::string fileoutput = "";
  std::vector<std::string>::iterator it;
  for (it = outputlines.begin(); it != outputlines.end(); it++) {
    fileoutput.append(*it + "\n");
  }
  Core::BinaryData datahash;
  Core::BinaryData fileoutputdata(fileoutput.begin(),
                            fileoutput.end() - (fileoutput.size() ? 1 : 0));
  Crypto::sha256(fileoutputdata, datahash);
  if (datahash == filehash) {
    return;
  }
  filehash = datahash;
  Path tmpdatafile = datafile.toString() + ".tmp";
  util::Result res;
  if (state == DataFileState::EXISTS_DECRYPTED) {
    Core::BinaryData ciphertext;
    Core::BinaryData keydata(key.begin(), key.end());
    DataFileHandlerMethod::encrypt(fileoutputdata, keydata, ciphertext);
    res = FileSystem::writeFile(tmpdatafile, ciphertext);
  }
  else {
    if (state == DataFileState::NOT_EXISTING) {
      state = DataFileState::EXISTS_PLAIN;
    }
    res = FileSystem::writeFile(tmpdatafile, fileoutputdata);

  }
  if (!res.success) {
    global->getEventLog()->log("DataFileHandler", "Error writing " + tmpdatafile.toString() + ": " + res.error);
    return;
  }
  res = FileSystem::move(tmpdatafile, datafile);
  if (!res.success) {
    global->getEventLog()->log("DataFileHandler", "Error moving " + tmpdatafile.toString() + " -> " + datafile.toString() + ": " + res.error);
    return;
  }
}

bool DataFileHandler::changeKey(const std::string& key, const std::string& newkey) {
  if (state != DataFileState::EXISTS_DECRYPTED || this->key != key) {
    return false;
  }
  this->key = newkey;
  filehash.clear();
  writeFile();
  return true;
}

void DataFileHandler::clearOutputData() {
  outputlines.clear();
}

void DataFileHandler::addOutputLine(const std::string& owner, const std::string& line) {
  outputlines.push_back(owner + "." + line);
}

void DataFileHandler::getDataFor(const std::string& owner, std::vector<std::string>* matches) {
  matches->clear();
  std::vector<std::string>::iterator it;
  int len = owner.length() + 1;
  for (it = decryptedlines.begin(); it != decryptedlines.end(); it++) {
    if (it->compare(0, len, owner + ".") == 0) {
      matches->push_back(it->substr(len));
    }
  }
}
