PROGRAM  = push
VERSION  = 0.0.1
SOURCES  = main.c term.c lined.c cli.c

export PATH=../zesarux/src:../kickc/bin:../z88dk/bin:$(shell echo $$PATH)
export ZCCCFG=../z88dk/lib/config/
export CC65_HOME=../cc65/

DEFINES  = -DVERSION=\"$(VERSION)\"
TARGET   = $(PROGRAM)-$(MACHINE)

# native
ifndef SDK
CC       = gcc
BIN      = $(PROGRAM)
TARGET   = $(PROGRAM)
CFLAGS   = -MMD -MP -O -g3 -Wno-format-security
DEFINES += -DGCC -DLINUX -DHAVE_FILEIO
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION -DHAVE_OSD
SOURCES += condrv.c linux.c
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
CFLAGS   = -vn -startup=1 -create-app -I../z88dk/include
CFLAGS  += -L../z88dk/lib
CFLAGS  += -pragma-define:CLIB_EXIT_STACK_SIZE=0
CFLAGS  += -pragma-define:CLIB_CONIO_NATIVE_COLOUR=1
DEFINES += -DZ88DK -DHAVE_CONIO -DAMALLOC 
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
CFLAGS  += -pragma-redirect:fputc_cons=fputc_cons_native
CFLAGS  += -pragma-redirect=CRT_FONT=_font_8x8_bbc_system
DEFINES += -DZX
CFLAGS  += -lndos
SOURCES += zxspec.c
endif

ifeq ($(MACHINE),zxn)
DEFINES += -DZXN
SOURCES += zxnext.c
endif

ifdef TEST
PROGRAM = test
SOURCES = test.c
SOURCE  = test.c
endif

########################################

.SUFFIXES:
.PHONY: all clean

all: $(TARGET)

########################################

kickc:
	kickc.sh -p $(MACHINE) $(CFLAGS) $(DEFINES) -o $(BIN) $(SOURCES)

z88dk:
	zcc +$(MACHINE) $(CFLAGS) $(DEFINES) -o $(BIN) $(SOURCES)

zx:
	$(MAKE) SDK=z88dk MACHINE=zx z88dk

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

zxemu: clean zx
	zesarux --romfile ../48.rom --machine 48k $(PROGRAM)-zx.tap

zxnemu: clean zxn
	zesarux --romfile ../next.rom --machine TBBlue $(PROGRAM)-zxn.tap

########################################

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:.c=.d)
endif

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFINES) -o $@ $<

$(TARGET): $(SOURCES:.c=.o)
	$(CC) $(LDFLAGS) -o $(BIN) $^

clean:
	$(RM) *.o *.d *.map *.mem *.asm *.dbg *.vs *.klog *.bin

distclean: clean
	$(RM) $(BIN) *.prg *.tap test push-zx
