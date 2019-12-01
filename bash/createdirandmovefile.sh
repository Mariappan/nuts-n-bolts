#!/bin/sh

#for file in {START}*.*; do
for file in *.deb; do
    dir=${file%%.*}
    mkdir -p "$dir"
    mv "$file" "$dir"
done

