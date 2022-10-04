#!/bin/bash

art_font_size=6

kitty --title="Play Ascii Art" \
      --start-as normal \
      --override font_size=${art_font_size} \
      --override initial_window_width=320c \
      --override initial_window_height=80c \
      --override tab_bar_min_tabs=2 \
      ./Asciiville-Intro-1.3.1.mkv.sh 2> /dev/null
