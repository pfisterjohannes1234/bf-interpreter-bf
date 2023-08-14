#!/bin/bash
set -eu

#Script to convert interpret.c to a "simpler" version so that convert.py can understand it.
#Output file is interpret_.c which is very C-like but not 100% C and it uses only a small subset of
# C


scriptError()
{
  echo "Error on line $1" 1>&2
  exit 1
}

trap 'scriptError $LINENO' ERR



gcc -DGENERATE_SIMPLE=1 -E interpret.c | grep -v '^#' > interpret_.c

