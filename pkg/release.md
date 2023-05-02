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

Asciiville provides utilities, tools, applications, games, and scripts to create a rich text-based command line environment enabling the creation of Ascii Art, viewing the included Ascii Art galleries, creating your own custom Ascii Art galleries, and much more. See the [Asciiville README](https://github.com/doctorfree/Asciiville#readme) for an overview of supported features.

Architecture independent native installation packages are provided for Arch Linux, Debian based Linux systems, and RPM based Linux systems. Compressed tar archives and an installation script are provided for all other platforms.

Asciiville 3.0.2 and later have the option to utilize [Homebrew](https://brew.sh) to install packages during `ascinit` post installation initialization. To use Homebrew, invoke `ascinit brew` on Linux platforms. Homebrew is the default on Apple macOS.

## Installation

Download the [latest Arch, Debian, or RPM package format release](https://github.com/doctorfree/Asciiville/releases) for your platform. If your platform does not support Arch, Debian, or RPM format installs (e.g. Apple macOS) then download the compressed binary distribution archive for your platform and the `Install-bin.sh` script.

Installation on macOS must be performed using the [manual installation](#manual-installation) described below.

### Arch Linux based installation

Install the package on Arch Linux based systems by executing the command:

```shell
sudo pacman -U ./Asciiville_3.0.2-1-any.pkg.tar.zst
```

### Debian based installation

Install the package on Debian based systems by executing the commands:

```shell
sudo apt update -y
sudo apt install ./Asciiville_3.0.2-1.deb
```

Similarly, on a Raspberry Pi:

```shell
sudo apt update -y
sudo apt install ./Asciiville_3.0.2-1.deb
```

### RPM based installation

Install the package on RPM based systems by executing the command

```shell
sudo dnf update -y
sudo dnf localinstall ./Asciiville-3.0.2-1.rpm
```

### Manual installation

On systems for which the Arch, Debian, or RPM packages will not suffice, install manually either by cloning the repository or by downloading the `Install-bin.sh` script and the gzip'd distribution archive.

Asciiville can be installed by cloning the repository and executing the `Install` script:

```bash
git clone https://github.com/doctorfree/Asciiville
cd Asciiville
./Install
```

Alternatively, after downloading the installation script and distribution archive, as a user with sudo privilege execute the commands:

```shell
chmod 755 Install-bin.sh
sudo ./Install-bin.sh /path/to/Asciiville_3.0.2-1.<arch>.tgz
```

For example, to install Asciiville version 3.0.2 release 1 on Apple macOS using the installation script and archive previously downloaded to `$HOME/Downloads/`:

```shell
chmod 755 ~/Downloads/Install-bin.sh
sudo ~/Downloads/Install-bin.sh ~/Downloads/Asciiville_3.0.2-1.Darwin.tgz
```

After successfully installing Asciiville, as a normal user run the `ascinit` command to initialize Asciiville. **[Note:]** Run the `ascinit` command as the user who will be using `asciiville`. No `sudo` is needed, just `ascinit`.

## Configuration

- REQUIRED: execute the `ascinit` command to initialize _mutt/neomutt_, _tmux_, _ranger_, _rainbowstream_, and install terminal emulators
  - The `ascinit` command should be executed as a normal user with sudo privilege
    - ascinit # (not 'sudo ascinit')
  - Execute `ascinit -c` rather than `ascinit` if no terminal emulators or graphical utilities are desired
- OPTIONAL: authorize the command line Twitter client by executing `rainbowstream`

See the [Asciiville README](https://github.com/doctorfree/Asciiville#readme) for additional configuration info.

## Usage

Execute `man asciiville` to view the _asciiville_ manual page. Explore the features and capabilities of _asciiville_ by running it in interactive menu mode with the command:

```console
asciiville
```

Asciiville is a suite of character based utilities, art, and animation. As such, it is intended for use in a terminal window from the command line. To view some of the ASCII animation capabilities provided in Asciiville, try the ASCIImatics animations. For example:

```console
asciisplash -a -i
```

## Removal

Removal of the package on Arch Linux based systems can be accomplished by issuing the command:

```shell
sudo pacman -Rs asciiville
```

Removal of the package on Debian based systems can be accomplished by issuing the command:

```shell
sudo apt remove asciiville
```

Removal of the package on RPM based systems can be accomplished by issuing the command:

```shell
sudo dnf remove Asciiville
```

On systems for which the manual installation was performed using the `Install-bin.sh` script, remove Asciiville manually by downloading the `Uninstall-bin.sh` script and, as a user with sudo privilege, execute the commands:

```shell
chmod 755 Uninstall-bin.sh
sudo ./Uninstall-bin.sh
```

Note that manual removal of Asciiville using the `Uninstall-bin.sh` script will not remove any of the dependencies manually installed above. Manual installation and removal of Asciiville is not as robust as packaged installation and removal. Hopefully additional platform packaging will be available in the future. If you would like to assist with this effort, see the ['Contributing' section of the Asciiville README](https://github.com/doctorfree/Asciiville#contributing).

## Changelog

Asciiville Version 3 adds support for:

- Automated Neovim installation and configuration
- Install and use Brew on all platforms
- Automatic installation of many Ascii games
- Improved interactive menu system
- Add `a` to interactive art view mode to auto zoom
- Add `d` to interactive art view mode to set default art font size
- Add `f` keyboard control for display of ascii art filename
- Add `zoom_depth` and `zoom_interval` settings in config and ascii art menu
- Asciiville is now architecture independent
- Many installation improvements on Apple macOS
- Move `btop` to external package install
- Move `cbftp` to external package install
- Move `endoh1` to external package install
- Add `term-image` command - display and browse images in the terminal
- Move installation location back to `/usr` except macOS to `/usr/local`
- Improved support for macOS manual installation
- Moved several package/utility installs from `postinstall` to `ascinit`
- Several bug fixes

See [CHANGELOG.md](https://github.com/doctorfree/Asciiville/blob/master/CHANGELOG.md) for a full list of changes in every Asciiville release
