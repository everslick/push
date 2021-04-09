PROGRAM  = push
VERSION  = 0.0.2
SOURCES  = main.c term.c lined.c cli.c parse.c

export PATH=../../zesarux/src:../../kickc/bin:../../z88dk/bin:$(shell echo $$PATH)
export ZCCCFG=../../z88dk/lib/config/
export CC65_HOME=../../cc65/

DEFINES  = -DVERSION=$(VERSION)
TARGET   = $(PROGRAM)-$(MACHINE)

# native
ifndef SDK
CC       = gcc
SDK      = $(CC)
BIN      = $(PROGRAM)
TARGET   = $(PROGRAM)
MACHINE  = posix
CFLAGS   = -MMD -MP -O -g3 -Wno-format-security
DEFINES += -DGCC -DPOSIX -DHAVE_FILEIO
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION -DHAVE_OSD
SOURCES += posix.c
endif

ifeq ($(SDK),cc65)
CC       = cl65
BIN      = $(TARGET).prg
CFLAGS   = -t $(MACHINE) --create-dep $(<:.c=.d) -O
LDFLAGS  = -t $(MACHINE) -m $(TARGET).map
DEFINES += -DCC65 -DHAVE_CONIO -DHAVE_PETSCII
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION -DHAVE_OSD
endif

ifeq ($(SDK),kickc)
BIN      = $(TARGET).prg
CFLAGS   = -a -Sc -Si -Wfragment
DEFINES += -DNULL=\(\(void*\)0\)
DEFINES += -DKICKC -DHAVE_CONIO -DHAVE_PETSCII
DEFINES += -DHAVE_HINTS -DHAVE_OSD
endif

ifeq ($(SDK),z88dk)
BIN      = $(TARGET)
CFLAGS   = -vn -startup=1 -create-app -I../z88dk/include # --hardware-keyboard
CFLAGS  += -pragma-define:CLIB_EXIT_STACK_SIZE=0
CFLAGS  += -pragma-define:CLIB_CONIO_NATIVE_COLOUR=1
DEFINES += -DZ88DK -DAMALLOC -DHAVE_CONIO -DHAVE_OSD -DHAVE_SWCURSOR
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION
endif

ifeq ($(MACHINE),c64)
DEFINES += -DC64 -DHAVE_FILEIO -DHAVE_SDIEC
SOURCES += sdiec.c
endif

ifeq ($(MACHINE),m65)
DEFINES += -DM65
endif

ifeq ($(MACHINE),zx)
CFLAGS  += -pragma-define:ansicolumns=$(COLS)
CFLAGS  += -pragma-need=ansiterminal
CFLAGS  += -pragma-export:__zx_console_attr=0x5C8d
CFLAGS  += -pragma-export:__zx_mode0_console_w=0
DEFINES += -DZX -DCOLUMNS=$(COLS) -DHAVE_FILEIO -DHAVE_DIVMMC
CFLAGS  += -lesxdos -O3
SOURCES += zxspec.c divmmc.c
endif

ifeq ($(MACHINE),zxn)
DEFINES += -DZXN
CFLAGS  += -lesxdos -O3 -clib=classic
SOURCES += zxnext.c
endif

ifdef TEST
PROGRAM = test
SOURCES = test.c
SOURCE  = test.c
endif

DEFINES += -DTOOLCHAIN=$(SDK) -DMACHINE=$(MACHINE)

########################################

.SUFFIXES:
.PHONY: all clean

all: $(TARGET)

########################################

kickc:
	kickc.sh -p $(MACHINE) $(CFLAGS) $(DEFINES) -o $(BIN) $(SOURCES)

z88dk:
	zcc +$(MACHINE) $(CFLAGS) $(DEFINES) -o $(BIN) $(SOURCES)

zx64:
	$(MAKE) SDK=z88dk MACHINE=zx COLS=64 z88dk

zx:
	$(MAKE) SDK=z88dk MACHINE=zx COLS=32 z88dk

zxn:
	$(MAKE) SDK=z88dk MACHINE=zxn z88dk

m65:
	$(MAKE) SDK=kickc MACHINE=m65 kickc

c65:
	$(MAKE) SDK=cc65 MACHINE=m65

c64:
	$(MAKE) SDK=cc65 MACHINE=c64

c64emu: clean c64
	x64 $(PROGRAM)-c64.prg

m65emu: clean m65
	xmega65 -besure -prg main.prg

zx64emu: clean zx64
	zesarux --romfile ../../48.rom --machine 48k $(PROGRAM)-zx.tap

zxemu: clean zx
	zesarux --romfile ../../48.rom --machine 48k $(PROGRAM)-zx.tap

zxnemu: clean zxn
	zesarux --romfile ../../next.rom --machine TBBlue $(PROGRAM)-zxn.tap

########################################

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:.c=.d)
endif

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFINES) -o $@ $<

$(TARGET): $(SOURCES:.c=.o)
	$(CC) $(LDFLAGS) -o $(BIN) $^

clean:
	$(RM) *.o *.d *.map *.mem *.asm *.dbg *.vs *.klog *.bin *.vsf

distclean: clean
	$(RM) $(BIN) *.prg *.tap test push-zx
