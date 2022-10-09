#!/bin/sh

if [ -e /usr/bin/w3m ]; then
    /usr/bin/w3m "$@" 2>/dev/null &
    exit 0
fi

if [ -e /usr/bin/firefox ]; then
    /usr/bin/firefox "$@" 2>/dev/null &
    exit 0
fi

if [ -e /Applications/ ]; then
    open --background "$@" 2>/dev/null &
    exit 0
fi

# Fallback: might be a headless system
echo "$@" | less 
exit 0
