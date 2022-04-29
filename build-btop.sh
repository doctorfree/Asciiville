#!/bin/bash
#
# Repo: https://github.com/doctorfree/Asciiville
#
# Fork of Repo: https://github.com/aristocratos/btop
#
# Build Dependencies:
# On Ubuntu 20.04
#   sudo apt update -y
#   sudo apt upgrade -y
#   sudo apt install coreutils git make tar zstd
#   sudo apt install -y build-essential
#   sudo apt install -y gcc-10 g++-10 cpp-10
#   sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 \
#                            --slave /usr/bin/g++ g++ /usr/bin/g++-10 \
#                            --slave /usr/bin/gcov gcov /usr/bin/gcov-10
#
# Usage: ./build-btop.sh [-i]
# Where -i indicates install btop after compiling

usage() {
    printf "\nUsage: ./build-btop.sh [-i] [-p prefix] [-u]"
    printf "\nWhere:"
    printf "\n\t-i indicates install"
    printf "\n\t-p prefix specifies installation prefix (default /usr)"
    printf "\n\t-u displays this usage message and exits\n"
    printf "\nNo arguments: configure with prefix=/usr, build\n"
    exit 1
}

PROJ=btop
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
    echo "Run: git clone https://github.com/doctorfree/Asciiville"
    echo "Exiting"
    exit 1
}

cd ${PROJ}

[ -f bin/btop ] || make STATIC=true STRIP=true
chmod +x bin/btop

[ "${INSTALL}" ] && sudo PREFIX=${PREFIX} make install
