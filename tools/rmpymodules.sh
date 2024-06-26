#!/bin/bash

VENV_DIR="${HOME}/.venv"
[ -f ${VENV_DIR}/bin/activate ] && source ${VENV_DIR}/bin/activate

for pkg in setuptools asciimatics ddgr socli future xtermcolor \
           ffmpeg-python pyaudio term-image urlscan pyfiglet durdraw video-to-ascii
do
    python3 -m pip uninstall -y ${pkg}
done
