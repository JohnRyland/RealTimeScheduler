###############################################################################################
#
# Real-time Scheduler
# Copyright (c) 2022, John Ryland
# All rights reserved.
#
###############################################################################################

TARGET := real-time_scheduler
BUILD_DIR := build

SOURCES := $(wildcard src/*.cpp)
HEADERS := $(wildcard includes/*.h)

# Get host details
ifneq (,$(findstring Windows,$(OS)))
  UNAME  := Windows
  ARCH   := $(PROCESSOR_ARCHITECTURE)
else
  UNAME  := $(shell uname -s)
  ARCH   := $(shell uname -m)
endif

# Pick driver for host
ifeq ($(UNAME),Darwin)
 DRIVER = macos
 C_OPTS = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-poison-system-directories
 L_OPTS =
else ifeq ($(UNAME),Windows)
 DRIVER = windows
 C_OPTS =
 L_OPTS =
else ifeq ($(UNAME),Linux)
 DRIVER = linux
 C_OPTS = -Wall -Wextra
 L_OPTS = -lrt
 # everything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-poison-system-directories
else
 DRIVER = dos
endif

# Include driver code
HEADERS += $(wildcard include/driver/*.h)
SOURCES += $(wildcard src/drivers/${DRIVER}/*.cpp)
OBJECTS := $(patsubst %.cpp,${BUILD_DIR}/%.o,$(SOURCES))

all: ${BUILD_DIR} ${TARGET}

${TARGET}: ${OBJECTS}
	g++ -g $^ -o $@ ${L_OPTS}

${BUILD_DIR}:
	@mkdir -p $@

${BUILD_DIR}/%.o: %.cpp ${HEADERS}
	@mkdir -p $(dir $@)
	g++ -g -c $< -o $@ -I./src -I./include -I./include/driver --std=c++11 ${C_OPTS}

debug: ${TARGET}
	./${TARGET}

clean:
	rm -rf ${BUILD_DIR} ${TARGET}


