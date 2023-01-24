ifeq ($(shell uname -s | grep Darwin >/dev/null 2>&1; echo $$?),0)
	FIG_FONTS = /usr/local/share/figlet-fonts
	PREFIX ?= /usr/local
else
	FIG_FONTS = /usr/share/figlet-fonts
	PREFIX ?= /usr
endif
FONT = Lean
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1
DOCDIR ?= $(PREFIX)/share/doc/asciiville
BINS = bin/asciiart bin/asciijulia bin/asciimpplus bin/asciiplasma \
	   bin/asciisplash bin/asciiville bin/ascinit \
	   bin/make_ascii_art bin/set_xfce_trans bin/show_ascii_art
MANS = man/man1/asciiart.1 man/man1/asciijulia.1 man/man1/asciimpplus.1 \
	   man/man1/asciiplasma.1 man/man1/asciisplash-tmux.1 \
	   man/man1/asciisplash.1 man/man1/asciiville.1 man/man1/cbftp.1 \
	   man/man1/show_ascii_art.1

ifeq ($(shell command -v g++-11 >/dev/null; echo $$?),0)
	CXX := g++-11
else ifeq ($(shell command -v g++11 >/dev/null; echo $$?),0)
	CXX := g++11
else ifeq ($(shell command -v g++ >/dev/null; echo $$?),0)
	CXX := g++
endif

override CXX_VERSION := $(shell $(CXX) -dumpfullversion -dumpversion || echo 0)

ifeq ($(CXX),g++)
	ifeq ($(shell g++ --version | grep clang >/dev/null 2>&1; echo $$?),0)
		V_MAJOR := 0
	else
		V_MAJOR := $(shell echo $(CXX_VERSION) | cut -f1 -d".")
	endif
	ifneq ($(shell test $(V_MAJOR) -ge 11; echo $$?),0)
		ifeq ($(shell command -v g++-11 >/dev/null; echo $$?),0)
			override CXX := g++-11
			override CXX_VERSION := $(shell $(CXX) -dumpfullversion -dumpversion || echo 0)
		endif
	endif
endif

PLATFORM ?= $(shell uname -s || echo unknown)
ifneq ($(filter unknown Darwin, $(PLATFORM)),)
	override PLATFORM := $(shell $(CXX) -dumpmachine | awk -F"-" '{ print (NF==4) ? $$3 : $$2 }')
	ifeq ($(PLATFORM),apple)
		override PLATFORM := macos
	endif
endif

ifeq ($(shell uname -v | grep ARM64 >/dev/null 2>&1; echo $$?),0)
	ARCH ?= arm64
else
	ARCH ?= $(shell $(CXX) -dumpmachine | cut -d "-" -f 1)
endif

ifeq ($(shell command -v lolcat >/dev/null; echo $$?),0)
	LOL_CMD := lolcat
else
	LOL_CMD := cat
endif

ifeq ($(shell command -v figlet >/dev/null; echo $$?),0)
	FIG_CMD := figlet -c -d $(FIG_FONTS) -f $(FONT) -k -t Asciiville | tr ' _/' ' ()' | $(LOL_CMD)
else
ifeq ($(shell test -f asciiville.asc; echo $$?),0)
	FIG_CMD := cat asciiville.asc | $(LOL_CMD)
else
	FIG_CMD := echo Asciiville
endif
endif

.PHONY: all info aur deb rpm cbftp nethack clean

all: cbftp nethack aur deb rpm

info:
	@$(FIG_CMD)
	@printf "\033[1;92mPLATFORM   \033[1;93m?| \033[0m$(PLATFORM)\n"
	@printf "\033[1;96mARCH       \033[1;93m?| \033[0m$(ARCH)\n"
	@printf "\033[1;93mCXX        \033[1;93m?| \033[0m$(CXX) \033[1;93m(\033[97m$(CXX_VERSION)\033[93m)\n"

cbftp: info
	@./build cbftp

nethack: info
	@./build nethack

deb: info
	@./mkpkg

rpm: info
	@./mkpkg

aur: info
	@./mkpkg

clean: info
	@./clean
