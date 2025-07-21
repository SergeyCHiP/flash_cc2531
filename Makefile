# Определяем ОС
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
# macOS
LDLIBS=-lm
CFLAGS=-g -D_GNU_SOURCE
LDFLAGS=-g
else
# Linux
LDLIBS=-lgpiod -lcrypt -lm -lrt
CFLAGS=-g -D_GNU_SOURCE
LDFLAGS=-g
endif

all: cc_chipid cc_read cc_write cc_erase

clean:
	rm -f *.o cc_chipid cc_read cc_write cc_erase

cc_erase : cc_erase.o CCDebugger.o GPIOlibgpiod.o
	gcc $(LDFLAGS) -o $@ $^ $(LDLIBS)

cc_write : cc_write.o CCDebugger.o GPIOlibgpiod.o
	gcc $(LDFLAGS) -o $@ $^ $(LDLIBS)

cc_read : cc_read.o CCDebugger.o GPIOlibgpiod.o
	gcc $(LDFLAGS) -o $@ $^ $(LDLIBS)

cc_chipid : cc_chipid.o CCDebugger.o GPIOlibgpiod.o
	gcc $(LDFLAGS) -o $@ $^ $(LDLIBS)

cc_chipid.o : cc_chipid.c CCDebugger.h
	gcc $(CFLAGS) -c $*.c

CCDebugger.o : CCDebugger.c CCDebugger.h GPIOlibgpiod.h
	gcc $(CFLAGS) -c $*.c

GPIOlibgpiod.o : GPIOlibgpiod.c GPIOlibgpiod.h
	gcc $(CFLAGS) -c $*.c
