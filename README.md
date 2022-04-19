# Asciiville

Asciiville is where you go for ASCII art, animations, and utilities.
The Asciiville project provides integration and extension of several
packages as well as providing convenience commands to invoke a variety
of components used to display ASCII art, animations, and utilities.

## Table of contents

1. [Overview](#overview)
1. [Asciiville Commands](#asciiville-commands)
1. [Quickstart](#quickstart)
1. [Requirements](#requirements)
1. [Installation](#installation)
    1. [Debian package installation](#debian-package-installation)
    1. [RPM Package installation](#rpm-package-installation)
1. [Documentation](#documentation)
    1. [Btop++ README](#btop++-readme)
    1. [Man Pages](#man-pages)
    1. [Usage](#usage)
    1. [Example invocations](#example-invocations)
1. [Removal](#removal)

## Overview

Asciiville integrations and extensions are aimed at the character
based terminal user. They enable an easy to use seamlessly
integrated control of a variety of ASCII art, animation, and
utilities in a lightweight character based environment.

At the core of Asciiville is the `asciiville` command which acts as
a front-end for a variety of terminal and/or `tmux` sessions.

The `asciiville` command can be used to invoke:

* The lightweight character based system monitor, `btop`
* The lightweight character based web browser, `lynx`
* The lightweight character based mail client, `neomutt`
* The lightweight character based file manager, `ranger`
* One or more terminal emulators running a command
* A tmux session
* Character based ASCII art
* Any character based client the user wishes to run
* One of several asciimatics animations optionally accompanied by audio

Integration is provided for:

* [btop](btop/README.md), character based system monitor
* [lynx](https://lynx.invisible-island.net/), character based web browser
* [neomutt](https://neomutt.org/), character based mail client
* [ranger](https://ranger.github.io/), character based file manager
* [mplayer](http://mplayerhq.hu/design7/info.html), a media player
* [asciimatics](https://github.com/peterbrittain/asciimatics) - automatically display a variety of character based animation effects
* [asciinema](https://asciinema.org/) - automatically create ascii character based video clips
* [tmux](https://github.com/tmux/tmux/wiki), a terminal multiplexer
* Enhanced key bindings for extended control of terminal windows
* Several terminal emulators
    * xfce4-terminal
    * gnome-terminal
    * tilix
    * cool-retro-term

### Asciiville Commands

Asciiville adds the following commands to your system:

* **asciiville** : primary user interface, invokes terminal emulators, ascii animations, system monitor, mail client, file manager, web browser, and more
* **btop** : character based system monitor
* **ascinit** : one-time initializaton of a user's Asciiville configuration
* **btop-tmux** : runs btop in a tmux session
* **asciisplash** : fun ascii art screens using ASCIImatics animations. Ascii art commands:
    * **asciijulia** : ASCIImatics animated zoom on a Julia Set
    * **asciiplasma** : ASCIImatics animated plasma graphic
    * **asciimpplus** : ASCIImatics animated Music Player Plus splash screen
* **chktermcolor** : checks if a terminal has 24-bit true color support
* **set_xfce_trans** : sets an xfce4-terminal window's transparency level

Additional detail and info can be found in the
[Asciiville Wiki](https://github.com/doctorfree/Asciiville/wiki).

### Main btop system monitor features

* Easy to use, with a game inspired menu system.
* Full mouse support, all buttons with a highlighted key is clickable and mouse scroll works in process list and menu boxes.
* Fast and responsive UI with UP, DOWN keys process selection.
* Function for showing detailed stats for selected process.
* Ability to filter processes.
* Easy switching between sorting options.
* Tree view of processes.
* Send any signal to selected process.
* UI menu for changing all config file options.
* Auto scaling graph for network usage.
* Shows IO activity and speeds for disks
* Battery meter
* Selectable symbols for the graphs
* Custom presets

## Quickstart

* Install the latest Debian or RPM format installation package from the [Asciiville Releases](https://github.com/doctorfree/Asciiville/releases) page
* Run the `ascinit` command (must be done as your normal user, no need for `sudo`)
* See the [online asciiville man page](https://github.com/doctorfree/Asciiville/wiki/asciiville.1) or `man asciiville` for different ways to invoke the `asciiville` command

## Requirements

Asciiville can be installed on Debian or RPM based Linux systems.
It requires:

* tilix (>= 1.9.1)
* xfce4-terminal (>= 0.8.9.1)
* cool-retro-term (>= 1.1.1)
* tmux
* asciimatics
* asciinema
* lynx
* neomutt
* ranger
* mplayer

These dependencies will all be automatically installed if not present
when Asciiville is installed using the Debian or RPM packaging.

## Installation

Asciiville v1.0.0 and later can be installed on Linux systems using
either the Debian packaging format or the Red Hat Package Manager (RPM).

### Debian package installation

Many Linux distributions, most notably Ubuntu and its derivatives, use the
Debian packaging system.

To tell if a Linux system is Debian based it is usually sufficient to
check for the existence of the file `/etc/debian_version` and/or examine the
contents of the file `/etc/os-release`.

To install on a Debian based Linux system, download the latest Debian format
package from the
[Asciiville Releases](https://github.com/doctorfree/Asciiville/releases).

Install the Asciiville package by executing the command

```console
sudo apt install ./Asciiville_<version>-<release>.amd64.deb
```
or
```console
sudo dpkg -i ./Asciiville_<version>-<release>.amd64.deb
```

### RPM Package installation

Red Hat Linux, SUSE Linux, and their derivatives use the RPM packaging
format. RPM based Linux distributions include Fedora, AlmaLinux, CentOS,
openSUSE, OpenMandriva, Mandrake Linux, Red Hat Linux, and Oracle Linux.

To install on an RPM based Linux system, download the latest RPM format
package from the
[Asciiville Releases](https://github.com/doctorfree/Asciiville/releases).

Install the Asciiville package by executing the command

```console
sudo yum localinstall ./Asciiville_<version>-<release>.x86_64.rpm
```
or
```console
sudo rpm -i ./Asciiville_<version>-<release>.x86_64.rpm
```

## Documentation

All Asciiville commands have manual pages. Execute `man <command-name>`
to view the manual page for a command. The `asciiville` frontend is the primary
user interface for Asciiville and the manual page for `asciiville` can be
viewed with the command `man asciiville`. Most commands also have
help/usage messages that can be viewed with the **-u** argument option,
e.g. `asciiville -u`.

### Btop++ README
- [**btop/README.md**](btop/README.md) - Introduction to the btop system monitor

### Man Pages

- [**asciiville**](markdown/asciiville.1.md) : Primary Asciiville user interface
- [**asciijulia**](markdown/asciijulia.1.md) : asciimatics animation of a Julia Set
- [**asciimpplus**](markdown/asciimpplus.1.md) : asciimatics animation of Asciiville intro
- [**asciiplasma**](markdown/asciiplasma.1.md) : asciimatics animation with Plasma effect
- [**mpcinit**](markdown/mpcinit.1.md) : Asciiville initialization
- [**btop-tmux**](markdown/btop-tmux.1.md) : Asciiville in a tmux session
- [**btop**](markdown/btop.1.md) : Asciiville system monitor
- [**asciisplash-tmux**](markdown/asciisplash-tmux.1.md) : Asciiville asciimatics animations in a tmux session
- [**asciisplash**](markdown/asciisplash.1.md) : Asciiville asciimatics animations

### Usage

The usage message for `asciiville` provides a brief
summary of the command line options:

```
Usage: asciiville [-a] [-b] [-c client] [-f] [-g] [-i]
		[-jJ] [-k] [-l] [-m] [-n num] [-N] [-p] [-P script]
		[-r] [-R] [-s song] [-S] [-t] [-T] [-x] [-y] [-z] [-u]
Terminal/Command options:
	-c 'command' indicates use 'command'
	-f indicates fullscreen display
	-g indicates use gnome terminal emulator
	-i indicates start asciiville in interactive mode
	-l indicates use lynx as the default command
	-P script specifies the ASCIImatics script to run in visualizer pane
	-r indicates use retro terminal emulator
	-t indicates use tilix terminal emulator
	-x indicates use xfce4 terminal emulator
	-y indicates use ranger as the default command
	-z indicates use neomutt as the default command
ASCIImatics animation options:
	-a indicates play audio during ASCIImatics display
	-b indicates use backup audio during ASCIImatics display
	-j indicates use Julia Set scenes in ASCIImatics display
	-J indicates Julia Set with several runs using different parameters
	-m indicates use MusicPlayerPlus scenes in ASCIImatics display
	-n num specifies the number of times to cycle ASCIImatics scenes
	-N indicates use alternate comments in Plasma ASCIImatics scenes
	-p indicates use Plasma scenes in ASCIImatics display
	-s song specifies a song to accompany an ASCIImatics animation
		'song' can be the full pathname to an audio file or a
		relative pathname to an audio file in the MPD music library
		or /home/ronnie/Music/
	-S indicates display ASCIImatics splash animation
General options:
	-k indicates kill Asciiville tmux sessions and ASCIImatics scripts
	-R indicates record tmux session with asciinema
	-T indicates use a tmux session for either ASCIImatics or command
	-u displays this usage message and exits

Type 'man asciiville' for detailed usage info on asciiville
Type 'man btop' for detailed usage info on the btop system monitor
Type 'man asciisplash' for detailed usage info on the asciisplash command
```

```
Usage: asciisplash [-A] [-a] [-b] [-C] [-c num] [-d] [-jJ] [-m] [-p] [-s song] [-u]
Where:
	-A indicates use all effects
	-a indicates play audio during ASCIImatics display
	-b indicates use backup audio during ASCIImatics display
	-C indicates use alternate comments in Plasma effect
	-c num specifies the number of times to cycle
	-d indicates enable debug mode
	-j indicates use Julia Set effect
	-J indicates Julia Set with several runs using different parameters
	-m indicates use Asciiville effect
	-p indicates use Plasma effect
	-s song specifies the audio file to play as accompaniment
		'song' can be the full pathname to an audio file or a relative
		pathname to an audio file in the MPD music library or
		$HOME/Music/
	-u displays this usage message and exits
```

### Example client invocations
The `asciiville` command is intended to serve as the primary interface to invoke
the `btop` system monitor and `cava` spectrum visualizer. The `asciiville` command
utilizes several different terminal emulators and can also be used to invoke
any specified system monitor. Some example invocations of `asciiville` follow.

Open the btop client and cava visualizer in fullscreen mode. The client
will open in the xfce4-terminal emulator and the visualizer in gnome-terminal:

`asciiville -f`

Open the btop client and cava visualizer in fullscreen mode using the
tilix terminal emulator and displaying the visualizer using quarter-height:

`asciiville -f -q -t`

Open the cantata MPD graphical client and cava visualizer:

`asciiville -c`

Open the btop client in the cool-retro-term terminal and cava visualizer
in gnome-terminal:

`asciiville -r`

The btop system monitor can be opened directly without using asciiville.
Similarly, the cava spectrum visualizer can be opened directly without asciiville.

`btop` # In one terminal window

`cava`    # In another terminal window

To test the btop lyrics fetchers:

`btop --test-lyrics-fetchers`

## Removal

On Debian based Linux systems where the Asciiville package was installed
using the Asciiville Debian format package, remove the Asciiville
package by executing the command:

```console
    sudo apt remove asciiville
```
or
```console
    sudo dpkg -r asciiville
```

On RPM based Linux systems where the Asciiville package was installed
using the Asciiville RPM format package, remove the Asciiville
package by executing the command:

```console
    sudo yum remove Asciiville
```
or
```console
    sudo rpm -e Asciiville
```

The Asciiville package can be removed by executing the "Uninstall"
script in the Asciiville source directory:

```console
    git clone git@github.com:doctorfree/Asciiville.git
    cd Asciiville
    ./Uninstall
```
