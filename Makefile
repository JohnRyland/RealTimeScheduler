######################################################################
##  Copyright

#  Universal Simple Makefile
#  (C) Copyright 2017-2021
#  John Ryland
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


######################################################################
##  Cross-platform settings

ifneq (,$(findstring Windows,$(OS)))
  UNAME      := Windows
  ARCH       := $(PROCESSOR_ARCHITECTURE)
  TARGET_EXT := .exe
  DEL        := del /q
  RMDIR      := rmdir /s /q
  SEPERATOR  := $(subst /,\,/)
  MKDIR       = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
  GREP        = 
  NULL       := nul
else
  UNAME      := $(shell uname -s)
  ARCH       := $(shell uname -m)
  TARGET_EXT :=
  DEL        := rm 
  RMDIR      := rm -rf
  SEPERATOR  := /
  MKDIR       = mkdir -p $(1)
  GREP        = grep $(1) $(2) || true
  NULL       := /dev/null
endif


######################################################################
##  Compiler, tools and options

CC            = cc
CXX           = c++
LINK          = c++
STRIP         = strip
LINKER        = c++
CTAGS         = ctags
C_FLAGS       = $(CFLAGS) $(DEFINES:%=-D%) $(INCLUDES:%=-I%)
CXX_FLAGS     = $(CXXFLAGS) $(C_FLAGS)
LINK_FLAGS    = $(LFLAGS) $(LIBRARIES:%=-l%)
STRIP_FLAGS   = -S
DEBUG_FLAGS   = -g
RELEASE_FLAGS = -DNDEBUG
OBJECTS       = $(SOURCES:%=$(TEMP_DIR)/release/objs/%.o)
OBJECTS_D     = $(SOURCES:%=$(TEMP_DIR)/debug/objs/%.o)
DEPENDS       = $(OBJECTS:$(TEMP_DIR)/release/objs/%.o=$(TEMP_DIR)/release/deps/%.d)
DEPENDS_D     = $(OBJECTS_D:$(TEMP_DIR)/debug/objs/%.o=$(TEMP_DIR)/debug/deps/%.d)
BASENAME      = $(notdir $(patsubst %/,%,$(abspath ./)))
PLATFORM      = $(UNAME)
COMPILER      = $(shell $(CXX) --version | tr [a-z] [A-Z] | grep -o -i 'CLANG\|GCC' | head -n 1)
COMPILER_VER  = $(shell $(CXX) --version | grep -o "[0-9]*\.[0-9]" | head -n 1)
MAKEFILE_DIR  = $(notdir $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST))))))
PROJECT_FILE  = $(BASENAME).pro

# Allow variables to be expanded again on a second pass
.SECONDEXPANSION:


######################################################################
##  Project

-include $(PROJECT_FILE)


######################################################################
##  Output destinations

TARGET_DIR   = bin
TEMP_DIR     = build
TARGET_BIN   = $(TARGET_DIR)/$(TARGET)$(TARGET_EXT)
TARGET_D_BIN = $(TARGET_DIR)/$(TARGET)_d$(TARGET_EXT)
TAGS         = $(TEMP_DIR)/tags


######################################################################
##  Build rules

all: $(PROJECT_FILE) $(TAGS) $(TARGET_BIN) $(ADDITIONAL_DEPS)
	@$(call GREP,"TODO" $(SOURCES) $(wildcard *.h))

purge:
	$(RMDIR) $(TEMP_DIR) $(TARGET_DIR)

debug: $(TAGS) $(TARGET_D_BIN)
	@echo Running $(TARGET_D_BIN) ...
	@$(TARGET_D_BIN) --debug && echo PASSED

$(PROJECT_FILE):
	@echo PROJECT      = $(BASENAME)> $@
	@echo TARGET       = $(BASENAME)>> $@
	@echo SOURCES      = $(wildcard *.c *.cpp)>> $@
	@echo DOCS         = $(wildcard *.md *.txt *.html)>> $@
	@echo DEFINES      = >> $@
	@echo INCLUDES     = >> $@
	@echo LIBRARIES    = m>> $@
	@echo CFLAGS       = -Wall>> $@
	@echo CXXFLAGS     = -std=c++11>> $@
	@echo LFLAGS       = >> $@

