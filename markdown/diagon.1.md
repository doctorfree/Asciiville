---
title: DIAGON
section: 1
header: User Manual
footer: diagon 1.0.127
date: May 07, 2022
---
# NAME
diagon - Generate ascii art diagram.

# SYNOPSIS
**diagon** [options] (translator) [translator-options] 

# DESCRIPTION
Diagon is an interactive interpreter. It transforms markdown-style expression
into an ascii-art representation. It is written in C++ and use WebAssembly,
HTML and CSS to make a Web Application. We also expose a command line interface
for personal and 3rd party tools.
  
# COMMAND LINE OPTIONS
**-h, --help**
:    Print this page.

**-v, --version**
: Print the version.

**-l, --list**
:    List the available translators.

# TRANSLATORS
 - Math
 - Sequence
 - Tree
 - Table
 - Grammar
 - Frame
 - GraphDAG
 - GraphPlanar
 - Flowchart

# TRANSLATOR-OPTIONS:

**--help**
: Display translator specific help. For example:

diagon Math --help

diagon Table --help

**-- <input>**
: Read the input from the command line. Without this option, it is read from the standard input.

**-option=value**
: Provide a translator specific option.

# COOKBOOK
Reading from:

* command_line:
    * diagon Math -- 1+1/2
* file:
    * diagon Math < filename
* stdin:
    * diagon Math

* Providing options:
    * diagon Math -style=Unicode -- 1 + 1/2
    * diagon Math -style=Ascii   -- 1 + 1/2
    * diagon Math -style=Latex   -- 1 + 1/2
  
# WEBSITE
  This tool can also be used as a WebAssembly application on the website:

  https://arthursonzogni.com/Diagon/

# EXAMPLES
**diagon Math -- "1+1/2 + sum(i,0,10) = 112/2"**
```
            10         
          ___        
      1   ╲       112
  1 + ─ + ╱   i = ───
      2   ‾‾‾      2 
           0         
```

# AUTHORS
Written by Arthur Sonzogni

Modifications by Ronald Record github@ronrecord.com

# LICENSING
DIAGON is distributed under an Open Source license.
See the file LICENSE in the DIAGON source distribution
for information on terms &amp; conditions for accessing and
otherwise using DIAGON and for a DISCLAIMER OF ALL WARRANTIES.

# BUGS
Submit bug reports online at:

https://github.com/doctorfree/Asciiville/issues

# SEE ALSO
**asciimpplus**(1), **asciiplasma**(1), **asciisplash**(1), **asciisplash-tmux**(1), **asciiville**(1)

Full documentation and sources at:

https://github.com/doctorfree/Asciiville

