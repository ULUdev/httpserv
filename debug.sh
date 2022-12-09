#!/bin/bash
# This file runs gdb in the required environment for debugging cweb and test/mock
DEBUGFLAGS=""
LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$PWD" gdb $DEBUGFLAGS $@
