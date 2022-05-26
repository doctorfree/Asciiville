Version 1.4.0 release 3 adds support for:

* Add aewan ascii art creation tools
* Add per-gallery config file support
* Add Vintage ascii art gallery

Version 1.4.0 release 2 added support for:

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

Version 1.4.0 release 1 added support for:

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

Supported features include:

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
* Character based ASCII Art and image to ascii conversion utility `jp2a`
* Display system info
* Display the Phase of the Moon
* Display a weather report
* Display the MusicPlayerPlus or RoonCommandLine interactive menus
* Any character based client the user wishes to run
* One of several asciimatics animations optionally accompanied by audio

## Installation

Install the package on Debian based systems by executing the commands:

```bash
sudo apt update -y
sudo apt install ./Asciiville_1.4.0-3.amd64.deb
```

or, on a Raspberry Pi:

```bash
sudo apt update -y
sudo apt install ./Asciiville_1.4.0-3.armhf.deb
```

Install the package on RPM based systems by executing the command
```bash
sudo dnf update -y
sudo dnf localinstall ./Asciiville-1.4.0-3.x86_64.rpm
```

On systems for which neither the Debian or RPM packages will suffice,
install manually by downloading the `Install-bin.sh` script and either
the gzip'd distribution archive or the zip'd distribution archive.
After downloading the installation script and distribution archive,
as a user with sudo privilege execute the commands:

```bash
chmod 755 Install-bin.sh
sudo ./Install-bin.sh /path/to/Asciiville_1.4.0-3.<arch>.tgz
or
sudo ./Install-bin.sh /path/to/Asciiville_1.4.0-3.<arch>.zip
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
Removal of the package on Debian based systems can be accomplished by issuing the command:

```bash
sudo apt remove asciiville
```

Removal of the package on RPM based systems can be accomplished by issuing the command:

```bash
sudo dnf remove Asciiville
```

On systems for which the manual installation was performed using
the `Install-bin.sh` script, remove Asciiville manually by downloading
the `Uninstall-bin.sh` script and, as a user with sudo privilege,
execute the commands:

```bash
chmod 755 Uninstall-bin.sh
sudo ./Uninstall-bin.sh
```

Note that manual removal of Asciiville using the `Uninstall-bin.sh` script
will not remove any of the dependencies manually installed above. Manual
installation and removal of Asciiville is not as robust as packaged
installation and removal. Hopefully additional platform packaging will be
available in the future. If you would like to assist with this effort, see the
['Contributing' section of the Asciiville README](https://github.com/doctorfree/Asciiville#contributing).
