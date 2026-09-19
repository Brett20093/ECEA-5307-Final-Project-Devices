#!/bin/sh

MOD_PATH="/lib/modules/$(uname -r)/extra/"

case "$1" in
    start)
	    echo "Starting freezer monitor"
		modprobe reed_switch
		modprobe 1602_lcd
		start-stop-daemon -S -n freezer_monitor --exec /usr/bin/freezer_monitor -- -d
		;;
	stop)
	    echo "Stopping freezer monitor"
		modprobe -r reed_switch
		modprobe -r 1602_lcd
		start-stop-daemon -K -n aesdsocket
		;;
	*)
	    echo "Usage: $0 {start|stop}"
	exit 1
esac

exit 0
        