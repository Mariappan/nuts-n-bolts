#!/bin/bash


for i in {1..5}; do
    echo Server :0$i;
    ssh Server-0${i} 'uptime'
done
