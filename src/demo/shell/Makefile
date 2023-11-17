#
#  Universal Simple Makefile
#  (C) Copyright 2017
#  John Ryland
#
# Description:
#   Simple boilerplate of a Makefile where you don't
#   need to provide anything, it will generate a project
#   file for you which you can customize if you like.
#   The project file will be initially populated with
#   any source files in the current directory. Header
#   file dependancies are then automatically calculated.
#
OBJECTS      = $(patsubst %.cpp, build/.objs/%.o, $(SOURCES))
DEPENDS      = $(patsubst build/.objs/%.o, build/.deps/%.cpp.d, $(OBJECTS))
BASENAME     = $(shell basename $(PWD))
PLATFORM     = $(shell uname -s)
ARCH         = $(shell uname -m)
COMPILER     = $(shell c++ --version | tr [a-z] [A-Z] | grep -o 'CLANG\|GCC' | head -n 1)
COMPILER_VER = $(shell c++ --version | grep -o "[0-9]\.[0-9]" | head -n 1)
PROJECT_FILE = $(BASENAME).pro

-include $(PROJECT_FILE)

all: $(PROJECT_FILE) build/$(TARGET) $(ADDITIONAL_DEPS)
		@grep "TODO" $(SOURCES) *.h

clean:
		rm -rf build

$(PROJECT_FILE):
		@echo "TARGET   = $(BASENAME)" > $@
		@echo "SOURCES  = $(shell ls *.c *.cpp 2> /dev/null)" >> $@
		@echo "CFLAGS   =" >> $@
		@echo "LFLAGS   =" >> $@

build/.deps/%.cpp.d: %.cpp
		@mkdir -p `dirname $@`
		c++ $(CFLAGS) -MT $(patsubst %.cpp, build/.objs/%.o, $<) -MD -E $< -MF $@ > /dev/null

build/.objs/%.o: %.cpp build/.deps/%.cpp.d
		@mkdir -p `dirname $@`
		c++ $(CFLAGS) -c $< -o $@

build/$(TARGET): $(OBJECTS) $(DEPENDS)
		c++ $(LFLAGS) $(OBJECTS) -o $@
		strip -S $@

.PHONY: all clean

-include $(DEPENDS)

debug:
		@echo "BASENAME     = $(BASENAME)"
		@echo "PROJECT_FILE = $(PROJECT_FILE)"
		@echo "PLATFORM     = $(PLATFORM)"
		@echo "ARCH         = $(ARCH)"
		@echo "COMPILER     = $(COMPILER)"
		@echo "VERSION      = $(COMPILER_VER)"

