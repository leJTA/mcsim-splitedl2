#!/bin/bash
./mcsim -mdfile ../Apps/md/md-o3-closed.py -runfile ../Apps/list/run-$1.py -num_proc_per_hthread 1