$(TAGS): $(patsubst %, ./%, $(SOURCES) $(wildcard *.h))
	@echo Updating tags
	@$(CTAGS) --tag-relative=yes --c++-kinds=+pl --fields=+iaS --extra=+q --language-force=C++ -f $@ $^ 2> $(NULL)


######################################################################
##  Implicit rules

.SUFFIXES: .cpp .c

$(TEMP_DIR)/release/deps/%.cpp.d: %.cpp
	@$(call MKDIR,$(dir $@))
	@$(CXX) $(CXX_FLAGS) -MT $(patsubst %.cpp, $(TEMP_DIR)/release/objs/%.cpp.o, $<) -MQ dependancies -MQ $(TAGS) -MQ project -MD -E $< -MF $@ > $(NULL)

$(TEMP_DIR)/debug/deps/%.cpp.d: %.cpp
	@$(call MKDIR,$(dir $@))
	@$(CXX) $(CXX_FLAGS) -MT $(patsubst %.cpp, $(TEMP_DIR)/debug/objs/%.cpp.o, $<) -MD -E $< -MF $@ > $(NULL)

$(TEMP_DIR)/release/deps/%.c.d: %.c
	@$(call MKDIR,$(dir $@))
	@$(CC) $(C_FLAGS) -MT $(patsubst %.c, $(TEMP_DIR)/release/objs/%.c.o, $<) -MQ dependancies -MQ $(TAGS) -MQ project -MD -E $< -MF $@ > $(NULL)

$(TEMP_DIR)/debug/deps/%.c.d: %.c
	@$(call MKDIR,$(dir $@))
	@$(CC) $(C_FLAGS) -MT $(patsubst %.c, $(TEMP_DIR)/debug/objs/%.c.o, $<) -MD -E $< -MF $@ > $(NULL)

$(TEMP_DIR)/release/objs/%.cpp.o: %.cpp $(TEMP_DIR)/release/deps/%.cpp.d
	@$(call MKDIR,$(dir $@))
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(TEMP_DIR)/debug/objs/%.cpp.o: %.cpp $(TEMP_DIR)/debug/deps/%.cpp.d
	@$(call MKDIR,$(dir $@))
	$(CXX) $(CXX_FLAGS) -c $< -o $@

$(TEMP_DIR)/release/objs/%.c.o: %.c $(TEMP_DIR)/release/deps/%.c.d
	@$(call MKDIR,$(dir $@))
	$(CC) $(C_FLAGS) -c $< -o $@

$(TEMP_DIR)/debug/objs/%.c.o: %.c $(TEMP_DIR)/debug/deps/%.c.d
	@$(call MKDIR,$(dir $@))
	$(CC) $(C_FLAGS) -c $< -o $@


######################################################################
##  Compile target

$(TARGET_BIN): $(OBJECTS) $(DEPENDS)
	@$(call MKDIR,$(dir $@))
	$(LINKER) $(LINK_FLAGS) $(OBJECTS) -o $@
	$(STRIP) -S $@

$(TARGET_D_BIN): $(OBJECTS_D) $(DEPENDS_D)
	@$(call MKDIR,$(dir $@))
	$(LINKER) $(LINK_FLAGS) $(OBJECTS_D) -o $@

-include $(DEPENDS)
-include $(DEPENDS_D)


######################################################################
##  Vim integrations

define generate_tree_items
	@printf '$(2)'
	$(eval ITEMS := $(3))
	$(eval LAST := $(if $(ITEMS),$(word $(words $(ITEMS)), $(ITEMS)),))
  @printf '$(foreach F, $(ITEMS),\n$(1) $(if $(filter $F,$(LAST)),┗━,┣━) $(notdir $F) \t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t $(abspath $F))\n'
