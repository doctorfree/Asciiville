#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>

#include "datafiletoolfuncs.h"
#include "../datafilehandlermethod.h"
#include "../filesystem.h"
#include "../path.h"

int usage() {
  std::cerr << "datafilewrite: encrypts the content of a cbftp plain text "
            << "data file.\n\n"
            << "Usage: datafilewrite <infile> <--outfile= OR --defaultoutfile>"
            << std::endl;
  return 0;
}

int main(int argc, char ** argv) {
  if (argc != 3) {
    return usage();
  }
  const std::string path = argv[1];
  Path outpath = DataFileHandlerMethod::getDataFile();
  bool outfileset = false;
  std::string outfile;
  for (int i = 2; i < argc; i++) {
    if (!strncmp(argv[i], "--outfile=", 10)) {
      outfileset = true;
      outpath = argv[i] + 10;
    }
    else if (!strcmp(argv[i], "--defaultoutfile")) {
      outfileset = true;
    }
    else {
      return usage();
    }
  }
  if (!outfileset) {
    return usage();
  }

  if (!checkInputFile(path)) return -1;
  std::cerr << "Using data file: " << outpath.toString() << std::endl;
  Core::BinaryData key = getPassphrase();

  Core::BinaryData rawdata;
  FileSystem::readFile(path, rawdata);

  Core::BinaryData ciphertext;
  if (!DataFileHandlerMethod::encrypt(rawdata, key, ciphertext)) {
    std::cerr << "Error: The input file is not a valid cbftp data file."
              << std::endl;
    return -1;
  }

  FileSystem::writeFile(outpath, ciphertext);
  return 0;
}
