CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
INCDIR = ../lcd
CFLAGS ?= -O0 -g -Wall -Werror -I$(INCDIR)
LDFLAGS ?=
LIBS ?= -lpthread 
all: freezer_monitor

freezer_monitor: freezer_monitor.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f *.o
	rm -f freezer_monitor