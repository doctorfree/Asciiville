#!/bin/sh
#
# $Id: bootstrap.sh,v 1.2 2005/09/17 14:58:33 j_ali Exp $
#
case "$0" in
    */*)	confdir=`echo $0 | sed 's:/[^/]*$::'` ;;
    *)		confdir='.' ;;
esac
set -e
cd $confdir
# Avoid autoheader creating backup file
rm -f autoconf.hin
aclocal -I m4
# We can't avoid running autotools in multiple directories,
# but at least we can have only one cache.
if [ -d autom4te.cache ]; then		# aclocal v1.7 doesn't create cache
    rm -rf ../../autom4te.cache
    mv autom4te.cache ../..
fi
# autoheader must be run from topdir since configure.ac refers
# to sys/autoconf/autoconf.hin
(cd ../.. && autoheader -I sys/autoconf sys/autoconf/configure.ac)
mv ../../autom4te.cache .
autoconf configure.ac > ../../configure
rm -rf autom4te.cache aclocal.m4
