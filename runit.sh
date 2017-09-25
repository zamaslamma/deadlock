#!/bin/sh
END=1000
for i in $(seq 1 $END); do
echo "$i"
./program
done
