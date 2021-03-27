PROGRAM  = push
VERSION  = 0.0.1
SOURCES  = main.c term.c lined.c cli.c

export PATH=../kickc/bin:../z88dk/bin:$(shell echo $$PATH)
export ZCCCFG=../z88dk/lib/config/
export CC65_HOME=../cc65/

DEFINES  = -DVERSION=\"$(VERSION)\"
TARGET   = $(PROGRAM)-$(MACHINE)

# nativ
ifndef SDK
CC       = gcc
BIN      = $(PROGRAM)
TARGET   = $(PROGRAM)
CFLAGS   = -MMD -MP -O -g3 -Wno-format-security
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION
SOURCES += vt100.c
DEFINES += -DVT100
endif

ifeq ($(SDK),cc65)
CC       = cl65
BIN      = $(TARGET).prg
CFLAGS   = -t $(MACHINE) --create-dep $(<:.c=.d) -O
LDFLAGS  = -t $(MACHINE) -m $(TARGET).map
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION
endif

ifeq ($(SDK),kickc)
BIN      = $(TARGET).prg
DEFINES += -DNULL=\(\(void*\)0\)
DEFINES += -DKICKC -DHAVE_HINTS
endif

ifeq ($(SDK),z88dk)
BIN      = $(TARGET)
CFLAGS   = -vn -OS3 -O3 -I../z88dk/include/
DEFINES += -DHAVE_CONIO -DAMALLOC 
DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION
SOURCES  = arg-parser.c
endif

ifeq ($(MACHINE),c64)
DEFINES += -DC64 -DHAVE_CONIO
endif

ifeq ($(MACHINE),mega65)
DEFINES += -DM65 -DHAVE_CONIO
endif

ifeq ($(MACHINE),zx)
DEFINES += -DZX
CFLAGS  += -clib=new
#CFLAGS  += -compiler=sccz80
#CFLAGS  += -compiler=sdcc
endif

ifeq ($(MACHINE),zxn)
DEFINES += -DZX
#CFLAGS  += -clib=new
#CFLAGS  += -compiler=sccz80
#CFLAGS  += -compiler=sdcc
endif

#zcc +cpm -vn -O3 -clib=new cmdline.c -o cmdline -create-app
#SO3 --max-allocs-per-node200000 -startup=1 -clib=sdcc_iy -vn -DSPECTRUM_32COL

########################################

.SUFFIXES:
.PHONY: all clean

all: $(TARGET)

########################################

kickc:
	kickc.sh -e -p $(MACHINE) $(DEFINES) -o $(BIN) main.c

z88dk:
	zcc +$(MACHINE) $(CFLAGS) $(DEFINES) $(SOURCES) -o $(BIN) -create-app

zx:
	$(MAKE) SDK=z88dk MACHINE=zx z88dk

zxn:
	$(MAKE) SDK=z88dk MACHINE=zxn z88dk

m65:
	$(MAKE) SDK=kickc MACHINE=mega65 kickc

c64:
	$(MAKE) SDK=cc65 MACHINE=c64
	x64 $(PROGRAM)-c64.prg

########################################

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:.c=.d)
endif

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFINES) -o $@ $<

$(TARGET): $(SOURCES:.c=.o)
	$(CC) $(LDFLAGS) -o $(BIN) $^

clean:
	$(RM) *.o *.d *.map *.mem *_CODE.bin *_UNASSIGNED.bin

distclean: clean
	$(RM) $(BIN) *.prg *.tap


