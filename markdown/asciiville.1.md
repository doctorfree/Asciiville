---
title: ASCIIVILLE
section: 1
header: User Manual
footer: asciiville 1.0.0
date: April 16, 2022
---
# NAME
asciiville - Launch an MPD music player client and cava spectrum visualizer

# SYNOPSIS
**asciiville** [-A] [-a] [-b] [-c] [-C client] [-D] [-d music_directory] [-f]
		[-h] [-i] [-jJ] [-k] [-m] [-M enable|disable|start|stop|restart|status]
		[-n num] [-N] [-p] [-P script] [-q] [-r] [-R] [-S] [-t] [-T] [-u]

# DESCRIPTION
The *asciiville* command acts as a front-end for launching the mpcplus music player client and cava spectrum visualizer in various terminal emulators and window placements. It can be used to display these utilities juxtaposed in separate windows or fullscreen overlayed with transparency. Alternately, asciiville can launch the cantata MPD client or any specified MPD client along with the cava spectrum visualizer. Command line options also support running the *asciiville* windows in a tmux session and recording that session using *asciinema*.

The *asciiville* command can be used to control the *mpd* and *mpd.socket* system services when invoked with the `-M action` command line option. The Music Player Daemon (MPD) can be started, stopped, enabled, disabled, restarted, and status queried.

The *asciiville* command can also act as a front-end to the *mppsplash* and *mppsplash-tmux* commands when invoked with the `-S` and `-T` command line options.

When invoked with the `-i` option, `asciiville` presents a selection menu and operates in interactive mode.

Occasionally a tmux session or asciimatics script will hang. Previously started tmux sessions and asciimatics scripts can be quickly and easily killed by executing the `asciiville -k` command.

# COMMAND LINE OPTIONS
*MPCplus/Visualizer options:*

**-A**
: indicates display album cover art (implies tmux session)

**-c**
: indicates use cantata MPD client rather than mpcplus

**-C 'client'**
: indicates use 'client' MPD client rather than mpcplus

**-f**
: indicates fullscreen display

**-h**
: indicates half-height for cava window (with -f only)

**-P script**
: specifies the ASCIImatics script to run in visualizer pane

**-q**
: indicates quarter-height for cava window (with -f only)

**-r**
: indicates use retro terminal emulator

**-t**
: indicates use tilix terminal emulator

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

**-D**
: indicates download album cover art

**-d 'music_directory'**
: specifies the music directory to use for downloaded album cover art (without this option -D will use the `music_directory` setting in `/etc/mpd.conf`

**-i**
: indicates interactive mode with selection menus

**-k**
: indicates kill MusicPlayerPlus tmux sessions and ASCIImatics scripts

**-M 'enable|disable|start|stop|restart|status'**
: Enable, disable, start, stop, restart, or get the status of the MPD and MPD socket system services 

**-R**
: indicates record tmux session with asciinema

**-T**
: indicates use a tmux session for either ASCIImatics or mpcplus

**-u**
: Displays this usage message and exits

# EXAMPLES
**asciiville**
: Launches `mpcplus` music player client running in xfce4 terminal emulator with cava spectrum visualizer running in a gnome-terminal terminal emulator window. 

**asciiville -i**
: Launches `asciiville` in interactive mode with menu selections controlling actions rather than command line arguments

**asciiville -r**
: Launches `mpcplus` music player client running in cool-retro-term terminal emulator with cava spectrum visualizer running in a gnome-terminal terminal emulator window. 

**asciiville -c**
: Launches `cantata` music player client running in a separate window with cava spectrum visualizer running in a gnome-terminal terminal emulator window. 

**asciiville -C cmus**
: Launches the `cmus` music player client with cava spectrum visualizer running in a gnome-terminal terminal emulator window. 

**asciiville -C mcg**
: Launches the CoverGrid music player client (`mcg`) running in a separate window with cava spectrum visualizer running in a gnome-terminal terminal emulator window. 

**asciiville -f -q -t**
: Launches `mpcplus` music player client in fullscreen mode with cava spectrum visualizer in quarter-screen mode, both running in a tilix terminal emulator window. 

**asciiville -a -T**
: Launches `mpcplus` music player client and visualizer running in a tmux session displaying album cover art. 

**asciiville -M stop**
: Stops the Music Player Daemon service and the associated MPD socket service

**asciiville -R -T**
: Creates an asciinema recording of `mpcplus` music player client and visualizer running in a tmux session

**asciiville -S -j -a**
: Launch `mppsplash` displaying the Julia Set asciimatics animation with audio

**asciiville -d**
: Download album cover art for any albums in the music library that do not already have cover art 

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
**mppsplash**(1), **mpcplus**(1), **mpcpluskeys**(1), **cava**(1)

Full documentation and sources at:

https://github.com/doctorfree/Asciiville

