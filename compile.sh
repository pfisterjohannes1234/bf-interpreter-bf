#!/bin/bash
set -eu

scriptError()
{
  echo "Error on line $1" 1>&2
  exit 1
}

trap 'scriptError $LINENO' ERR

gcc -ggdb -Wall -Wextra interpret.c -o bf-interpreter-c
