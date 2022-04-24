-include ../../Makefile.inc

SRC_C = $(wildcard *.c)
SRC_CPP = $(wildcard *.cpp)
OBJS = $(SRC_C:%.c=%.o) $(SRC_CPP:%.cpp=%.o)
LIB = libext.a
CLEANFILES = $(wildcard $(OBJS) $(OBJS:%.o=%.d) $(LIB))

all: $(OBJS)
	@+$(MAKE) --no-print-directory $(LIB)

quiet:
	@#

$(LIB): $(OBJS) | quiet
	ar rc $@ $^ && ranlib $@

clean:
	@if test $(firstword $(CLEANFILES)); then rm -rf $(CLEANFILES); echo rm -rf $(CLEANFILES); fi

ifneq ($(wildcard ../../Makefile.inc),)
$(OBJS): ../../Makefile.inc
endif

#-include $(OBJS:%.o=%.d)
