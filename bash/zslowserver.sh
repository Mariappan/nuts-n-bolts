#!/bin/bash

COMPARE=:
if [ $# -ge 1 ]; then
    COMPARE=$1
fi

for i in {1..5}; do
    echo Server: 0$i
    ssh server-0${i} uptime
    ssh server-0${i} ps aux | grep Xvnc | awk '/rfbauth/ { print $12 "    " $1;}'| grep ${COMPARE}
done
