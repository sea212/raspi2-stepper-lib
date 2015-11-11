# Makefile Uebungsblatt 4 Echtzeitverarbeitung

SRCPATH = $(CURDIR)/src/
OBJPATH = $(CURDIR)/obj/

CC = gcc
CFLAGS = -std=gnu99 -W -Wall -Wno-unused-parameter -Wfatal-errors
LIBS = -lpthread -D_GNU_SOURCE -lrt -I/usr/local/include -L/usr/local/lib -lwiringPi

.PHONY: all clean distclean

all: steplib turn_once turn_once_rt load_tachometer

steplib: $(SRCPATH)steplib.c
	$(CC) -c $(CFLAGS) $< -o $(OBJPATH)$@.o $(LIBS)

turn_once: $(SRCPATH)turn_once.c
	$(CC) $(CFLAGS) $< -o $@ $(OBJPATH)steplib.o $(LIBS)

turn_once_rt: $(SRCPATH)turn_once_rt.c
	$(CC) $(CFLAGS) $< -o $@ $(OBJPATH)steplib.o $(LIBS)

load_tachometer: $(SRCPATH)load_tachometer.c
	$(CC) $(CFLAGS) $< -o $@ $(OBJPATH)steplib.o $(LIBS)

clean:
	rm -f steplib turn_once turn_once_rt load_tachometer

distclean: clean
