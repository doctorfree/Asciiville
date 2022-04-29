FIG_FONTS = /usr/share/figlet-fonts
FONT = Lean
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

ifeq ($(shell command -v lolcat >/dev/null; echo $$?),0)
	LOL_CMD := lolcat
else
	LOL_CMD := cat
endif
ifeq ($(shell command -v figlet >/dev/null; echo $$?),0)
	FIG_CMD := figlet -c -d $(FIG_FONTS) -f $(FONT) -k -t Asciiville | tr ' _/' ' ()' | $(LOL_CMD)
else
ifeq ($(shell test -f asciiville.txt; echo $$?),0)
	FIG_CMD := cat asciiville.txt | $(LOL_CMD)
else
	FIG_CMD := echo Asciiville
endif
endif

.PHONY: all info deb rpm btop cbftp jp2a clean

all: btop cbftp jp2a deb rpm

info:
	@$(FIG_CMD)

btop: info
	@./build-btop.sh

cbftp: info
	@./build-cbftp.sh

jp2a: info
	@./build-jp2a.sh

deb: info
	@./mkdeb

rpm: info
	@./mkrpm

clean: info
	@./clean
