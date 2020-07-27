#!/bin/bash

./generate_trace.sh -max_instrs 4392580190 -prefix pipes-classic/in -slice_size 1 1 -- /home/cumulus/cpu2006/benchspec/CPU2006/429.mcf/run/run_base_ref_gcc41-64bit.0001/mcf_base.gcc41-64bit /home/cumulus/cpu2006/benchspec/CPU2006/429.mcf/run/run_base_ref_gcc41-64bit.0001/inp.in &

str=""
for i in {0..7}
do
  str="$str pipes-classic/out$i.mid"
done
echo $str

./input_dispatch pipes-classic/in $str &

for i in {0..7}
do
  ./repeater pipes-classic/out$i.mid pipes-classic/out$i &
done
