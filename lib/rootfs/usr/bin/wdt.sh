#!/bin/sh

cnt=1
while [ $cnt -le $1 ] ; do
        let "cnt++"
        echo 1 > /dev/watchdog
        sleep 15
done
