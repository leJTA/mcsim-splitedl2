#!/bin/bash

args=""
for var in "$@"
do
  args="$args $var"
done

$HOME/pin/intel64/bin/pinbin -t $HOME/mcsim/TraceGen/tracegen $args
