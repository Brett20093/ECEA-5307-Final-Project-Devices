CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
CFLAGS ?= -O0 -g -Wall -Werror
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