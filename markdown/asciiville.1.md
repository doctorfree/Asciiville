---
title: ASCIIVILLE
section: 1
header: User Manual
footer: asciiville 1.0.0
date: April 16, 2022
---
# NAME
asciiville - Launch a terminal emulator and specified character based command, ascii art, asciimatics animation, and more

# SYNOPSIS
**asciiville** [-a] [-b] [-c client] [-f] [-g] [-i]
		[-jJ] [-k] [-l] [-m] [-n num] [-N] [-p] [-P script]
		[-r] [-R] [-s song] [-S] [-t] [-T] [-x] [-y] [-z] [-u]

# DESCRIPTION
The *asciiville* command acts as a front-end for launching character based utilities and ascii art in various terminal emulators. Asciiville can be used to launch any specified character based command. Command line options also support running the *asciiville* window in a tmux session and recording that session using *asciinema*.

The *asciiville* command can also act as a front-end to the *asciisplash* and *asciisplash-tmux* commands when invoked with the `-S` and `-T` command line options.

When invoked with the `-i` option, `asciiville` presents a selection menu and operates in interactive mode.

Occasionally a tmux session or asciimatics script will hang. Previously started tmux sessions and asciimatics scripts can be quickly and easily killed by executing the `asciiville -k` command.

# COMMAND LINE OPTIONS

*Terminal/Command options:*

**-c 'command'**
: indicates use 'command'

**-f**
: indicates fullscreen display

**-g**
: indicates use gnome terminal emulator

**-i**
: indicates start asciiville in interactive mode

**-l**
: indicates use lynx as the default command

**-P script**
: specifies the ASCIImatics script to run in visualizer pane

**-r**
: indicates use retro terminal emulator

**-t**
: indicates use tilix terminal emulator

**-x**
: indicates use xfce4 terminal emulator

**-y**
: indicates use ranger as the default command

**-z**
: indicates use neomutt as the default command

*ASCIImatics animation options:*

**-a**
: indicates play audio during ASCIImatics display

**-b**
: indicates use backup audio during ASCIImatics display

**-j**
: indicates use Julia Set scenes in ASCIImatics display

**-J**
: indicates Julia Set with several runs using different parameters

**-m**
: indicates use MusicPlayerPlus scenes in ASCIImatics display

**-n num**
: specifies the number of times to cycle ASCIImatics scenes

**-N**
: indicates use alternate comments in Plasma ASCIImatics scenes

**-p**
: indicates use Plasma scenes in ASCIImatics display

**-s song**
: specifies a song to accompany an ASCIImatics animation

**-S**
: indicates display ASCIImatics splash animation

*General options:*

**-k**
: indicates kill Asciiville tmux sessions and ASCIImatics scripts

**-R**
: indicates record tmux session with asciinema

**-T**
: indicates use a tmux session for either ASCIImatics or command

**-u**
: Displays this usage message and exits

# EXAMPLES
**asciiville**
: Launches `btop` system monitor running in a gnome-terminal emulator window. 

**asciiville -i**
: Launches `asciiville` in interactive mode with menu selections controlling actions rather than command line arguments

**asciiville -r -y**
: Launches `ranger` file manager running in cool-retro-term terminal emulator. 

**asciiville -c cmus -g**
: Launches the `cmus` music player client running in a gnome-terminal emulator window. 

**asciiville -f -t -z**
: Launches `neomutt` mail client in fullscreen mode running in a tilix terminal emulator window. 

**asciiville -l -T -x**
: Launches `lynx` web browser running in a tmux session in an xfce4-terminal window. 

**asciiville -R -T**
: Creates an asciinema recording of `btop` system monitor running in a tmux session

**asciiville -S -j -a**
: Launch `asciisplash` displaying the Julia Set asciimatics animation with audio

# AUTHORS
Written by Ronald Record github@ronrecord.com

# LICENSING
ASCIIVILLE is distributed under an Open Source license.
See the file LICENSE in the ASCIIVILLE source distribution
for information on terms &amp; conditions for accessing and
otherwise using ASCIIVILLE and for a DISCLAIMER OF ALL WARRANTIES.

# BUGS
Submit bug reports online at:

https://github.com/doctorfree/Asciiville/issues

# SEE ALSO
**asciijulia**(1), **asciimpplus**(1), **asciiplasma**(1), **asciisplash**(1), **asciisplash-tmux**(1)

Full documentation and sources at:

https://github.com/doctorfree/Asciiville

