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
**asciiville** [-a] [-A] [-b] [-c command] [-f] [-g] [-i] [-I]
		[-jJ] [-k] [-l] [-L] [-m] [-M] [-n num] [-N] [-p] [-P script]
		[-r] [-R] [-s song] [-S] [-t] [-T] [-x] [-y] [-z] [-u]

# DESCRIPTION
The *asciiville* command acts as a front-end for launching character based utilities and ascii art in various terminal emulators. Asciiville can be used to launch any specified character based command. Command line options also support running the *asciiville* window in a tmux session and recording that session using *asciinema*.

The *asciiville* command can also act as a front-end to the *asciisplash* and *asciisplash-tmux* commands when invoked with the `-S` and `-T` command line options.

When invoked with the `-i` option, `asciiville` presents a selection menu and operates in interactive mode. Included in the wide variety of options available in the Asciiville interactive menus are selections to display the *MusicPlayerPlus* and *RoonCommandLine* interactive menus.

The interactive menu interface provides three types of menu options. Some menu selections trigger the execution of a command. These are usually menu entries beginning with *Run*. Other menu selections are used to set the command that would be run when a terminal emulator is selected. These are usually menu entries beginning with *Use*. Finally, some menu selections can be used to toggle preferences like *Fullscreen*, *Use Tmux*, and *Record Tmux Session*. Once a command has been selected with the *Use* menu entry and desired options are set then the command can be executed by selecting one of the terminal emulators (menu entries beginning with *Run <command> in <terminal name> Terminal*).

Previously started tmux sessions and asciimatics scripts can be quickly and easily terminated by executing the `asciiville -k` command.

# COMMAND LINE OPTIONS

*Terminal/Command options:*

**-c 'command'**
: Indicates run 'command' in selected terminal window. If *command* is one of the special keywords (*maps*, *moon*, *search*, *speed*, *twitter*, *weather*) then display a map, the phase of the Moon, perform a web search, perform a speed test, run the command line twitter client, or display a weather report.

**-f**
: Indicates fullscreen display

**-g**
: Indicates use gnome terminal emulator

**-i**
: Indicates start asciiville in interactive mode

**-I**
: Indicates display system info

**-l**
: Indicates use lynx as the default command

**-L**
: Indicates use lolcat coloring where appropriate

**-P script**
: Specifies the ASCIImatics script to run

**-r**
: Indicates use retro terminal emulator

**-t**
: Indicates use tilix terminal emulator

**-x**
: Indicates use xfce4 terminal emulator

**-y**
: Indicates use ranger as the default command

**-z**
: Indicates use neomutt as the default command

*ASCIImatics animation options:*

**-a**
: Indicates play audio during ASCIImatics display

**-A**
: Indicates use Asciiville scenes in ASCIImatics display

**-b**
: Indicates use backup audio during ASCIImatics display

**-j**
: Indicates use Julia Set scenes in ASCIImatics display

**-J**
: Indicates Julia Set with several runs using different parameters

**-m**
: Indicates use MusicPlayerPlus scenes in ASCIImatics display

**-M**
: Indicates use the MusicPlayerPlus `mpcplus` music player client

**-n num**
: Specifies the number of times to cycle ASCIImatics scenes

**-N**
: Indicates use alternate comments in Plasma ASCIImatics scenes

**-p**
: Indicates use Plasma scenes in ASCIImatics display

**-s song**
: Specifies a song to accompany an ASCIImatics animation

**-S**
: Indicates display ASCIImatics splash animation

*General options:*

**-k**
: Indicates kill Asciiville tmux sessions and ASCIImatics scripts

**-R**
: Indicates record tmux session with asciinema

**-T**
: Indicates use a tmux session for either ASCIImatics or command

**-u**
: Displays this usage message and exits

# EXAMPLES
**asciiville**
: Launches `btop` system monitor running in a gnome-terminal emulator window. 

**asciiville -i**
: Launches `asciiville` in interactive mode with menu selections controlling actions rather than command line arguments

**asciiville -r -y**
: Launches `ranger` file manager running in cool-retro-term terminal emulator. 

**asciiville -M -t**
: Launches `mpcplus` music player running in Tilix terminal emulator. 

**asciiville -c maps**
: Displays a zoomable map of the world using `mapscii`. 

**asciiville -c moon**
: Displays the Phase of the Moon using `wttr.in`. 

**asciiville -c search**
: Launches the `ddgr` command line web search in the current terminal window. 

**asciiville -c twitter**
: Launches the `rainbowstream` command line Twitter client in the current terminal window. 

**asciiville -c weather**
: Displays a weather report for your IP address location using `wttr.in`. 

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
**asciiart**(1), **asciijulia**(1), **asciimpplus**(1), **asciinema**(1), **asciiplasma**(1), **asciisplash**(1), **asciisplash-tmux**(1), **ddgr**(1), **lynx**(1), **neomutt**(1), **ranger**(1)

Full documentation and sources at:

https://github.com/doctorfree/Asciiville

