# Changelog

All notable changes to this project will be documented in this file.

# Asciiville-1.4.0r3 (2022-05-30)

* Add aewan ascii art creation tools
* Add per-gallery config file support
* Add Vintage ascii art gallery

# Asciiville-1.4.0r2 (2022-05-24)

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

# Asciiville-1.4.0r1 (2022-05-17)

* Add manual installation script for non Debian/RPM systems
* Support for xfce4-terminal slideshows with FIFO
* Updated NetHack to latest development snapshot
* Add Tetris video game to build and packaging
* Add several new Ascii Art galleries
* Add Ninvaders game
* Add termprofset command to manage terminal profile settings
* Add `newsboat` RSS Feed reader
* Maintain Asciiville preferences in `$HOME/.config/asciiville/config`
* Beginning with version 1.4.0 a console-only setup is supported
    * No graphical utilities are installed if `ascinit -c` used
    * Console screen used exclusively for display
* Move installation of terminal emulators to ascinit
* Add `got` translation tool
* Add `tdraw` ascii drawing tool
* Dynamically generate Art folder menu entries
* User generated ASCII Art galleries can be added to menu
* Add generate ascii art menu entry to art menu
* Use architecture in distribution filenames
* Add menu for generating and viewing ascii art

# Asciiville-1.3.1r2 (2022-05-06)

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

# Asciiville-1.3.1r1 (2022-05-01)

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

# Asciiville-1.3.0r1 (2022-04-20)

Initial release with support for:

* The lightweight character based system monitor, `btop`
* The lightweight character based web browser, `lynx`
* The lightweight character based mail client, `neomutt` or `mutt`
* The lightweight character based file manager, `ranger`
* One or more terminal emulators running a command
* A tmux session
* A command line web search
* Command line character based Twitter client
* A network download/upload speed test
* The AAlib BB demo running in a tmux session (Debian based systems only)
* Character based ASCII art
* Display system info
* Display the Phase of the Moon
* Display a weather report
* Any character based client the user wishes to run
* One of several asciimatics animations optionally accompanied by audio

