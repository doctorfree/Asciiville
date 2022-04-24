#!/bin/bash
#
# Repo: https://github.com/doctorfree/Asciiville
#
# Fork of Repo: https://github.com/cslarsen/jp2a
#
# Usage: ./build-jp2a.sh [-i]
# Where -i indicates install jp2a after compiling

usage() {
    printf "\nUsage: ./build-jp2a.sh [-i] [-p prefix] [-u]"
    printf "\nWhere:"
    printf "\n\t-i indicates install"
    printf "\n\t-p prefix specifies installation prefix (default /usr)"
    printf "\n\t-u displays this usage message and exits\n"
    printf "\nNo arguments: configure with prefix=/usr, build\n"
    exit 1
}

PROJ=jp2a
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
[ -f src/jp2a ] || {
    autoreconf -vi
#   ./configure --with-jpeg-prefix=/usr/local \
#               --with-curl-config=`which curl-config`
    ./configure
    make -j
}

chmod +x src/jp2a

[ "${INSTALL}" ] && sudo PREFIX=${PREFIX} make install
