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
sudo pacman -U ./Asciiville_2.0.0-1-x86_64.pkg.tar.zst
```

### Debian based installation

Install the package on Debian based systems by executing the commands:

```bash
sudo apt update -y
sudo apt install ./Asciiville_2.0.0-1.amd64.deb
```

or, on a Raspberry Pi:

```bash
sudo apt update -y
sudo apt install ./Asciiville_2.0.0-1.armhf.deb
```

### RPM based installation

Install the package on RPM based systems by executing the command
```bash
sudo dnf update -y
sudo dnf localinstall ./Asciiville-2.0.0-1.x86_64.rpm
```

### Manual installation

On systems for which the Arch, Debian, or RPM packages will not suffice, install manually by downloading the `Install-bin.sh` script and either the gzip'd distribution archive or the zip'd distribution archive.  After downloading the installation script and distribution archive, as a user with sudo privilege execute the commands:

```bash
chmod 755 Install-bin.sh
sudo ./Install-bin.sh /path/to/Asciiville_2.0.0-1.<arch>.tgz
or
sudo ./Install-bin.sh /path/to/Asciiville_2.0.0-1.<arch>.zip
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

Version 2.0.0 release 1 adds support for:

* Modularize Asciiville installation and initialization
  * Remove aewan, jp2a, and ninvaders from source, build, and packaging
  * use external build and packaging to install in `ascinit`
* Update btop to 1.2.9
* Install the kitty terminfo entry when installing kitty
* Add any2ascii command and its dependencies
* Add asciibrow command
* Use rich to format usage messages if available
* Add kitty session startup for zsh users
* Add Jekyll theme for Github Pages
* Update tmux configuration
* Add VHS install in ascinit
* Improve newsboat config, additional newsboat scripts
* Use HOME/.config/newsboat for Newsboat config
* Use kitty for image display in Newsboat
* Center ascii art when displaying in Kitty

See [CHANGELOG.md](https://github.com/doctorfree/Asciiville/blob/master/CHANGELOG.md) for a full list of changes in every Asciiville release
