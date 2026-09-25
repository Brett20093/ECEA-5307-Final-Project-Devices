CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
INCDIR = ../lcd
CFLAGS ?= -O0 -g -Wall -Werror -Wno-format-truncation -I$(INCDIR)
LDFLAGS ?=
LIBS ?= -lpthread 
all: freezer_monitor

freezer_monitor: buzzer_helper.o temperature_helper.o reed_switch_helper.o freezer_monitor.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
	
clean:
	rm -f *.o
	rm -f freezer_monitor