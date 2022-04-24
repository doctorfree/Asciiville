include ../Makefile.inc

SRC = $(wildcard *.cpp)
OBJS = $(wildcard $(SRC:%.cpp=%.o) $(SRC:%.cpp=%.d))

all: $(SRC:%.cpp=%.o)
	@#

ifeq ($(HAS_SVN), 0)
ifneq ($(SVN_ROOT_DIR),)
SVN_DIR := $(SVN_ROOT_DIR)/.svn/tmp
else
SVN_DIR := $(wildcard $(ROOT_DIR)/.svn/tmp)
endif
endif

buildinfo.o: buildinfo.cpp ../Makefile.inc $(SVN_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $< -DBUILDTIME=$(DATE) -DVERSION=$(VERSION) -DBOXTAG=$(BOXTAG)

datafilehandler.o: datafilehandler.cpp ../Makefile.inc $(SVN_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $< -DDATA_FILE=$(DATA_FILE)

clean:
	@if test $(firstword $(OBJS)); then rm -rf $(OBJS); echo rm -rf $(OBJS); fi

%.o: %.cpp ../Makefile.inc

-include $(SRC:%.cpp=%.d)
