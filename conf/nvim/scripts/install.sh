#!/bin/bash
#
# Copyright (C) 2022 Michael Peter <michaeljohannpeter@gmail.com>
# Copyright (C) 2023 Ronald Record <ronaldrecord@gmail.com>
#
# Install Neovim and all dependencies for the Neovim config at:
#     https://github.com/doctorfree/Asciiville/conf/nvim/init.vim
#
# Adapted for Asciiville from https://github.com/Allaman/nvim.git
# See https://github.com/doctorfree/nvim

set -e

OS=""
PYTHON=
LINUX_DISTRIBUTION=""
LINUX_HOMEBREW="https://docs.brew.sh/Homebrew-on-Linux"
DOC_HOMEBREW="https://docs.brew.sh"
BREW_EXE="brew"

abort () {
  printf "ERROR: %s\n" "$@" >&2
  exit 1
}

log () {
  [ "${quiet}" ] || {
    printf "################################################################\n"
    printf "%s\n" "$@"
  }
}

check_prerequisites () {
  if [ -z "${BASH_VERSION:-}" ]; then
    abort "Bash is required to interpret this script."
  fi

  if [[ $EUID -eq 0 ]]; then
    abort "Script must not be run as root user"
  fi

  command -v sudo > /dev/null 2>&1 || { abort "sudo not found - please install"; }

  arch=$(uname -m)
  if [[ $arch =~ "arm" || $arch =~ "aarch64" ]]; then
    abort "Only amd64/x86_64 is supported"
  fi
}

get_os () {
  if [[ "$OSTYPE" =~ "darwin"* ]]; then
    OS="apple"
  elif [[ "$OSTYPE" =~ "linux" ]]; then
    OS="linux"
    log "Running on Linux"
  fi
}

