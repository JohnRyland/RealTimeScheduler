# Bootstrap makefile to fetch module dependencies (including build system) if don't have them already
# This avoids using git submodules. A couple of reasons, first is submodules are a pain to keep updated.
# Second is that there shouldn't be a requirement to use git. This applies to both the module and the
# parent project, this solution is orthogonal to the versioning system, however we do have a dependency
# on the make command. This ought to still be able to support specifying a version.

MODULES=https://github.com/JohnRyland/GenericMake.git \
				https://github.com/JohnRyland/TestFramework.git

.modules/%.git:
	@echo "Fetching module: $(@:.modules/%.git=%)"
	@git clone $(filter %$(@:.modules/%=%),$(MODULES)) $@ 2> /dev/null

all: $(patsubst %.git,.modules/%.git,$(notdir $(MODULES)))
	@$(MAKE) release

-include .modules/GenericMake.git/Makefile

