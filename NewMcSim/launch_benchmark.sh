#!/bin/bash
./newmcsim -mdfile ../Apps/md/new-md-o3-closed-1-16.py -runfile ../Apps/list/run-trace.py -num_proc_per_hthread 1 > out-1-16.txt
./newmcsim -mdfile ../Apps/md/new-md-o3-closed-1-8.py -runfile ../Apps/list/run-trace.py -num_proc_per_hthread 1 > out-1-8.txt
./newmcsim -mdfile ../Apps/md/new-md-o3-closed-1-4.py -runfile ../Apps/list/run-trace.py -num_proc_per_hthread 1 > out-1-4.txt

