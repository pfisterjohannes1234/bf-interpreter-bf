#!/bin/bash
set -eu

scriptError()
{
  echo "Error on line $1" 1>&2
  exit 1
}

trap 'scriptError $LINENO' ERR



gcc -DGENERATE_SIMPLE=1 -E interpret.c | grep -v '^#' > interpret_.c

