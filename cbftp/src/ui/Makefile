include ../../Makefile.inc

SRC = $(wildcard *.cpp)
SCREENS_SRC = $(wildcard screens/*.cpp)
LIB = libui.a
CLEANFILES = $(wildcard $(SRC:%.cpp=%.o) $(SRC:%.cpp=%.d) $(LIB))

all: $(SRC:%.cpp=%.o) screens
	@+$(MAKE) --no-print-directory $(LIB)

.PHONY: screens
screens:
	@+$(MAKE) -C $@

quiet:
	@#

$(LIB): $(SRC:%.cpp=%.o) $(SCREENS_SRC:%.cpp=%.o) | quiet
	ar rc $@ $^ && ranlib $@

clean:
	@if test $(firstword $(CLEANFILES)); then rm -rf $(CLEANFILES); echo rm -rf $(CLEANFILES); fi
	@+$(MAKE) -C screens $@

%.o: %.cpp ../../Makefile.inc
ifneq ($(NO_NCURSESW),)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $< -DNO_NCURSESW
else
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<
endif

-include $(SRC:%.cpp=%.d)
