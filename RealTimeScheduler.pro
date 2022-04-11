PROJECT  = RealTimeScheduler
TARGET   = RealTimeScheduler

# Pick driver for host
ifeq ($(UNAME),Darwin)
 DRIVER  = macos
else ifeq ($(UNAME),Windows)
 DRIVER  = windows
else ifeq ($(UNAME),Linux)
 DRIVER  = linux
 LIBRARIES = rt
else
 DRIVER  = dos
endif

SOURCES  = $(wildcard src/*.cpp src/drivers/${DRIVER}/*.cpp)
DOCS     = $(wildcard *.md *.txt docs/*)
INCLUDES = include include/driver
CFLAGS   = -Wall -Wextra -Werror
CXXFLAGS = -std=c++11
MODULES  = https://github.com/JohnRyland/TestFramework.git
