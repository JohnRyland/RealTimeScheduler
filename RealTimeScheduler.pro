PROJECT  = RealTimeScheduler
TARGET   = RealTimeScheduler

# Pick config for host
ifeq ($(UNAME),Darwin)
 CONFIG    = macos
 OS_DEFINE = _MACOS
else ifeq ($(UNAME),Windows)
 CONFIG    = win32
 OS_DEFINE = _WIN32
else ifeq ($(UNAME),Linux)
 CONFIG    = linux
 LIBRARIES = rt
 OS_DEFINE = _LINUX
else
 CONFIG    = dos
 OS_DEFINE = _MSDOS
endif

print-%:
	@echo $* = $($*)

SOURCES  = $(wildcard src/*.cpp src/demo/*.cpp src/kernel/*.cpp src/runtime/*.cpp src/modules/*.cpp bootloader/i386/src/end.S)
DOCS     = $(wildcard *.md *.txt docs/*)
INCLUDES = configs/${CONFIG} include include/kernel include/module include/runtime
CFLAGS   = -Wall -Wextra -Werror -nostdlib -D$(OS_DEFINE)
CXXFLAGS = -std=c++20
MODULES  = https://github.com/JohnRyland/TestFramework.git

