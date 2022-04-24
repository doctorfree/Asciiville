# uncomment and edit this line to link with a static openssl
#STATIC_SSL_PATH := /path/to/openssl

# relative path to the ui to use
UI_PATH := src/ui

# path where the data file will be stored
DATA_FILE := "\"~/.cbftp/data\""

UNAME_S := $(shell uname -s)
ifneq ($(STATIC_SSL_PATH),)
STATIC_SSL_INCLUDE := -I$(STATIC_SSL_PATH)/include
STATIC_SSL_LINK := $(STATIC_SSL_PATH)/libssl.a $(STATIC_SSL_PATH)/libcrypto.a
SSL_LINKFLAGS := $(STATIC_SSL_LINK) -ldl
else
SSL_LINKFLAGS := -lssl -lcrypto
endif

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

HAS_SVN := $(shell svn info $(ROOT_DIR) > /dev/null 2>&1; echo $$?)
REDIST_VERSION := $(shell cat $(ROOT_DIR)/.version 2>/dev/null)

ifeq ($(HAS_SVN), 0)
VERSION := "\"svn:r$(shell svn info $(ROOT_DIR)|grep Revision|awk '{ print $$2 }')\""
SVN_ROOT_DIR := $(shell svn info $(ROOT_DIR)|grep 'Working Copy Root'|awk '{print $$5 }')
else ifneq ($(REDIST_VERSION),)
VERSION := "\"redist:r$(REDIST_VERSION)\""
else
VERSION := "\"unknown\""
endif

DATE := "\"$(shell date)\""

BOXTAG := "\"cbftp\""

CXX ?= g++
ifndef OPTFLAGS
OPTFLAGS = -g -O0
else
$(info Using OPTFLAGS provided by shell)
endif
ifndef CXXFLAGS
CXXFLAGS = $(OPTFLAGS) -std=c++11 -Wall -MMD -MP
ifdef STATIC_SSL_INCLUDE
CXXFLAGS += $(STATIC_SSL_INCLUDE)
endif
else
$(info Using CXXFLAGS provided by shell)
endif
ifndef CFLAGS
CFLAGS = $(OPTFLAGS) -Wall -MMD -MP
else
$(info Using CFLAGS provided by shell)
endif
ifndef LINKFLAGS
  LINKFLAGS=$(SSL_LINKFLAGS) -lpthread
  ifneq ($(filter $(UNAME_S),Darwin OpenBSD NetBSD),)
    NO_NCURSESW=1
  endif
  ifneq ($(wildcard /etc/alpine-release),)
    NO_NCURSESW=1
  endif
  ifneq ($(NO_NCURSESW),)
    LINKFLAGS+=-lncurses
  else
    LINKFLAGS+=-lncursesw
  endif
else
  $(info Using LINKFLAGS provided by shell)
endif
