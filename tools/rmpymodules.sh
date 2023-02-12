#!/bin/bash

for pkg in setuptools asciimatics ddgr rainbowstream socli future \
           xtermcolor ffmpeg-python pyaudio term-image urlscan pyfiglet \
           video-to-ascii
do
    python3 -m pip uninstall ${pkg}
done
