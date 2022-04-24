-include ../../Makefile.inc

SRC = $(wildcard *.cpp)
LIB = libhttp.a
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
endif

-include $(SRC:%.cpp=%.d)
