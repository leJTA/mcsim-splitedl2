#!/bin/bash

args=""
for var in "$@"
do
  args="$args $var"
done

/home/armel/pin/intel64/bin/pinbin -t /home/armel/mcsim/TraceGen/tracegen $args
