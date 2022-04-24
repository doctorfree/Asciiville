#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>

#include "datafiletoolfuncs.h"
#include "../crypto.h"
#include "../datafilehandlermethod.h"
#include "../filesystem.h"
#include "../path.h"

int usage() {
  std::cerr << "datafilecat: decrypts the content of a cbftp data file.\n\n"
            << "Usage: datafilecat [--infile=] [--outfile=]"
            << std::endl;
  return 0;
}

int main(int argc, char ** argv) {
  Path path = DataFileHandlerMethod::getDataFile();
  bool useoutfile = false;
  std::string outfile;
  for (int i = 1; i < argc; i++) {
    if (!strncmp(argv[i], "--infile=", 9)) {
      path = argv[i] + 9;
    }
    else if (!strncmp(argv[i], "--outfile=", 10)) {
      useoutfile = true;
      outfile = argv[i] + 10;
    }
    else {
      return usage();
    }
  }


  if (!checkInputFile(path)) return -1;
  std::cerr << "Using data file: " << path.toString() << std::endl;


  Core::BinaryData rawdata;
  FileSystem::readFile(path, rawdata);

  Core::BinaryData decryptedtext;

  if (!Crypto::isMostlyASCII(rawdata)) {
    Core::BinaryData key = getPassphrase();
    if (!DataFileHandlerMethod::decrypt(rawdata, key, decryptedtext)) {
      std::cerr << "Error: Either the passphrase is wrong, or the input file is"
                << " not a valid cbftp data file."
                << std::endl;
      return -1;
    }
  }
  else {
    decryptedtext = rawdata;
  }

  if (useoutfile) {
    FileSystem::writeFile(outfile, decryptedtext);
  }
  else {
    std::cout << std::string((const char *)&decryptedtext[0],
                             decryptedtext.size());
    std::cerr << std::endl;
  }
  return 0;
}
