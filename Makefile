# The directories containing the source files, separated by ':'
VPATH=src:../Library

ifeq ($(CFG),)
CFG=debug
endif

# The source files: regardless of where they reside in the source tree,
# VPATH will locate them...
Group0_SRC = \
    camera_daemon.c

# Build a Dependency list and an Object list, by replacing the .c
# extension to .d for dependency files, and .o for object files.
Group0_DEP = $(patsubst %.c, deps.$(CFG)/Group0_%.d, ${Group0_SRC})
Group0_OBJ = $(patsubst %.c, objs.$(CFG)/Group0_%.o, ${Group0_SRC})

# Your final binary
TARGET=camera_daemon

# What compiler to use for generating dependencies: 
# it will be invoked with -MM -MP
# CDEP = clang
# CC = clang
CDEP = gcc
CC = gcc

# What include flags to pass to the compiler
INCLUDEFLAGS= -I ../Library -I src

# Separate compile options per configuration
ifeq ($(CFG),debug)
CFLAGS += -g -Wall -D_DEBUG ${INCLUDEFLAGS}
else
CFLAGS += -O2 -Wall ${INCLUDEFLAGS}
endif

# A common link flag for all configurations
LDFLAGS += -lSDL

all:	inform bin.$(CFG)/${TARGET}

inform:
ifneq ($(CFG),release)
ifneq ($(CFG),debug)
	@echo "Invalid configuration "$(CFG)" specified."
	@echo "You must specify a configuration when running make, e.g."
	@echo  "make CFG=debug"
	@echo  
	@echo  "Possible choices for configuration are 'release' and 'debug'"
	@exit 1
endif
endif
	@echo "Configuration "$(CFG)
	@echo "------------------------"

bin.$(CFG)/${TARGET}: ${Group0_OBJ} | inform
	@mkdir -p $(dir $@)
	$(CC) -g -o $@ $^ ${LDFLAGS}

objs.$(CFG)/Group0_%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<

deps.$(CFG)/Group0_%.d: %.c
	@mkdir -p $(dir $@)
	@echo Generating dependencies for $<
	@set -e ; $(CDEP) -MM -MP $(INCLUDEFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,objs.$(CFG)\/Group0_\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	@rm -rf \
	deps.debug objs.debug bin.debug \
	deps.release objs.release bin.release

# Unless "make clean" is called, include the dependency files
# which are auto-generated. Don't fail if they are missing
# (-include), since they will be missing in the first invocation!
ifneq ($(MAKECMDGOALS),clean)
-include ${Group0_DEP}
endif
