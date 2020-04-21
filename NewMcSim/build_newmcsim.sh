#!/bin/bash

make clean
make INCS=-I/home/armel/pin/extras/xed-intel64/include -j 4