get_linux_distribution () {
  local release
  release=$(cat /etc/*-release)
  if [[ "$release" =~ "Debian" ]]; then
    LINUX_DISTRIBUTION="debian"
  elif [[ "$release" =~ "Ubuntu" ]]; then
    LINUX_DISTRIBUTION="ubuntu"
  elif [[ "$release" =~ "Arch" ]]; then
    LINUX_DISTRIBUTION="arch"
  elif [ -f /etc/os-release ]
  then
    . /etc/os-release
    [ "${ID}" == "debian" ] || [ "${ID_LIKE}" == "debian" ] && {
      LINUX_DISTRIBUTION="debian"
      debian=1
    }
    [ "${ID}" == "arch" ] || [ "${ID_LIKE}" == "arch" ] && {
      LINUX_DISTRIBUTION="arch"
      arch=1
    }
    [ "${ID}" == "fedora" ] && {
      LINUX_DISTRIBUTION="fedora"
      fedora=1
    }
    [ "${arch}" ] || [ "${debian}" ] || [ "${fedora}" ] || {
      echo "${ID_LIKE}" | grep debian > /dev/null && debian=1
    }
  else
    if [ -f /etc/arch-release ]
    then
      LINUX_DISTRIBUTION="arch"
    else
      have_apt=`type -p apt`
      if [ "${have_apt}" ]
      then
        LINUX_DISTRIBUTION="debian"
      else
        if [ -f /etc/fedora-release ]
        then
          LINUX_DISTRIBUTION="fedora"
        else
          have_dnf=`type -p dnf`
          if [ "${have_dnf}" ]
          then
            LINUX_DISTRIBUTION="fedora"
          else
            abort "Unknown operating system distribution"
          fi
        fi
      fi
    fi
  fi
}

install_brew () {
  if ! command -v brew >/dev/null 2>&1; then
    log "Installing Homebrew, please be patient"
    BREW_URL="https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh"
    have_curl=`type -p curl`
    [ "${have_curl}" ] || abort "The curl command could not be located."
    curl -fsSL "${BREW_URL}" > /tmp/brew-$$.sh
    [ $? -eq 0 ] || {
      rm -f /tmp/brew-$$.sh
      curl -kfsSL "${BREW_URL}" > /tmp/brew-$$.sh
    }
    [ -f /tmp/brew-$$.sh ] || abort "Brew install script download failed"
    chmod 755 /tmp/brew-$$.sh
    NONINTERACTIVE=1 /bin/bash -c "/tmp/brew-$$.sh" > /dev/null 2>&1
    rm -f /tmp/brew-$$.sh
    export HOMEBREW_NO_INSTALL_CLEANUP=1
    export HOMEBREW_NO_ENV_HINTS=1
    if [ -f ${HOME}/.profile ]
    then
      BASHINIT="${HOME}/.profile"
    else
      if [ -f ${HOME}/.bashrc ]
      then
        BASHINIT="${HOME}/.bashrc"
      else
        BASHINIT="${HOME}/.profile"
      fi
    fi
    # shellcheck disable=SC2016
    if [ -x /home/linuxbrew/.linuxbrew/bin/brew ]
    then
      BREW_EXE="/home/linuxbrew/.linuxbrew/bin/brew"
      HOMEBREW_HOME="/home/linuxbrew/"
    else
      if [ -x /usr/local/bin/brew ]
      then
        BREW_EXE="/usr/local/bin/brew"
        HOMEBREW_HOME="/usr/local/"
      else
        if [ -x /opt/homebrew/bin/brew ]
        then
          BREW_EXE="/opt/homebrew/bin/brew"
          HOMEBREW_HOME="/opt/homebrew/"
        else
          abort "Homebrew brew executable could not be located"
        fi
      fi
    fi
    if [ -f "${BASHINIT}" ]
    then
      grep "^eval \"\$(${BREW_EXE} shellenv)\"" "${BASHINIT}" > /dev/null || {
        echo 'eval "$(XXX shellenv)"' | sed -e "s%XXX%${BREW_EXE}%" >> "${BASHINIT}"
      }
    else
      echo 'eval "$(XXX shellenv)"' | sed -e "s%XXX%${BREW_EXE}%" > "${BASHINIT}"
    fi
    [ -f "${HOME}/.zshrc" ] && {
      grep "^eval \"\$(${BREW_EXE} shellenv)\"" "${HOME}/.zshrc" > /dev/null || {
        echo 'eval "$(XXX shellenv)"' | sed -e "s%XXX%${BREW_EXE}%" >> "${HOME}/.zshrc"
      }
    }
    eval "$(${BREW_EXE} shellenv)"
    have_brew=`type -p brew`
    [ "${have_brew}" ] && BREW_EXE="brew"
    log "Install gcc (recommended by brew)"
    ${BREW_EXE} install -q gcc > /dev/null 2>&1
  fi
  [ "${HOMEBREW_HOME}" ] || {
    brewpath=$(command -v brew)
    if [ $? -eq 0 ]
    then
      HOMEBREW_HOME=`dirname ${brewpath} | sed -e "s%/bin$%%"`
    else
      HOMEBREW_HOME="Unknown"
    fi
  }
}

install_neovim_dependencies () {
  log "Installing Neovim dependencies"
  PKGS="fd ripgrep fzf tmux go node python warrensbox/tap/tfswitch"
  for pkg in ${PKGS}
  do
    ${BREW_EXE} install -q ${pkg} > /dev/null 2>&1
  done
  if ! command -v cargo >/dev/null 2>&1; then
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
  fi
}

install_neovim_head () {
  if ! command -v nvim >/dev/null 2>&1; then
    log "Installing Neovim HEAD"
    ${BREW_EXE} install -q --HEAD neovim > /dev/null 2>&1
  elif [[ ! $(nvim --version) =~ "dev" ]]; then
    log "Neovim is installed but not HEAD version"
  else
    log "Skipping Neovim installation"
  fi
}

git_clone_neovim_config () {
  local neovim_config_path="$HOME/.config/nvim"
  if [[ -d "${neovim_config_path}" ]]; then
    log "${neovim_config_path} already exists"
  else
    log "Cloning Neovim config to ${neovim_config_path}"
    git clone https://github.com/doctorfree/nvim.git "${neovim_config_path}"
  fi
}

check_python () {
  have_python=`type -p python3`
  if [ "${have_python}" ]
  then
    PYTHON="python3"
  else
    have_python=`type -p python`
    if [ "${have_python}" ]
    then
      PYTHON="python"
    else
      PYTHON=
    fi
  fi
}

install_npm () {
  # Install nvm, node, npm, language servers
  NODE_VERSION="18.14.0"
  NVM_VERSION="0.39.3"
  NVM_INSTALL="https://raw.githubusercontent.com/nvm-sh/nvm/v$NVM_VERSION/install.sh"
  log "[*] Installing nvm $NVM_VERSION ..."
  # Installs Node Version Manager to ~/.nvm
  # Also detects bash or zsh and appends source lines
  # to ~/.bashrc or ~/.zshrc accordingly
  curl --silent --location "${NVM_INSTALL}" > /tmp/nvm-$$.sh
  [ $? -eq 0 ] || {
    rm -f /tmp/nvm-$$.sh
    curl --insecure --silent --location "${NVM_INSTALL}" > /tmp/nvm-$$.sh
    cat /tmp/nvm-$$.sh | sed -e "s/--fail/--insecure --fail/" > /tmp/n$$
    cp /tmp/n$$ /tmp/nvm-$$.sh
    rm -f /tmp/n$$
  }
  if [ -s /tmp/nvm-$$.sh ]
  then
    chmod 755 /tmp/nvm-$$.sh
    /bin/bash -c "/tmp/nvm-$$.sh" > /dev/null 2>&1
    rm -f /tmp/nvm-$$.sh
  else
    printf "\nERROR: Download of NVM installation script failed"
    printf "\nSee https://github.com/nvm-sh/nvm#installing-and-updating"
    printf "\nto manually install the node version manager\n"
  fi
  export NVM_DIR="$HOME/.nvm"
  # To get the nvm command working without sourcing bash configs
  [ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"
  # Installs specific version of Node
  have_nvm=`type -p nvm`
  [ "${have_nvm}" ] && {
    log "[*] Installing node $NODE_VERSION ..."
    nvm install $NODE_VERSION
  }
  have_npm=`type -p npm`
  [ "${have_npm}" ] && {
    log "[*] Setting npm config to use ~/.local as prefix ..."
    # npm install -g will now install to ~/.local/bin
    npm config set prefix '~/.local/'

    log "[*] Installing language servers ..."
    # Could also install the language servers with brew, for example:
    #   brew install bash-language-server
    #
    # python language server
    npm i -g pyright > /dev/null 2>&1
    # typescript language server
    npm i -g typescript typescript-language-server > /dev/null 2>&1
    # bash language server
    npm i -g bash-language-server > /dev/null 2>&1
    # awk language server
    npm i -g awk-language-server > /dev/null 2>&1
    # css language server
    npm i -g cssmodules-language-server > /dev/null 2>&1
    # vim language server
    npm i -g vim-language-server > /dev/null 2>&1
    # docker language server
    npm i -g dockerfile-language-server-nodejs > /dev/null 2>&1
    # For other language servers, see:
    # https://github.com/neovim/nvim-lspconfig/blob/master/doc/server_configurations.md
  }
  # Python virtual env in Asciiville breaks backwards compatibility
  log '[*] Installing Python dependencies ...'
  check_python
  [ "${PYTHON}" ] || {
    # Could not find Python, install with Homebrew
    log '[*] Installing Python with Homebrew ...'
    ${BREW_EXE} install -q python > /dev/null 2>&1
    check_python
  }
  [ "${PYTHON}" ] && {
    ${PYTHON} -m pip install wheel > /dev/null 2>&1
    ${PYTHON} -m pip install pynvim doq > /dev/null 2>&1
  }
}

main () {
  check_prerequisites
  local common_packages="git curl gip tar unzip"
  get_os
  if [[ $OS == "linux" ]]; then
    get_linux_distribution
    if [[ $LINUX_DISTRIBUTION == "debian" || $LINUX_DISTRIBUTION == "ubuntu" ]]; then
      log "Running on DEB based system"
      log "Updating package database"
      sudo apt -q -y update > /dev/null 2>&1
      # shellcheck disable=SC2086
      log "Installing common packages"
      sudo apt -q -y install build-essential ${common_packages} > /dev/null 2>&1
      install_brew
      install_neovim_dependencies
      install_neovim_head
      git_clone_neovim_config
    elif [[ $LINUX_DISTRIBUTION == "arch" ]]; then
      log "Running on Arch based system"
      log "Updating package database"
      sudo pacman -Sy --noconfirm > /dev/null 2>&1
      # shellcheck disable=SC2086
      log "Installing common packages"
      sudo pacman -S --noconfirm base-devel ${common_packages} > /dev/null 2>&1
      install_brew
      install_neovim_dependencies
      install_neovim_head
      git_clone_neovim_config
    elif [[ $LINUX_DISTRIBUTION == "fedora" ]]; then
      log "Running on RPM based system"
      log "Updating package database"
      sudo dnf --assumeyes --quiet update > /dev/null 2>&1
      # shellcheck disable=SC2086
      log "Installing common packages"
      sudo dnf --assumeyes --quiet groupinstall "Development Tools" > /dev/null 2>&1
      sudo dnf --assumeyes --quiet install ${common_packages} > /dev/null 2>&1
      install_brew
      install_neovim_dependencies
      install_neovim_head
      git_clone_neovim_config
    fi
  elif [[ $OS == "apple" ]];then
    log "Running on macOS system"
    # shellcheck disable=SC2086
    have_xcode=`type -p xcode-select`
    [ "${have_xcode}" ] || abort "Xcode must be installed. See the App store."
    xcode-select -p > /dev/null 2>&1
    [ $? -eq 0 ] || xcode-select --install
    install_brew
    log "Installing common packages"
    for pkg in ${common_packages}
    do
      ${BREW_EXE} install -q ${pkg} > /dev/null 2>&1
    done
    install_neovim_dependencies
    install_neovim_head
    git_clone_neovim_config
  fi
  install_npm
  INFO="################################################################
- Homebrew installed in ${HOMEBREW_HOME}
- See ${DOC_HOMEBREW}
- Neovim, nvm, node, npm, and language servers installed
- All packages required for Neovim managed by Homebrew
- This could result in duplicate tool installations!
- Your ~/.profile, ~/.bashrc, or ~/.zshrc modified to source paths

After this script is finished
- Log out or open a new shell
- If not invoked from ascinit run the command: nvim -c 'PlugInstall' -c 'qa'
- Open nvim and Enjoy!
################################################################"

  [ "${quiet}" ] || echo "${INFO}"
}

quiet=

while getopts "q" flag; do
  case $flag in
    q)
        quiet=1
        ;;
  esac
done

main
