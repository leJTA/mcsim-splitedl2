#!/bin/bash
args=""
for var in "$@"
do
  args="$args $var"
done

/home/armel/pin/intel64/bin/pinbin -t /home/armel/pin/source/tools/ManualExamples/obj-intel64/inscount2.so -- $args
cat inscount.out
