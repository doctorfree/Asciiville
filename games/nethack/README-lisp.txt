The "lisp" window-port patch is originally part of nethack-el by Ryan Yeske and
Shawn Betts, hosted at http://savannah.gnu.org/projects/nethack-el.  It is
released under a modified BSD license.  The license text is attached at the end
of this file.

This port enables playing UnNetHack in Emacs.  To setup, follow these steps:

1. Build UnNetHack with lisp graphics enabled.  This is done by adding the
   `--enable-lisp-graphics' option to the configure script, e.g.:

     ./configure --prefix=$HOME/unnethack \
                 --with-owner=`id -un` \
		 --with-group=`id -gn` \
		 --enable-wizmode=`id -un` \
		 --enable-lisp-graphics

   Then build and install UnNetHack as usual.

2. Install nethack-el.  Please refer to nethack-el's manual for details.

3. In ~/.emacs, add these lines after loading nethack:

     (setq nethack-program "/PATH/TO/unnethack")
     (setenv "NETHACKOPTIONS" "windowtype=lisp")

4. Finally, M-x nethack RET to begin playing.

Good luck, and happy hacking!

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this list of conditions
    and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other materials provided
    with the distribution.
 3. The name of the author may not be used to endorse or promote products derived from this
    software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
