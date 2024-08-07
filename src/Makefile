PROGRAM  = push
VERSION  = 0.0.2
SOURCES  = main.c term.c lined.c cli.c parse.c screen.c

export CC65_HOME=../../cc65
export ZCCCFG=../../z88dk/lib/config/
export PATH=$(CC65_HOME)/bin:../../xemu/build/bin:../../zesarux/src:../../kickc/bin:../../z88dk/bin:../../oscar64/bin:$(shell echo $$PATH)

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
DEFINES += -DCC65 -DHAVE_CONIO
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION -DHAVE_OSD
endif

ifeq ($(SDK),kickc)
BIN      = $(TARGET).prg
CFLAGS   = -a -Sc -Si -Wfragment
DEFINES += -DKICKC -DHAVE_CONIO
DEFINES += -DHAVE_HINTS -DHAVE_OSD
endif

ifeq ($(SDK),oscar64)
BIN      = $(PROGRAM)-o64.prg
DEFINES += -DHAVE_CONIO
DEFINES += -DHAVE_HINTS -DHAVE_OSD
DEFINES += -DOSCAR64 -DNOFLOAT -DNOLONG
CFLAGS   = -i=../../oscar64/include/$(MACHINE) -psci
SOURCES += str.c
endif

ifeq ($(SDK),z88dk)
BIN      = $(TARGET)$(COLS)
CFLAGS   = -vn -startup=1 -create-app -I../z88dk/include # --hardware-keyboard
CFLAGS  += -pragma-define:CLIB_EXIT_STACK_SIZE=0
CFLAGS  += -pragma-define:CLIB_CONIO_NATIVE_COLOUR=1
DEFINES += -DZ88DK -DAMALLOC -DHAVE_CONIO -DHAVE_OSD -DHAVE_SWCURSOR
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION
endif

ifeq ($(MACHINE),m65)
DEFINES += -DM65 -DHAVE_PETSCII
SOURCES += mega65.c str.c
endif

ifeq ($(MACHINE),c64)
DEFINES += -DC64 -DHAVE_PETSCII
endif

ifeq ($(MACHINE),atari)
DEFINES += -DATARI -DHAVE_FILEIO
SOURCES += atari.c
endif

ifeq ($(MACHINE),atarixl)
DEFINES += -DATARI -DHAVE_FILEIO
SOURCES += atari.c
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
DEFINES += -DZXN -DHAVE_FILEIO -DHAVE_ESXDOS
CFLAGS  += -lesxdos -O3 -clib=classic
SOURCES += zxnext.c esxdos.c
endif

ifeq ($(SDIEC),1)
DEFINES += -DHAVE_SDIEC -DHAVE_FILEIO
SOURCES += sdiec.c
endif

DEFINES += -DTOOLCHAIN=$(SDK) -DMACHINE=$(MACHINE)

########################################

.SUFFIXES:
.PHONY: all clean

all: $(TARGET)

########################################

kickc:
	kickc.sh -p mega65 $(CFLAGS) $(DEFINES) -o $(BIN) $(SOURCES)

z88dk:
	zcc +$(MACHINE) $(CFLAGS) $(DEFINES) -o $(BIN) $(SOURCES)

oscar64:
	oscar64 -o=$(BIN) -tf=prg -tm=$(MACHINE) $(DEFINES) $(CFLAGS) $(SOURCES)

zx64:
	$(MAKE) SDK=z88dk MACHINE=zx COLS=64 z88dk
	chmod +x $(PROGRAM)-zx64.tap

zx zx32:
	$(MAKE) SDK=z88dk MACHINE=zx COLS=32 z88dk
	chmod +x $(PROGRAM)-zx32.tap

zxn:
	$(MAKE) SDK=z88dk MACHINE=zxn z88dk
	chmod +x $(PROGRAM)-zxn.tap

m65:
	$(MAKE) SDK=kickc MACHINE=m65 kickc
	chmod +x $(PROGRAM)-m65.prg

c65:
	$(MAKE) SDK=cc65 MACHINE=m65
	chmod +x $(PROGRAM)-m65.prg

c64:
	$(MAKE) SDK=cc65 MACHINE=c64 SDIEC=1
	chmod +x $(PROGRAM)-c64.prg

o64:
	$(MAKE) SDK=oscar64 MACHINE=c64 oscar64
	chmod +x $(PROGRAM)-o64.prg

xe:
	$(MAKE) SDK=cc65 MACHINE=atari
	chmod +x $(PROGRAM)-atari.prg

xl:
	$(MAKE) SDK=cc65 MACHINE=atarixl
	chmod +x $(PROGRAM)-atarixl.prg

c64emu: c64
	x64 $(PROGRAM)-c64.prg

o64emu: o64
	x64 $(PROGRAM)-o64.prg

m65emu: m65
	xmega65.native -besure -prg $(PROGRAM)-m65.prg

zx64emu: zx64
	zesarux --romfile ../../48.rom --machine 48k $(PROGRAM)-zx.tap

zxemu: zx
	zesarux --romfile ../../48.rom --machine 48k $(PROGRAM)-zx.tap

zxnemu: zxn
	zesarux --romfile ../../next.rom --machine TBBlue $(PROGRAM)-zxn.tap

xeemu: xe
	atari800 -xe $(PROGRAM)-atari.prg

xlemu: xl
	atari800 -xl $(PROGRAM)-atarixl.prg

########################################

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:.c=.d)
endif

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFINES) -o $@ $<

$(TARGET): $(SOURCES:.c=.o)
	$(CC) $(LDFLAGS) -o $(BIN) $^

clean:
	$(RM) *.o *.d *.map *.mem *.asm *.dbg *.vs *.klog *.bin *.vsf *.int *.lbl push-zx32 push-zx64

distclean: clean
	$(RM) $(BIN) *.prg *.tap push-*
	$(RM) -rf foo
