This release adds support for:

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
sudo apt install ./Asciiville_1.3.1-2.amd64.deb
```

or, on a Raspberry Pi:

```bash
sudo apt update -y
sudo apt install ./Asciiville_1.3.1-2.armhf.deb
```

Install the package on RPM based systems by executing the command
```bash
sudo dnf update -y
sudo dnf localinstall ./Asciiville-1.3.1-2.x86_64.rpm
```

## Configuration

* REQUIRED: execute the `ascinit` command to initialize *mutt/neomutt*, *tmux*, *ranger*, and *rainbowstream*
* OPTIONAL: authorize the command line Twitter client by executing `rainbowstream`

See the [Asciiville README](https://github.com/doctorfree/Asciiville#readme) for additional configuration info.

## Usage

Execute `man asciiville` to view the *asciiville* manual page. Explore the features and capabilities of *asciiville* by running it in interactive menu mode with the command `asciiville -i`.

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
