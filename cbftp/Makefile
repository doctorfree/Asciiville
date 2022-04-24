include Makefile.inc

BINS = cbftp $(BINDIR)/cbftp-debug $(BINDIR)/datafilecat $(BINDIR)/datafilewrite
BINDIR = bin
SRC_TARGETS := src src/core src/http src/ext
SRC = $(wildcard src/*.cpp)
LIBS := $(wildcard $(addsuffix /*.a,$(SRC_TARGETS)))
OBJS = $(wildcard $(SRC:%.cpp=%.o))
DATAFILETOOL_SRC_DEP = src/crypto.cpp src/filesystem.cpp src/path.cpp src/core/util.cpp src/util.cpp

ifneq ($(UI_PATH),)
UI_DEP = $(wildcard $(UI_PATH)/*.a)
ifeq ($(UNAME_S),Darwin)
UI_LINK = -Wl,-force_load $(UI_DEP)
else
UI_LINK = -Wl,--whole-archive $(UI_DEP) -Wl,--no-whole-archive
endif
SRC_TARGETS := $(SRC_TARGETS) $(UI_PATH)
endif

CLEAN_TARGETS = $(addprefix clean-,$(SRC_TARGETS))

.PHONY: $(SRC_TARGETS) $(CLEAN_TARGETS)

all: $(BINS)

$(SRC_TARGETS):
	@+$(MAKE) -C $@

$(BINDIR):
	mkdir -p $@

cbftp: $(SRC_TARGETS) | $(BINDIR)
	@+$(MAKE) --no-print-directory $(BINDIR)/cbftp

docker-build:
	docker build -t cbftp .

docker-run:
	docker run -it -p 55477:55477 --mount 'type=bind,source=$(HOME)/.cbftp,target=/root/.cbftp' cbftp

$(BINDIR)/cbftp: $(OBJS) $(UI_DEP) $(LIBS)
	$(CXX) -o $(BINDIR)/cbftp $(OPTFLAGS) $(SRC:%.cpp=%.o) $(UI_LINK) $(LIBS) $(LINKFLAGS)

$(BINDIR)/cbftp-debug: misc/start_with_gdb.sh | $(BINDIR)
	cp misc/start_with_gdb.sh $@; chmod +x bin/cbftp-debug

$(BINDIR)/datafilecat: $(DATAFILETOOL_SRC_DEP) src/tools/datafilecat.cpp Makefile.inc | $(BINDIR)
	$(CXX) -o $@ $(OPTFLAGS) -std=c++11 -DDATA_FILE=$(DATA_FILE) $(STATIC_SSL_INCLUDE) \
	$(DATAFILETOOL_SRC_DEP) src/tools/datafilecat.cpp $(SSL_LINKFLAGS) -lpthread

$(BINDIR)/datafilewrite: $(DATAFILETOOL_SRC_DEP) src/tools/datafilewrite.cpp Makefile.inc | $(BINDIR)
	$(CXX) -o $@ $(OPTFLAGS) -std=c++11 -DDATA_FILE=$(DATA_FILE) $(STATIC_SSL_INCLUDE) \
	$(DATAFILETOOL_SRC_DEP) src/tools/datafilewrite.cpp $(SSL_LINKFLAGS) -lpthread

linecount:
	find -not -path './src/ext/*'|grep -e '\.h$$' -e '\.cpp$$'|awk '{print $$1}'|xargs wc -l|sort -n

clean: $(CLEAN_TARGETS)
	@if test -d $(BINDIR); then rm -rf $(BINDIR); echo rm -rf $(BINDIR); fi

$(CLEAN_TARGETS):
	@+$(MAKE) -C $(@:clean-%=%) clean
