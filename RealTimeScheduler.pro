PROJECT = RealTimeScheduler
TARGET = RealTimeScheduler

# Pick driver for host
ifeq ($(UNAME),Darwin)
 DRIVER = macos
 CFLAGS = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-poison-system-directories
else ifeq ($(UNAME),Windows)
 DRIVER = windows
else ifeq ($(UNAME),Linux)
 DRIVER = linux
 CFLAGS = -Wall -Wextra
 LIBRARIES = rt
else
 DRIVER = dos
endif

SOURCES = $(wildcard src/*.cpp src/drivers/${DRIVER}/*.cpp)
DOCS = $(wildcard *.md *.txt docs/*)
INCLUDES = include include/driver
CXXFLAGS = -std=c++11
