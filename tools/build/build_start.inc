######################################################################
##  Copyright

#  Universal Simple Makefile
#  (C) Copyright 2017-2023, by John Ryland.
#  All rights reserved.
#  
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  
#  1. Redistributions of source code must retain the above copyright notice, this
#     list of conditions and the following disclaimer. 
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#  
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
#  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


######################################################################
##  Description

#  Simple boilerplate of a Makefile where you don't need to provide anything,
#  it will generate a project file for you which you can customize if you like.
#
#  The project file will be initially populated with any source files in the
#  current directory. Header file dependancies are calculated automatically.
#
#  This is getting less simple now. It did avoid recursive make, however
#  it now does some recursion, mainly to gather lists of sources, but mostly
#  building from the top level make. The recursion can make it complicated
#  to understand what is happening.

######################################################################
##  Cross-platform settings

# Possible detections:
#  Windows:
#   uname -s => Windows_NT                  - From cmd.exe  (uname from scoop)
#   uname -s => MINGW64_NT-10.0-19044       - From git-bash.exe  (uname from msys)
#   uname -m => x86_64                      - From either
#   echo %OS% => Windows_NT                 - From either
#   echo %PROCESSOR_ARCHITECTURE% => AMD64  - Might depend
#  macOS:
#   $(UNAME) => Mac        - From make.exe
#   uname -s => Darwin     - From sh

UNAME = $(shell uname -s)

# Detect the platform
ifeq ($(OS),Windows_NT)
  PLATFORM   := Windows
  # TODO: detect if we have uname or not
else
  PLATFORM   := $(UNAME)
endif

# The version of make being used even when launched from cmd.exe is not acting 
# as though the host is windows, so not sure if need to keep this logic to
# branch on OS containing Windows when we need to change MKDIR, NULL etc.
# - Seems to depend on which make is called. There is mingw's make, and there
#   is another in scoop\shims\make.exe
# May need to test both
# - actually there are 4 combos to try. with the mingw make + mingw uname,
#   with scoop make + mingw uname, etc
#ifeq ($(UNAME),Windows_NT)
ifeq ($(UNAME),NoMingwToolsInstalled)
  ARCH       := $(PROCESSOR_ARCHITECTURE)
  DEL        := del /q
  RMDIR      := rmdir /s /q
  MKDIR       = IF NOT EXIST $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
  GREP        = 
  NULL       := nul
  WHICH       = $(shell where $(1) 2> nul)
else
  ARCH       ?= $(shell uname -m)
  DEL        := rm 
  RMDIR      := rm -rf
  MKDIR       = mkdir -p $(1)
  GREP        = grep $(1) $(2) || true
  WHICH       = $(shell which $(1))
  NULL       := /dev/null
endif

# PLATFORM and ARCH are the host values
# TARGET_PLATFORM and TARGET_ARCH are the target's values

# Assume not cross-building as the default
TARGET_PLATFORM  = $(PLATFORM)
TARGET_ARCH      = $(ARCH)
TARGET_EXT      :=

ifeq ($(PLATFORM),Darwin)
	OPEN     := open $(1)
endif
ifeq ($(PLATFORM),Linux)
	OPEN     := xdg-open $(1)
endif
ifeq ($(PLATFORM),Windows)
  TARGET_EXT := .exe
	#OPEN     := start $(1)
	OPEN     := cmd /c start $(1)
endif


######################################################################
##  Compiler, tools and options

CC           ?= gcc
CXX          ?= c++
STRIP        ?= strip
LINKER       ?= c++
CTAGS        ?= ctags
PANDOC       ?= pandoc
PANDOC_FLAGS  = -f markdown --template $(PANDOC_TEMPLATE) --resource-path=$(GENMAKE_DIR)pandoc
PANDOC_TEMPLATE = $(GENMAKE_DIR)pandoc/template.tex
DOXYGEN       = doxygen
GCOVR         = gcovr

ALL_INCLUDES  ?= $(sort $(shell $(MAKE) -s MAKE_FLAGS=-s FULL_SOURCES_IN= DEPENDS1= DEPENDS= DIRECT_SRCS= ALL_INCLUDES= $(PASS_THRU_ARGS) includes exported_includes))
ALL_INCLUDES_ := $(ALL_INCLUDES)
C_FLAGS       = $(CFLAGS) $(BUILD_TYPE_CFLAGS) $(DEFINES:%=-D%) $(ALL_INCLUDES_:%=-I%)

CXX_FLAGS     = $(CXXFLAGS) $(C_FLAGS)
LINK_FLAGS    = $(LFLAGS) $(BUILD_TYPE_LFLAGS)
LINK_LIBS     = $(LIBRARIES:%=-l%) $(LIBS)
STRIP_FLAGS   = -S
# MAKE_FLAGS    ?= -s

######################################################################
##  Project directories and files

# $(info Updating $(PROJECT_FILE) deps)

# adding FOR_FILE=path/filename.ext will build the target relative to the path. So if filename.ext is inside
# of an sub-project, and the target is 'debug', then it will build and run just the sub-project.

# BASE_DIR is set for sub-project builds and is the relative path to the sub-project
# output directories are prefixed with the sub-project paths to avoid collisions and for distinct intermediate targets
BASE_DIR     ?= $(if $(FOR_FILE),$(patsubst $(CURRENT_DIR)/%,%/,$(abspath $(dir $(FOR_FILE)))))
CUR_DIR       = $(realpath .)
OBJS_DIR      = $(OUTPUT_DIR)/objs
DEPS_DIR      = $(OUTPUT_DIR)/deps
DOCS_DIR      = $(OUTPUT_DIR)/docs

