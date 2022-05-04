---
title: ASCINIT
section: 1
header: User Manual
footer: ascinit 1.0.0
date: May 04, 2022
---
# NAME
ascinit - Asciiville initialization script

# SYNOPSIS
**ascinit** [-a] [-d] [-m] [-M] [-n] [-N] [-q] [-t] [-u]

# DESCRIPTION
The *ascinit* command should be run as a normal user (not the root user
and not with `sudo`) after installing Asciiville. It performs several
configuration initializations for the user. These include setting up:

* Asciiville profiles in Gnome and Tilix terminals
* Mutt and/or NeoMutt startup files
* Tmux configuration
* default Ranger and Rifle configuration files
* Asciimatics and Rainbowstream installation
* Optionally authorizing the Rainbow Stream app with Twitter

Although command line options are provided to control the action(s) of the
`ascinit` command (see below), the typical invocation will simply be `ascinit`
with no options. This default invocation performs a NeoMutt configuration,
does not configure Mutt, configures Tmux and Ranger and Rifle, installs
Asciimatics and Rainbowstream if not already installed, does not authorize
Rainbow Stream with Twitter, and creates an Asciiville profile in the terminals.

# COMMAND LINE OPTIONS
**-a**
: indicates do not ask to play an animation when done

**-d**
: indicates debug mode

**-m**
: indicates setup user Mutt configuration

**-M**
: indicates setup both Mutt and NeoMutt configurations

**-n**
: indicates setup user NeoMutt configuration

**-N**
: indicates setup neither Mutt or NeoMutt configuration

**-q**
: indicates quiet mode

**-t**
: indicates authorize the Rainbow Stream app at Twitter

**-u**
: indicates display this usage message and exit

# AUTHORS
Written by Ronald Record github@ronrecord.com

# LICENSING
ASCINIT is distributed under an Open Source license.
See the file LICENSE in the ASCINIT source distribution
for information on terms &amp; conditions for accessing and
otherwise using ASCINIT and for a DISCLAIMER OF ALL WARRANTIES.

# BUGS
Submit bug reports online at:

https://github.com/doctorfree/Asciiville/issues

# SEE ALSO
**asciiart**(1), **asciimpplus**(1), **asciiplasma**(1), **asciisplash**(1), **asciisplash-tmux**(1), **asciiville**(1)

Full documentation and sources at:

https://github.com/doctorfree/Asciiville

