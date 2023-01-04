#!/bin/bash
#
# install-dev-env.sh - install or remove the build dependencies

arch=
centos=
debian=
fedora=
[ -f /etc/os-release ] && . /etc/os-release
[ "${ID_LIKE}" == "debian" ] && debian=1
[ "${ID}" == "arch" ] && arch=1
[ "${ID}" == "centos" ] && centos=1
[ "${ID}" == "fedora" ] && fedora=1
[ "${debian}" ] || [ -f /etc/debian_version ] && debian=1

if [ "${debian}" ]
then
  PKGS="build-essential autotools-dev autoconf libtool cmake flex bison \
        pkg-config libncurses-dev coreutils git make tar zstd make g++ \
        libjpeg-dev libpng-dev libssl-dev libncursesw5-dev gcc-10 \
        g++-10 cpp-10"
  if [ "$1" == "-r" ]
  then
    sudo apt remove ${PKGS}
  else
    sudo apt install ${PKGS} pandoc zip
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 \
        --slave /usr/bin/g++ g++ /usr/bin/g++-10 \
        --slave /usr/bin/gcov gcov /usr/bin/gcov-10
  fi
else
  if [ "${arch}" ]
  then
    PKGS="base-devel cmake ncurses flex bison"
    if [ "$1" == "-r" ]
    then
      sudo pacman -Rs ${PKGS}
    else
      sudo pacman -S --needed ${PKGS} pandoc zip
    fi
  else
    have_dnf=`type -p dnf`
    if [ "${have_dnf}" ]
    then
      PINS=dnf
    else
      PINS=yum
    fi
    sudo ${PINS} makecache
    if [ "${fedora}" ]
    then
      PKGS="cmake ncurses-devel flex bison libtool automake \
            libjpeg-turbo-devel libpng-devel glibc-static libstdc++-static"
      if [ "$1" == "-r" ]
      then
        sudo ${PINS} -y remove ${PKGS}
        sudo ${PINS} -y remove gcc-c++
        sudo ${PINS} -y groupremove "Development Tools" "Development Libraries"
      else
        sudo ${PINS} -y groupinstall "Development Tools" "Development Libraries"
        sudo ${PINS} -y install gcc-c++
        sudo ${PINS} -y install ${PKGS} pandoc zip
      fi
    else
      if [ "${centos}" ]
      then
        sudo alternatives --set python /usr/bin/python3
        PKGS="cmake ncurses-devel flex bison libtool automake \
              libjpeg-turbo-devel libpng-devel"
        if [ "$1" == "-r" ]
        then
          sudo ${PINS} -y remove ${PKGS}
          sudo ${PINS} -y remove gcc-c++
          sudo ${PINS} -y groupremove "Development Tools"
        else
          sudo ${PINS} -y groupinstall "Development Tools"
          sudo ${PINS} -y install gcc-c++
          sudo ${PINS} -y install ${PKGS} pandoc zip
        fi
      else
        echo "Unrecognized operating system"
      fi
    fi
  fi
fi
