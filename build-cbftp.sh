#!/bin/bash
#
# Repo: https://github.com/doctorfree/Asciiville
#
# Fork of Repo: https://github.com/cbftp/cbftp-2022
#
# Linux build dependencies: make g++ libssl-dev libncursesw5-dev
# On Ubuntu 20.04
#   sudo apt update -y
#   sudo apt upgrade -y
#   sudo apt install make g++ libssl-dev libncursesw5-dev
#
# Usage: ./build-cbftp.sh [-i]
# Where -i indicates install cbftp after compiling

usage() {
    printf "\nUsage: ./build-cbftp.sh [-i] [-p prefix] [-u]"
    printf "\nWhere:"
    printf "\n\t-i indicates install"
    printf "\n\t-p prefix specifies installation prefix (default /usr)"
    printf "\n\t-u displays this usage message and exits\n"
    printf "\nNo arguments: configure with prefix=/usr, build\n"
    exit 1
}

PROJ=cbftp
INSTALL=
PREFIX=
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
    echo "Run: git clone https://github.com/doctorfree/Asciiville"
    echo "Exiting"
    exit 1
}

cd ${PROJ}

[ -f bin/cbftp ] || make
chmod +x bin/*

[ "${INSTALL}" ] && {
    [ -d ${PREFIX}/usr ] || sudo mkdir -p ${PREFIX}/usr
    [ -d ${PREFIX}/usr/bin ] || sudo mkdir -p ${PREFIX}/usr/bin
    sudo cp bin/* ${PREFIX}/usr/bin
}
