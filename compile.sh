#!/bin/bash

#Generate a compiled program from interpret.c This should be able to interpret brainfuck code
#Intended to debug the interpreter logic, not very usefull as normal brainfuck interpreter 
# (Slower and less flexible than other interpreters)

set -eu

scriptError()
{
  echo "Error on line $1" 1>&2
  exit 1
}

trap 'scriptError $LINENO' ERR

gcc -ggdb -Wall -Wextra interpret.c -o bf-interpreter-c
