PREFIX ?= /usr
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1
DOCDIR ?= $(PREFIX)/share/doc/asciiville
BINS = bin/asciiart bin/asciijulia bin/asciimpplus bin/asciiplasma \
	   bin/asciisplash bin/asciisplash-tmux bin/asciiville bin/ascinit \
	   bin/bb-tmux bin/btop-tmux bin/chktermcolor bin/make_ascii_art \
	   bin/set_xfce_trans bin/show_ascii_art bin/show_figlet_fonts
MANS = man/man1/asciiart.1 man/man1/asciijulia.1 man/man1/asciimpplus.1 \
	   man/man1/asciiplasma.1 man/man1/asciisplash-tmux.1 \
	   man/man1/asciisplash.1 man/man1/asciiville.1 man/man1/cbftp.1 \
	   man/man1/show_ascii_art.1

.PHONY: all deb rpm btop cbftp jp2a clean

all: btop cbftp jp2a deb rpm

btop:
	./build-btop.sh

cbftp:
	./build-cbftp.sh

jp2a:
	./build-jp2a.sh

deb:
	./mkdeb

rpm:
	./mkrpm

clean:
	./clean
