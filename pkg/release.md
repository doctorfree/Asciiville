## Table of contents

1. [Overview](#overview)
1. [Installation](#installation)
    1. [Arch Linux based installation](#arch-linux-based-installation)
    1. [Debian based installation](#debian-based-installation)
    1. [RPM based installation](#rpm-based-installation)
    1. [Manual installation](#manual-installation)
1. [Configuration](#configuration)
1. [Usage](#usage)
1. [Removal](#removal)
1. [Changelog](#changelog)

## Overview

Supported features include:

* Featureful ASCII Art display including slideshow and zoom capabilities
* Character based ASCII Art and image to ascii conversion utility `jp2a`
* The lightweight character based system monitor, `btop`
* The lightweight character based web browser, `w3m`
* The lightweight character based mail client, `neomutt`
* The lightweight character based FTP client, `cbftp`
* The lightweight character based music player, `mpcplus`
* The lightweight character based file manager, `ranger`
* The lightweight character based disk usage analyzer, `gdu`
* The lightweight character based journal app, `jrnl`
* One or more terminal emulators running a command
* A tmux session
* A command line web search
* A zoomable map of the world
* Command line character based Twitter client
* A network download/upload speed test
* The AAlib BB demo running in a tmux session (Debian based systems only)
* The ASCII text-based dungeon game `nethack` with Extended ASCII glyphs
* The `cmatrix` command that displays the screen from "The Matrix"
* Display system info
* Display the Phase of the Moon
* Display a weather report
* Display a Pokemon
* Display the MusicPlayerPlus or RoonCommandLine interactive menus
* Any character based client the user wishes to run
* Several asciimatics animations optionally accompanied by audio

## Installation

Download the [latest Arch, Debian, or RPM package format release](https://github.com/doctorfree/Asciiville/releases) for your platform.

### Arch Linux based installation

Install the package on Arch Linux based systems by executing the command:

```bash
sudo pacman -U ./Asciiville_1.4.1-2-x86_64.pkg.tar.zst
```

### Debian based installation

Install the package on Debian based systems by executing the commands:

```bash
sudo apt update -y
sudo apt install ./Asciiville_1.4.1-2.amd64.deb
```

or, on a Raspberry Pi:

```bash
sudo apt update -y
sudo apt install ./Asciiville_1.4.1-2.armhf.deb
```

### RPM based installation

Install the package on RPM based systems by executing the command
```bash
sudo dnf update -y
sudo dnf localinstall ./Asciiville-1.4.1-2.x86_64.rpm
```

### Manual installation

On systems for which the Arch, Debian, or RPM packages will not suffice, install manually by downloading the `Install-bin.sh` script and either the gzip'd distribution archive or the zip'd distribution archive.  After downloading the installation script and distribution archive, as a user with sudo privilege execute the commands:

```bash
chmod 755 Install-bin.sh
sudo ./Install-bin.sh /path/to/Asciiville_1.4.1-2.<arch>.tgz
or
sudo ./Install-bin.sh /path/to/Asciiville_1.4.1-2.<arch>.zip
```

## Configuration

* REQUIRED: execute the `ascinit` command to initialize *mutt/neomutt*, *tmux*, *ranger*, *rainbowstream*, and install terminal emulators
    * The `ascinit` command should be executed as a normal user with sudo privilege
        * ascinit # (not 'sudo ascinit')
    * Execute `ascinit -c` rather than `ascinit` if no terminal emulators or graphical utilities are desired
* OPTIONAL: authorize the command line Twitter client by executing `rainbowstream`

See the [Asciiville README](https://github.com/doctorfree/Asciiville#readme) for additional configuration info.

## Usage

Execute `man asciiville` to view the *asciiville* manual page. Explore the features and capabilities of *asciiville* by running it in interactive menu mode with the command:

```console
asciiville
```

Asciiville is a suite of character based utilities, art, and animation. As such, it is intended for use in a terminal window from the command line. To view some of the ASCII animation capabilities provided in Asciiville, try the ASCIImatics animations. For example:

```console
asciisplash -a -i
```

## Removal

Removal of the package on Arch Linux based systems can be accomplished by issuing the command:

```bash
sudo pacman -Rs asciiville
```

Removal of the package on Debian based systems can be accomplished by issuing the command:

```bash
sudo apt remove asciiville
```

Removal of the package on RPM based systems can be accomplished by issuing the command:

```bash
sudo dnf remove Asciiville
```

On systems for which the manual installation was performed using the `Install-bin.sh` script, remove Asciiville manually by downloading the `Uninstall-bin.sh` script and, as a user with sudo privilege, execute the commands:

```bash
chmod 755 Uninstall-bin.sh
sudo ./Uninstall-bin.sh
```

Note that manual removal of Asciiville using the `Uninstall-bin.sh` script will not remove any of the dependencies manually installed above. Manual installation and removal of Asciiville is not as robust as packaged installation and removal. Hopefully additional platform packaging will be available in the future. If you would like to assist with this effort, see the ['Contributing' section of the Asciiville README](https://github.com/doctorfree/Asciiville#contributing).

## Changelog

Version 1.4.1 release 2 adds support for:

* Install go in ascinit if not present
* Use Kitty as default terminal emulator if available, if not then use gnome-terminal
* Porting changes to support several modern compiler constraints
* Integration of asciifetch in asciiville menu
* Add signal handling and child process wait to asciifetch to enable kitty remote control
* Add zoom and keepalive to asciifetch
* Add splash screen to interactive startup
* Add show_pokemon command
* Add language and arg processing to weatherfetch, enhance show_weather command
* Add Pokemon display to commands and menu, enhanced weather report using weatherfetch
* Patch cbftp if on Arch to accomodate their unified ncurses implementation
* Use format string to avoid error when -Werror=format-security is enabled
* Fix aewan build in PKGBUILD
* Kitty integration, update Kitty configuration, set Kitty url opener in ascinit
* Add gum install to ascinit
* Add diyfetch examples to tools/bin
* Add Sphinx documentation for Asciiville docs on Read the Docs
* Add installation of rich-cli rich command in ascinit
* Add color display option to show_endo
* Add show_endo to display ascii fluid dynamics simulations
* Add endoh1 fluid dynamics ascii animation and build
* Add Webb telescope images, add tmux plugin manager

Version 1.4.1 release 1 adds support for:

* Zoomable display of ascii art
* Center ascii art in zoom/browse mode
* Turn linewrap off during ascii art display
* Add shuffle and length settings to asciiville config
* Update sample config with character palette settings
* Upgrade to Btop++ 1.2.7
* Fix jrnl default config and journals creation
* Adapt art font size for each file in file list display mode
* Add support for slideshow of files provided on command line
* Add support for slideshow/display of files from list in file
* Add support for centering borderless ascii art
* Add sample irssi config

Version 1.4.0 added support for:

* Manual installation and removal for non-Debian/RPM Linux systems
* Improved Ascii Art display features
* Additional Ascii Art
* Shuffle and random display modes now supported
* Several bug fixes
* Add aewan ascii art creation tools
* Add per-gallery config file support
* Add Vintage ascii art gallery
* Better support for console-only deployments
* Console-based mailcap configs used with `ascinit -c`
* Add sample NNTP account in NeoMutt (use NeoMutt to read Usenet newsgroups)
* Use encrypted credentials for NNTP server authenticaton
* Add preconfigured Cruzio account for NeoMutt
* Mailcap improvements for mutt, neomutt, and tuir
* Use jp2a in mailcaps for images
* Add support for TUIR - Terminal UI for Reddit
* Add khard contact management integration and configuration
* Add support for selecting multiple ascii art files with ranger
* Add ascii art selection to menus
* View individual ascii art via command line
* Compress ascii art files
* Add manual installation script for non Debian/RPM systems
* Beginning with version 1.4.0 a console-only setup is supported
    * No graphical utilities are installed if `ascinit -c` used
    * Console screen used exclusively for display
* Support for xfce4-terminal slideshows with FIFO
* Updated NetHack to latest development snapshot
* Add several new Ascii Art galleries
* Add new text-based games
* Add termprofset command to manage terminal profile settings
* Add `newsboat` RSS Feed reader
* Maintain Asciiville preferences in `$HOME/.config/asciiville/config`
* Move installation of terminal emulators to ascinit
* Add `got` translation tool
* Add `tdraw` ascii drawing tool
* Dynamically generate Art folder menu entries
* Add menu for generating and viewing ascii art
* User generated ASCII Art galleries can be added to menu

Version 1.3.1 included:

* W3M configuration with support for acting as a Markdown pager
* Mailcap and MIME type enhancements for NeoMutt, Mutt, and W3M
* NeoMutt and Mutt configuration in `ascinit`
* Enhanced Mutt/NeoMutt mailcap, auto view text/html MIME type
* Support for GPG encrypted passwords in NeoMutt
* Extended Help menu in `asciiville`
* Colorized `man` command output in Help menus
* Add support and integration for `jrnl` Journal application
* Add Matrix and NetHack commands
* Additional freely licensed songs for slideshow and animation audio tracks
* Rename all Ascii Art files to use `.asc` filename suffix
* Add rifle config for opening .asc files
* Simplify menus, match partial responses when possible
* Create and check initialization file in asciiville
* Add selection menu for command and terminal, add select song option
