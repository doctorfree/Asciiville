This is the initial release of Asciiville

Supported features include:

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
* Several asciimatics animations optionally accompanied by audio

## Installation
Install the package on Debian based systems by executing the command
```bash
sudo apt install ./Asciiville_1.0.0-1.amd64.deb
```

Install the package on RPM based systems by executing the command
```bash
sudo yum localinstall ./Asciiville-1.0.0-1.x86_64.rpm
```

## Configuration

* REQUIRED: execute the `ascinit` command to initialize *tmux*, *ranger*, and *rainbowstream*
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
sudo yum remove Asciiville
```
