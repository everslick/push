PROGRAM  = push
VERSION  = 0.0.1
DEFINES  = -DVERSION=\"$(VERSION)\"
SOURCES  = main.c term.c lined.c cli.c

export PATH=../kickc/bin:../z88dk/bin:$(shell echo $$PATH)
export ZCCCFG=../z88dk/lib/config/
export CC65_HOME=../cc65/

ifeq ($(TARGET),m65)
BIN      = $(PROGRAM).prg
DEFINES += -DKICKC -DM65 -DHAVE_CONIO
DEFINES += -DHAVE_HINTS
endif

ifeq ($(TARGET),zx)
BIN      = $(PROGRAM).zx
CFLAGS   = +zx -vn -SO3 -I../z88dk/include/ -clib=ansi
DEFINES += -DZX -DHAVE_CONIO -DSPECTRUM_32COL -DAMALLOC 
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION
SOURCES += zxspec.c
endif

ifeq ($(TARGET),c64)
CC       = cl65
BIN      = $(PROGRAM).prg
CFLAGS   = -t $(TARGET) --create-dep $(<:.c=.d) -O
LDFLAGS  = -t $(TARGET) -m $(PROGRAM).map
DEFINES += -DC64 -DHAVE_CONIO
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION
endif

# linux
ifndef TARGET
CC       = gcc
BIN      = $(PROGRAM)
CFLAGS   = -MMD -MP -O -g3 -Wno-format-security
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION
SOURCES += vt100.c
DEFINES += -DVT100
endif

#zcc +cpm -vn -O3 -clib=new cmdline.c -o cmdline -create-app
#SO3 --max-allocs-per-node200000 -startup=1 -clib=sdcc_iy -vn -DSPECTRUM_32COL

########################################

.SUFFIXES:
.PHONY: all clean
all: $(BIN)

run:
	x64 push.prg

c64:
	$(MAKE) TARGET=c64
	$(MAKE) run

spec:
	zcc $(CFLAGS) $(DEFINES) $(SOURCES) -o $(BIN) -create-app

mega:
	kickc.sh -e -p mega65 $(DEFINES) -DNULL=\(\(void*\)0\) -o $(BIN) main.c

zx:
	$(MAKE) TARGET=zx spec

m65:
	$(MAKE) TARGET=m65 mega

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:.c=.d)
endif

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFINES) -o $@ $<

$(BIN): $(SOURCES:.c=.o)
	$(CC) $(LDFLAGS) -o $(BIN) $^

clean:
	$(RM) *.o *.d *.prg *.map *.mem $(PROGRAM)
