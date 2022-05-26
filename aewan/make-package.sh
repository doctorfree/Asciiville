#!/bin/sh

if ! [ -f configure ]; then
   echo "./configure does not exist. Run autoconf."
   exit 1
fi

/bin/rm -rf autom4te.cache

X=`tempfile`
/bin/rm -f $X
mkdir $X

BASENAME=aewan-`cat version.h | grep AEWAN_PROGRAM_VERSION | grep -v AEWAN_PROGRAM_VERSION_NAME | cut -d\" -f2`

(cd .. && cp -a aewan $X/$BASENAME)
(cd $X && find $X -name CVS -type d -exec rm -rf {} \; 2>/dev/null )

(cd $X && tar -zc $BASENAME) > ../$BASENAME.tar.gz

/bin/rm -rf $X
echo "../$BASENAME.tar.gz generated."