FULL_SOURCES_IN ?= $(shell $(MAKE) $(MAKE_FLAGS) ALL_SOURCES= DEPENDS= "ALL_INCLUDES=$(ALL_INCLUDES_)" FULL_SOURCES_IN= $(PASS_THRU_ARGS) sources exported_sources)
FULL_SOURCES_ABS_IN := $(sort $(abspath $(FULL_SOURCES_IN)))
ALL_SOURCES   ?= $(sort $(shell $(MAKE) $(MAKE_FLAGS) DEPENDS= DEPENDS1= ALL_SOURCES= FULL_SOURCES_IN= "ALL_INCLUDES=$(ALL_INCLUDES_)" $(PASS_THRU_ARGS) direct_sources exported_sources))
FULL_SOURCES   := $(FULL_SOURCES_ABS_IN)
ALL_SOURCES_CP := $(ALL_SOURCES)
ABS_SOURCES   = $(sort $(abspath $(ALL_SOURCES_CP)))
REL_SOURCES   = $(ABS_SOURCES:$(CUR_DIR)/%=%)
DIRECT_SRCS   ?= $(SOURCES:%=$(BASE_DIR)%)

CODE          = $(filter %.c %.cpp %.S,$(REL_SOURCES))
OBJECTS       = $(CODE:%=$(OBJS_DIR)/%.o)
DEPENDS       ?= $(OBJECTS:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)

CODE1          = $(filter %.c %.cpp %.S,$(DIRECT_SRCS))
OBJECTS1       = $(CODE1:%=$(OBJS_DIR)/%.o)
DEPENDS1      ?= $(OBJECTS1:$(OBJS_DIR)/%.o=$(DEPS_DIR)/%.d)

# ALL includes .pro and Makefile files, full is recursive and includes .pro and Makefiles
FULL_CODE     = $(filter %.h %.hpp %.c %.cpp %.S,$(sort $(FULL_SOURCES:$(CUR_DIR)/%=%)))

SUBDIRS       = $(patsubst %/Makefile,%/subdir_target,$(filter %/Makefile,$(SOURCES:%=$(BASE_DIR)%)))
SUBPROJECTS   = $(patsubst %.pro,%.subproject_target,$(filter %.pro,$(SOURCES:%=$(BASE_DIR)%)))
PDFS          = $(patsubst %.md,$(DOCS_DIR)/%.pdf,$(DOCS))
CURRENT_DIR   = $(patsubst %/,%,$(abspath ./))
BASENAME      = $(notdir $(CURRENT_DIR))
COMPILER      = $(shell $(CXX) --version | tr [a-z] [A-Z] | grep -o -i 'CLANG\|GCC' | head -n 1)
COMPILER_VER  = $(shell $(CXX) --version | grep -o "[0-9]*\.[0-9]" | head -n 1)
MAKEFILE      = $(abspath $(firstword $(MAKEFILE_LIST)))
MAKEFILE_DIR  = $(notdir $(patsubst %/,%,$(dir $(MAKEFILE))))
GENMAKE_DIR  := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
SUB_BASENAME  = $(BASE_DIR)$(notdir $(BASE_DIR:%/=%))
SUB_PROJECT_FILE = $(if $(wildcard $(SUB_BASENAME).pro),$(SUB_BASENAME).pro,$(firstword $(wildcard $(BASE_DIR)*.pro) $(SUB_BASENAME).pro))
PROJECT_FILE ?= $(if $(FOR_FILE),$(SUB_PROJECT_FILE),$(if $(wildcard $(BASENAME).pro),$(BASENAME).pro,$(firstword $(wildcard *.pro) $(BASENAME).pro)))

VERSION       = 0.0.0
PACKAGE_NAME  = $(PROJECT)-$(VERSION)-$(TARGET_PLATFORM)-$(TARGET_ARCH)-$(BUILD_TYPE).zip
OUTPUT_FILE   = $(OUTPUT_DIR)/$(OUTPUT)


######################################################################
##  Build type options (overridden for different build types, defaults to release)

BUILD_TYPE        = release
BUILD_TYPE_CFLAGS = -DNDEBUG
BUILD_TYPE_LFLAGS =
BUILD_TYPE_SUFFIX =
PASS_THRU_ARGS2   = UNAME=$(UNAME) ARCH=$(ARCH) BUILD_TYPE=$(BUILD_TYPE) BUILD_TYPE_CFLAGS="$(BUILD_TYPE_CFLAGS)" BUILD_TYPE_LFLAGS="$(BUILD_TYPE_LFLAGS)" BUILD_TYPE_SUFFIX=$(BUILD_TYPE_SUFFIX)
PASS_THRU_ARGS    = PROJECT_FILE=$(PROJECT_FILE) BASE_DIR=$(BASE_DIR) $(PASS_THRU_ARGS2)

all: release


######################################################################
##  Output destinations

TARGET_DIR   = bin
TEMP_DIR     = .build
OUTPUT_DIR  := $(TEMP_DIR)/$(PLATFORM)-$(BUILD_TYPE)
MODULES_DIR  = .modules
TARGET_BIN   = $(TARGET_DIR)/$(TARGET)$(BUILD_TYPE_SUFFIX)$(TARGET_EXT)
TAGS         = $(TEMP_DIR)/tags
TEST_REPORT  = $(OUTPUT_DIR)/Testing/$(BASE_DIR)test-report.txt
TEST_XML_DIR = $(OUTPUT_DIR)/Testing/$(BASE_DIR)/xml
# E is variable used for preserving leading whitespace when calling $(info)
E:=
