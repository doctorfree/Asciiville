#!/bin/bash

find . -name \*\.asc | while read i
do
    wid=`head -1 "$i" | wc -c`
    [ ${wid} -lt 50 ] && {
        echo "$i width=${wid}"
    }
done
