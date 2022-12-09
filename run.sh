#!/bin/bash
# This script runs cweb with whatever flags and sets up the correct environment
CWEBFLAGS=""
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PWD" ./cweb $CWEBFLAGS $@
