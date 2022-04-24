#pragma once

#include <termios.h>
#include <unistd.h>
#include <string>
#include <iostream>

#include "../core/types.h"
#include "../filesystem.h"

void showInput(bool show) {
  termios tty;
  tcgetattr(STDIN_FILENO, &tty);
  if (show) {
    tty.c_lflag |= ECHO;
  }
  else {
    tty.c_lflag &= ~ECHO;
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

Core::BinaryData getPassphrase() {
  std::cerr << "Passphrase: ";
  showInput(false);
  std::string passphrase;
  std::getline(std::cin, passphrase);
  std::cerr << std::endl;
  showInput(true);
  return Core::BinaryData(passphrase.begin(), passphrase.end());
}

bool checkInputFile(const Path & path) {
  if (!FileSystem::fileExists(path)) {
    std::cerr << "Error: The input file does not exist." << std::endl;
    return false;
  }
  if (!FileSystem::fileExistsReadable(path)) {
    std::cerr << "Error: Could not read the input file." << std::endl;
    return false;
  }
  return true;
}
