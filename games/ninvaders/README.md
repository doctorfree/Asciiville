nInvaders
=========

(C)opyleft 2k2 by dettus
dettus@matrixx-bielefeld.de

Hello and welcome to nInvaders! Right now you are earth's only hope to save
it from alien invaders from space. Make your job, and make it good, mankind
believes in you...

COMPILING
---------

Install dependencies (here Ubuntu example):

```
apt-get install cmake libncurses-dev
```

Now you ready to build:
```
cmake -B cmake_build
cmake --build cmake_build -j2
```

Compiled binary `ninvaders` will be in `cmake_build` directory.

CONTROL KEYS
------------

Controlling nInvaders is really simple. Just press the cursor left/right keys 
to move it left or right, and press space to fire. The esc-key lets you quit 
at any time you want.

COMMAND LINE OPTIONS
--------------------

nInvaders accepts the following command line options
-l x  where x is your skill as a number between 0 (nightmare) and 9 (may i
      play daddy)
-gpl prints out the license information.

Any other option gives you a help screen.

LICENSING
---------
nInvaders is protected under the laws of the GPL and other countries. Any re-
distribution, reselling or copying is strictly allowed. You should have received
a copy of it with this package, if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
Or just start nInvaders with the -gpl option to learn more.

GREETZ
------

hello mom, hello dad, hello katja, hello there, hello elitza, hello wizzard, 
hello walli, hello melanie, hello judith, hello you folks at freshmeat, hello
skyper, hello gamma, hello sunfreeware, hello abuse, hello linus, hello alan,
hello herr von boedefeld, hello god, hello world.
and thank you Mike Saarna for adding ANIMATION!
