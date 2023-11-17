PROJECT  = RealTimeScheduler
TARGET   = RealTimeScheduler

# Pick driver for host
ifeq ($(UNAME),Darwin)
 DRIVER    = macos
 OS_DEFINE = _MACOS
else ifeq ($(UNAME),Windows)
 DRIVER    = windows
 OS_DEFINE = _WIN32
else ifeq ($(UNAME),Linux)
 DRIVER    = linux
 LIBRARIES = rt
 OS_DEFINE = _LINUX
else
 DRIVER    = dos
 OS_DEFINE = _MSDOS
endif

SOURCES  = $(wildcard src/*.cpp src/drivers/${DRIVER}/*.cpp src/drivers/*.cpp bootloader/i386/i386.pro)
DOCS     = $(wildcard *.md *.txt docs/*)
INCLUDES = include include/driver include/runtime
CFLAGS   = -Wall -Wextra -Werror -nostdlib -D$(OS_DEFINE)
CXXFLAGS = -std=c++11
MODULES  = https://github.com/JohnRyland/TestFramework.git
