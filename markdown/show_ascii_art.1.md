---
title: SHOE_ASCII_ART
section: 1
header: User Manual
footer: show_ascii_art 1.0.0
date: April 24, 2022
---
# NAME
show_ascii_art - display ascii art

# SYNOPSIS
**show_ascii_art** [-a art] [-A art_dir] [-b] [-c] [-C depth] [-d font_dir] [-D seconds] [-F large_font] [-f small_font] [-g] [-i image] [-l] [-L] [-n tabs] [-o] [-p palette] [-q] [-r] [-s show] [-S] [-u] [-t first_text] [-T second_text] [-h height] [-w width]

# DESCRIPTION
The *show_ascii_art* command displays one of the ascii art images included in
Asciiville. Command line options can be used to tell *show_ascii_art* to create
a new ascii art image from an image file in any format. Other command line
options can be used to specify the width and heigh of the converted image,
the fonts used to display accompanying text, and the text to display.

# COMMAND LINE OPTIONS

**-a 'art'**
: specifies which ascii art to display

**-A 'art_dir'**
: specifies the path to the ascii art folder

**-b**
: when generating ascii art use a border

**-c**
: when generating ascii art use ANSI colors

**-C 'depth'**
: specifies the color depth

    'depth' can be '4' (for ANSI), '8' (for 256 color palette)

    or '24' (for truecolor or 24-bit color)

**-d 'font_dir'**
: specifies the path to the figlet fonts

**-D 'seconds'**
: specifies the delay, in seconds, between screens

**-f 'small_font'**
: specifies the figlet font to use for small text

**-F 'large_font'**
: specifies the figlet font to use for large text

**-g**
: convert image to grayscale

**-i 'image'**
: specifies an image file to convert to ascii art

**-l**
: use lolcat coloring

**-L**
: lists the ascii art in the 'art_dir' and exits

**-n 'tabs'**
: specifies the number of tabs to indent image display

**-o**
: indicates overwrite any existing ascii art when saving

**-p 'palette'**
: specifies which character set to use for ascii art

    'palette' can be one of 'def', 'long', 'rev', 'longrev'

    'def' is the default set, 'long' a long set,

    'rev' reverses default, 'longrev' reverses long

    Any other argument to '-C' will be taken as the character set

**-q**
: don't display text, just the ascii art

**-r**
: indicates select random fonts

**-s 'show'**
: slide show of ascii art

    'show' can be Art, Fractals, Lyap, Owls, Waterfalls, Mixed

**-S**
: indicates save converted image ascii art in art_dir

**-t 'first_text'**
: specifies the first text to display

**-T 'second_text'**
: specifies the second text to display

**-h 'height'**
: specifies the height of the converted ascii art

**-w 'width'**
: specifies the width of the converted ascii art

If only one of 'width' and 'height' is provided, calculate the other from image aspect ratio

**-u**
: displays this usage message and exits

# EXAMPLES
**show_ascii_art**
: Without options show_ascii_art will display an ascii art image and "Welcome to Asciiville" text using Figlet fonts.

**show_ascii_art -i $HOME/Pictures/selfie.gif**
: Converts the GIF image file `$HOME/Pictures/selfie.gif` to JPEG format using the ImageMagick *convert* utility then generates ascii art from the JPEG file using the Asciiville *jp2a* utility and displays it along with figlet text.

**show_ascii_art -i $HOME/Pictures/profile.jpg -h 20 -r**
: Generates a 20 line ascii art from the JPEG image `$HOME/Pictures/profile.jpg` preserving aspect ratio using the Asciiville *jp2a* utility and displays it along with figlet text using randomly selected fonts.

# AUTHORS
Written by Ronald Record github@ronrecord.com

# LICENSING
SHOE_ASCII_ART is distributed under an Open Source license.
See the file LICENSE in the SHOE_ASCII_ART source distribution
for information on terms &amp; conditions for accessing and
otherwise using SHOE_ASCII_ART and for a DISCLAIMER OF ALL WARRANTIES.

# BUGS
Submit bug reports online at:

https://github.com/doctorfree/Asciiville/issues

# SEE ALSO
**asciiart**(1), **asciimpplus**(1), **asciiplasma**(1), **asciisplash**(1), **asciisplash-tmux**(1), **asciiville**(1)

Full documentation and sources at:

https://github.com/doctorfree/Asciiville

