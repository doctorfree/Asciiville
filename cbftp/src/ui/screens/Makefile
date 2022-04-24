include ../../../Makefile.inc

SRC = $(wildcard *.cpp)
CLEANFILES = $(wildcard $(SRC:%.cpp=%.o) $(SRC:%.cpp=%.d))

all: $(SRC:%.cpp=%.o)
	@#

clean:
	@if test $(firstword $(CLEANFILES)); then rm -rf $(CLEANFILES); echo rm -rf $(CLEANFILES); fi

%.o: %.cpp ../../../Makefile.inc
ifneq ($(NO_NCURSESW),)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $< -DNO_NCURSESW
else
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c -o $@ $<
endif

-include $(SRC:%.cpp=%.d)
