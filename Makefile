PROGRAM = push
VERSION = 0.0.1

DEFINES = -DVERSION=\"$(VERSION)\"

DEFINES += -DHAVE_HISTORY -DHAVE_HINTS -DHAVE_COMPLETION

SOURCES = main.c term.c lined.c cli.c

export CC65_HOME=../../cc65/

ifdef TARGET
CC       = cl65
CFLAGS   = -t $(TARGET) $(INCLUDE) --create-dep $(<:.c=.d) -O
LDFLAGS  = -t $(TARGET) -m $(PROGRAM).map
BIN      = $(PROGRAM).prg
else
CC       = gcc
CFLAGS   = -MMD -MP -O -g3 -Wno-format-security
BIN      = $(PROGRAM)
SOURCES += vt100.c
DEFINES += -DVT100
endif

########################################

.SUFFIXES:
.PHONY: all clean
all: $(BIN)

run:
	x64 push.prg

c64:
	$(MAKE) TARGET=c64
	$(MAKE) run

ifneq ($(MAKECMDGOALS),clean)
-include $(SOURCES:.c=.d)
endif

%.o: %.c
	$(CC) -c $(CFLAGS) $(DEFINES) -o $@ $<

$(BIN): $(SOURCES:.c=.o)
	$(CC) $(LDFLAGS) -o $(BIN) $^

mega:
	../../kickc/bin/kickc.sh -e -p mega65 \
		-DKICKC -DMEGA65 -DHAVE_HINTS       \
		-DNULL=\(\(void*\)0\)               \
		-DVERSION=\"0.0.1\"                 \
		main.c

clean:
	$(RM) *.o *.d *.prg *.map $(PROGRAM)
