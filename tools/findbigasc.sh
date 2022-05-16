#!/bin/bash

find art -name \*\.asc | while read i
do
    wid=`head -1 "$i" | wc -c`
    [ ${wid} -gt 350 ] && {
        echo "$i width=${wid}"
    }
done
