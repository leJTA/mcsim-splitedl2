#!/bin/bash

./generate_trace.sh -skip_instrs 1000000000 -max_instrs 4392580190 -prefix pipes-1-32/in -slice_size 1 1 -- /home/cumulus/cpu2006/benchspec/CPU2006/429.mcf/run/run_base_ref_gcc41-64bit.0001/mcf_base.gcc41-64bit /home/cumulus/cpu2006/benchspec/CPU2006/429.mcf/run/run_base_ref_gcc41-64bit.0001/inp.in &

str=""
for i in {0..11}
do
  str="$str pipes-1-16/out$i.mid"
  str="$str pipes-1-8/out$i.mid"
  str="$str pipes-1-4/out$i.mid"
  str="$str pipes-classic/out$i.mid"
done
echo $str

./input_dispatch pipes/in $str &

for i in {0..11}
do
  ./repeater pipes-1-16/out$i.mid pipes-1-16/out$i &
  ./repeater pipes-1-8/out$i.mid pipes-1-8/out$i &
  ./repeater pipes-1-4/out$i.mid pipes-1-4/out$i &
  ./repeater pipes-classic/out$i.mid pipes-classic/out$i &
done
