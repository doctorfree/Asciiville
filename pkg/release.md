This release includes support for:

* NeoMutt and Mutt configuration in `ascinit`
* Enhanced Mutt/NeoMutt mailcap, auto view text/html MIME type
* Support for GPG encrypted passwords in NeoMutt
* Extended Help menu in `asciiville`
* Add Matrix and NetHack commands
* Additional freely licensed songs for slideshow and animation audio tracks
* Rename all Ascii Art files to use `.asc` filename suffix
* Add rifle config for opening .asc files
* Simplify menus, match partial responses when possible
* Create and check initialization file in asciiville
* Add selection menu for command and terminal, add select song option

Previous releases include support for:

* Create armhf architecture Debian format package for Raspberry Pi installs
* With no arguments asciiville brings up a menu
* Add Mutt configuration to `ascinit`
* Install Asciiville Figlet Fonts in pyfiglet during postinstall
* Add `cbftp` FTP client and support for `ncftp`
* Add `jp2a` image to ascii converter
* Add `show_moon` and `show_weather` commands
* Create cbftp man page
* Add `show_figlet_fonts` command
* Add -d option to indicate use of disk usage analyzer
* Add -f option to indicate use of FTP client
* Add -X option to indicate use of current terminal window
* Change -f to -F option to indicate fullscreen terminal window
* Fixup exit from asciimatics scripts
* MusicPlayerPlus and RoonCommandLine menu integration
* Support for w3m, mapscii, and setting default browser
* Add Figlet Fonts and use Figlet Text for menu headers
* Lolcat install during postinstall
* Removed support for neomutt, just use Mutt
* Add semi-auto-configuration of Mutt
* Add `show_ascii_art` command to display/generate ascii art
* Add dependency on `ImageMagick`
* Add `show_figlet_fonts` command
* Add grayscale option and color depth option to `show_ascii_art`
* Fold my changes to jp2a with newer version
* Added character set option to `show_ascii_art`
* Updated screenshots
* Install btop themes, create btop man page
* Animate lolcat display
* Add Dragonfly, Space, and Doctorwhen ascii art and slideshows
* Add `make_ascii_art` command
* Option to play audio during slideshow
* Create 'Asciiville' profile for Tilix and gnome-terrminal in `ascinit`

Supported features include:

* The lightweight character based system monitor, `btop`
* The lightweight character based web browser, `lynx` or `w3m`
* The lightweight character based mail client, `mutt`
* The lightweight character based file manager, `ranger`
* The lightweight character based music player, `mpcplus`
* One or more terminal emulators running a command
* A tmux session
* A command line web search
* A zoomable map of the world
* Command line character based Twitter client
* A network download/upload speed test
* The AAlib BB demo running in a tmux session (Debian based systems only)
* Display and Generate character based ASCII art
* Convert any image to ascii art
* Display system info
* Display the Phase of the Moon
* Display a weather report
* Display the MusicPlayerPlus or RoonCommandLine interactive menus
* Any character based client the user wishes to run
* Several asciimatics animations optionally accompanied by audio

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
