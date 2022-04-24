#pragma once

#include <string>
#include <vector>

#include "core/types.h"
#include "path.h"

enum class DataFileState {
  NOT_EXISTING,
  EXISTS_PLAIN,
  EXISTS_ENCRYPTED,
  EXISTS_DECRYPTED
};

class DataFileHandler {
private:
  void parseDecryptedFile(const Core::BinaryData& data);
  Core::BinaryData rawdata;
  std::vector<std::string> decryptedlines;
  std::vector<std::string> outputlines;
  Path datafile;
  std::string key;
  Core::BinaryData filehash;
  DataFileState state;
public:
  DataFileHandler();
  DataFileState getState() const;
  bool tryDecrypt(const std::string& key);
  void writeFile();
  bool changeKey(const std::string& key, const std::string& newkey);
  bool setEncrypted(const std::string& key);
  bool setPlain(const std::string& key);
  void clearOutputData();
  void addOutputLine(const std::string& owner, const std::string& line);
  void getDataFor(const std::string& owner, std::vector<std::string>* matches);
  static Path getDataDir();
};
