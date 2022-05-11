Current documentation may be found on the project homepage at:

http://nethack-curses.wikia.com/wiki/NetHack_Curses_Interface_Wiki

The "curses" windowport is a new text-based interface for NetHack,
using high-level curses routines to control the display. Currently, it
has been compiled and tested on Linux, Windows, and Mac OS X, but it
should also be portable to a number of other systems, such as other
forms of UNIX, MSDOS, and OS/2.

Features
========

Some features of this interface compared to the traditional tty
interface include:
  * Dynamic window resizing (e.g. maximizing a terminal window)

  * Dynamic configurable placement of status and message windows,
    relative to the map

  * Makes better use of larger and smaller terminal windows, such as a
    multi-line message window if there is space

  * Optional mouse support (click to move player, etc) 

  * Menu text search/selection 

  * Fancier display (e.g. window borders, optional popup dialogs,
    splash screen, and better menus)

  * Improved keyboard navigation of menus

  * "cursesgraphics" option for fancier line-drawing characters for
    drawing the dungeon - this should work on most terminals/platforms

Gameplay
========

Gameplay should be similar to the tty interface for NetHack; the
differences are primarily visual. This windowport supports dymanic
resizing of the terminal window, so you can play with it to see how it
looks best to you during a game. Also, the align_status and
align_message options may be set during the game or in the config file
to place the status and message windows where you like, so you can
experiment to see what arrangement looks best to you.

For menus, in addition to the normal configurable keybindings for menu
navigation described in the Guidebook, you can use the right and left
arrows to to forward or backward one page, respectively, and the home
and end keys to go to the first and last pages, respectively.

Configuration Options
=====================

Some configuration options that are specific to or relevant to the
curses windowport are shown below. Copy any of these that you like to
your nethack configuration file (e.g. .nethackrc for UNIX or
NetHack.cnf for Windows):
#
# Use this if the binary was compiled with multiple window interfaces,
# and curses is not the default
OPTIONS=windowtype:curses
#
# Set this for Windows systems, or for PDCurses for SDL on any system.
# The latter uses a cp437 font, which works with this option
#OPTIONS=IBMgraphics
#
# Set this if IBMgraphics above won't work for your system.  Mutually
# exclusive with the above option, and should work on nearly any
# system.
OPTIONS=cursesgraphics
#
# Optionally specify the alignment of the message and status windows
# relative to the map window.  If not specified, the code will default
# to the locations used in the tty interface: message window on top,
# and status window on bottom.  Placing either of these on the right or
# left really only works well for winder terminal windows.
OPTIONS=align_message:bottom,align_status:right
#
# Use a small popup "window" for short prompts, e.g. "Really save?".
# If this is not set, the message window will be used for these as is
# done for the tty interface.
OPTIONS=popup_dialog
#
# Specify the initial window size for NetHack in units of characters.
# This is supported on PDCurses for SDL as well as PDCurses for
# Windows.
OPTIONS=term_cols:110,term_rows:32
#
# Controls the usage of window borders for the main NetHack windows
# (message, map, and status windows).  A value of 1 forces the borders
# to be drawn, a value of 2 forces them to be off, and a value of 3
# allows the code to decide if they should be drawn based on the size
# of the terminal window.
OPTIONS=windowborders:3
#
# Whether or not to display a fancy ascii-graphics splash screen on
# startup.
OPTIONS=splash_screen
#
# This controls what text attributes are used to highlight any pets.
# Any combination of single letters representing each text attribute
# may be used.  Not all attributes work on all terminals.
#
# Attributes may be one or more of: Bold, Inverse, Underline, blinK,
# iTalic, Rightline, or Leftline.  The default is underline.
OPTIONS=petattr:u
#
# Controls whether color and attributes are used for menu headers,
# window borders, message text, etc.  This is separate from the color
# option, which only controls the color of the map itself.
OPTIONS=guicolor
#
# Use the mouse for moving around.  This option defaults to false.
#
OPTIONS=mouse_support


Known Issues
============

The current list may be found at:

https://sourceforge.net/tracker/?group_id=317683&atid=1336046

Contact
=======

The curses windowport was written by Karl Garrison. I may be contacted
at kgarrison@pobox.com.

