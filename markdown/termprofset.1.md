---
title: TERMPROFSET
section: 1
header: User Manual
footer: termprofset 1.0.0
date: May 10, 2022
---
# NAME
termprofset - manipulate the profiles of a Gnome or Tilix terminal

# SYNOPSIS
**termprofset** [-f fontname] [-s fontsize] [-p profile] [-glrtu]

# DESCRIPTION
The *termprofset* command can be used to set or restore profile settings of a Gnome or Tilix terminal emulator (`gnome-terminal` and `tilix` commands). Profile settings that can be configured by *termprofset* are:

- font (font name and font size)
- use-system-font
- audible-bell
- use-theme-colors
- background-color
- foreground-color

When setting the profile of a terminal, the following settings are used:

- font 'Monospace 4'
- use-system-font false
- audible-bell false
- use-theme-colors false
- background-color '#000000'
- foreground-color '#AFAFAF'

The font name and font size can be specified on the command line using the `-f fontname` and `-s fontsize` options.

In addition to setting profile settings, *termprofset* can be used to restore profile settings saved from a previous invocation. Use the `-r` command line option to indicate restore saved profile settings. In this way, *termprofset* can be used to manipulate a profile temporarily. Display ascii art with a customized profile then restore the original profile settings.

Current profile settings can be listed with the `-l` option.

# COMMAND LINE OPTIONS
**-f 'fontname'**
: specifies the font name to set terminal's font to

**-s 'fontsize'**
: specifies the font size to set terminal's font to

**-p 'profile'**
: specifies the terminal profile to manipulate

**-r**
: indicates restore saved settings

**-g**
: indicates use Gnome terminal emulator

**-l**
: indicates list current profile settings and exit

**-t**
: indicates use Tilix terminal emulator

**-u**
: displays this usage message and exits

Default terminal emulator is Gnome

Default terminal profile is Asciiville

Default font is Monospace

Default font size is 4

# EXAMPLES

**termprofset**
: Without arguments, termprofset sets the Asciiville Gnome profile with a Monospace font and font size 4

**termprofset -t -s 8**
: Sets the Asciiville Tilix profile with a Monospace font and font size 8 

**termprofset -r -t**
: Restores the Asciiville Tilix profile with settings saved from a previous run

**termprofset -p default -t -s 18**
: Sets the default Tilix profile with a Monospace font and font size 18 

# AUTHORS
Written by Ronald Record github@ronrecord.com

# LICENSING
TERMPROFSET is distributed under an Open Source license.
See the file LICENSE in the TERMPROFSET source distribution
for information on terms &amp; conditions for accessing and
otherwise using TERMPROFSET and for a DISCLAIMER OF ALL WARRANTIES.

# BUGS
Submit bug reports online at:

https://github.com/doctorfree/Asciiville/issues

# SEE ALSO
**asciimpplus**(1), **asciiplasma**(1), **asciisplash**(1), **asciisplash-tmux**(1), **asciiville**(1), **show_ascii_art**(1)

Full documentation and sources at:

https://github.com/doctorfree/Asciiville

