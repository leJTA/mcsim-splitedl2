#!/bin/bash
args=""
for var in "$@"
do
  args="$args $var"
done

$HOME/pin/intel64/bin/pinbin -t $HOME/pin/source/tools/ManualExamples/obj-intel64/inscount2.so -- $args
cat inscount.out
