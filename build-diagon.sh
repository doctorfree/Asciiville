#!/bin/bash
#
# Repo: https://github.com/doctorfree/diagon
#
# Fork of Repo: https://github.com/ArthurSonzogni/Diagon
#
# Build Dependencies:
# On Ubuntu 20.04
#   sudo apt update -y
#   sudo apt upgrade -y
#   sudo apt install make uuid-dev libboost-graph-dev cmake default-jdk
#
# Usage: ./build-diagon.sh [-i]
# Where -i indicates install diagon after compiling

usage() {
    printf "\nUsage: ./build-diagon.sh [-i] [-p prefix] [-u]"
    printf "\nWhere:"
    printf "\n\t-i indicates install"
    printf "\n\t-p prefix specifies installation prefix (default /usr)"
    printf "\n\t-u displays this usage message and exits\n"
    printf "\nNo arguments: configure with prefix=/usr, build\n"
    exit 1
}

PROJ=diagon
INSTALL=
PREFIX=/usr
while getopts "ip:u" flag; do
    case $flag in
        i)
            INSTALL=1
            ;;
        p)
            PREFIX="$OPTARG"
            ;;
        u)
            usage
            ;;
    esac
done
shift $(( OPTIND - 1 ))

[ -d ${PROJ} ] || {
    echo "$PROJ does not exist or is not a directory."
    echo "Run: git clone https://github.com/doctorfree/diagon"
    echo "Exiting"
    exit 1
}

cd ${PROJ}

buildit=1
[ -L build/diagon ] && {
  diagonpath=`realpath build/diagon`
  [ -f ${diagonpath} ] && buildit=
}

[ "${buildit}" ] && {
  [ -d build ] || mkdir build
  cd build
  cmake .. -DCMAKE_BUILD_TYPE=Release
  make -j
}

[ "${INSTALL}" ] && sudo PREFIX=${PREFIX} make install
