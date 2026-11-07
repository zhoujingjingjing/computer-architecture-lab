#!/bin/sh

cd /benchmarks/
#/sbin/m5 switchcpu
/sbin/m5 dumpstats
/sbin/m5 resetstats
./ffos
echo "Cris says done with this bench! :-)"
/sbin/m5 exit
/sbin/m5 exit
