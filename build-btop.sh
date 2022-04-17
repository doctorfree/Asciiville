#!/bin/bash
#
# Repo: https://github.com/doctorfree/Asciiville
#
# Fork of Repo: https://github.com/aristocratos/btop
#
# Build Dependencies:
#   coreutils git make tar zstd
#
# Usage: ./build-btop.sh [-i]
# Where -i indicates install btop after compiling

usage() {
    printf "\nUsage: ./build-btop.sh [-aCiv] [-p prefix] [-u]"
    printf "\nWhere:"
    printf "\n\t-p prefix specifies installation prefix (default /usr)"
    printf "\n\t-u displays this usage message and exits\n"
    printf "\nNo arguments: configure with prefix=/usr, build\n"
    exit 1
}

PROJ=btop
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

make STATIC=true STRIP=true
chmod +x bin/btop

[ "${INSTALL}" ] && sudo PREFIX=${PREFIX} make install
