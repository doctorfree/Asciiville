#!/bin/bash
#
# Repo: https://github.com/doctorfree/Asciiville
#
# Fork of Repo: https://github.com/NetHack/NetHack
#
# Usage: ./build-nethack.sh [-i]
# Where -i indicates install nethack after compiling

usage() {
    printf "\nUsage: ./build-nethack.sh [-i] [-p prefix] [-u]"
    printf "\nWhere:"
    printf "\n\t-i indicates install"
    printf "\n\t-p prefix specifies installation prefix (default /usr)"
    printf "\n\t-u displays this usage message and exits\n"
    printf "\nNo arguments: configure with prefix=/usr, build\n"
    exit 1
}

PROJ=nethack
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

[ -f src/nethack ] || {
    sys/unix/setup.sh sys/unix/hints/linux-asciiville
    make fetch-lua
    make
    cd doc
    make
    cd ..
}
chmod +x src/nethack

[ "${INSTALL}" ] && sudo PREFIX=${PREFIX} make install
