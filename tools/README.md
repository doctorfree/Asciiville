# Asciiville Tools

These scripts and snippets are used in Asciiville development and testing.
They are not necessary for Asciiville users but may prove useful.
They are included here as a convenience and so I don't lose track of them.

This folder and its contents can safely be ignored by Asciiville users.

The scripts in `/usr/share/asciiville/tools/bin/` are a little closer
to usable but still really development/internal tools.

The `bin/show_art` script may prove useful to Asciiville users.
Either copy `/usr/share/asciiville/tools/bin/show_art` to a location
in your execution path, e.g. :

`cp /usr/share/asciiville/tools/bin/show_art $HOME/bin/show_art`

`chmod 755 $HOME/bin/show_art`

or execute `bash /usr/share/asciiville/tools/bin/show_art` to view all of the
ascii art in the current directory. Providing `show_art` with filename
arguments will display only those files:

```console
bash /usr/share/asciiville/tools/bin/show_art
```

or

```console
bash /usr/share/asciiville/tools/bin/show_art /tmp/file1.asc file2.asc.gz
```