endef

project:
	@printf '$(PROJECT)\n┃\n'
	$(call generate_tree_items,┃ ,┣━ Targets,    $(filter %,$(TARGET)))
	$(call generate_tree_items,┃ ,┃\n┣━ Sources, $(filter %.c,$^) $(filter %.cpp,$^))
	$(call generate_tree_items,┃ ,┃\n┣━ Headers, $(filter-out /%,$(filter %.h,$^) $(filter %.hpp,$^)))
	$(call generate_tree_items,┃ ,┃\n┣━ Docs,    $(filter %.md,$(DOCS)) $(filter %.txt,$(DOCS)) $(filter %.html,$(DOCS)))
	$(call generate_tree_items,  ,┃\n┗━ Project, $(wildcard $(filter-out %.d,$(MAKEFILE_LIST))))

vim_project_support:
	@printf '$(if $(PROJECT),true,false)\n'

# Output the user search paths (for vim/editor integration)
paths:
	@echo $(INCLUDES)

# Output the searched system paths the compiler will use (for vim/editor integration)
system_paths:
	@echo | $(CXX) -Wp,-v -x c++ - -fsyntax-only 2>&1 | grep "^ " | grep -v "(" | tr -d "\n"

dependancies:
	@echo $(patsubst %,\'%\',$^)

lldb-nvim.json: $(PROJECT_FILE)
	@echo ' {' > $@
	@echo '   "variables": { "target": "'$(TARGET_D_BIN)'" },' >> $@
	@echo '   "modes": { "code": {}, "debug": { ' >> $@
	@echo '      "setup": [ "target create {target}", [ "bp", "set" ] ], "teardown": [ [ "bp", "save" ], "target delete" ] ' >> $@
	@echo '   } },' >> $@
	@echo '   "breakpoints": { "@ll": [ ] }' >> $@
	@echo ' }' >> $@


######################################################################
##  Target management

FAKE_TARGETS = debug release profile clean purge verify help all info project paths system_paths dependancies null
MAKE_TARGETS = $(MAKE) -rpn null | sed -n -e '/^$$/ { n ; /^[^ .\#][^ ]*:/ { s/:.*$$// ; p ; } ; }' | grep -v "build/"
REAL_TARGETS = $(MAKE_TARGETS) | sort | uniq | grep -E -v $(shell echo $(FAKE_TARGETS) | sed 's/ /\\|/g')

null:

verify: test-report.txt
	@echo ""
	@echo " Test Results:"
	@echo "   PASS count: "`grep -c "PASS" test-report.txt`
	@echo "   FAIL count: "`grep -c "FAIL" test-report.txt`
	@echo ""

help:
	@echo ""
	@echo " Usage:"
	@echo "   $(MAKE) [target]"
	@echo ""
	@echo " Targets:"
	@echo "   "`$(MAKE_TARGETS)`
	@echo ""

info:
	@echo ""
	@echo " Info:"
	@echo "   BASENAME     = $(BASENAME)"
	@echo "   MAKEFILE_DIR = $(MAKEFILE_DIR)"
	@echo "   PROJECT_FILE = $(PROJECT_FILE)"
	@echo "   PLATFORM     = $(PLATFORM)"
	@echo "   ARCH         = $(ARCH)"
	@echo "   COMPILER     = $(COMPILER)"
	@echo "   VERSION      = $(COMPILER_VER)"
	@echo ""
	@echo " Make targets:"
	@echo "   "`$(MAKE_TARGETS)`
	@echo " Real targets:"
	@echo "   "`$(REAL_TARGETS)`
	@echo " Fake targets:"
	@echo "   $(FAKE_TARGETS)"
	@echo ""

clean:
	$(DEL) $(wildcard $(subst /,$(SEPERATOR),$(TAGS) $(OBJECTS) $(OBJECTS_D) $(DEPENDS) $(DEPENDS_D) $(TARGET_D_BIN) $(TARGET_BIN)))

.PHONY: $(FAKE_TARGETS)

