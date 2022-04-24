-include ../../Makefile.inc

SRC = $(wildcard *.cpp)
LIB = libcore.a
CLEANFILES = $(wildcard $(SRC:%.cpp=%.o) $(SRC:%.cpp=%.d) $(LIB))

all: $(SRC:%.cpp=%.o)
	@+$(MAKE) --no-print-directory $(LIB)

quiet:
	@#

$(LIB): $(SRC:%.cpp=%.o) | quiet
	ar rc $@ $^ && ranlib $@

clean:
	@if test $(firstword $(CLEANFILES)); then rm -rf $(CLEANFILES); echo rm -rf $(CLEANFILES); fi

ifneq ($(wildcard ../../Makefile.inc),)
%.o: %.cpp ../../Makefile.inc

sslmanager.o: sslmanager.cpp ../../Makefile.inc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o sslmanager.o sslmanager.cpp -DCORP=$(BOXTAG)
endif

-include $(SRC:%.cpp=%.d)
