#!/bin/bash
./newmcsim -mdfile ../Apps/md/new-md-o3-closed.py -runfile ../Apps/list/run-$1.py -num_proc_per_hthread 2
