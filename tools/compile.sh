#!/bin/bash

#Compile the small programs in this directory
# - A native brainfuck interpreter
# - A program to make brainfuck programs more human readable

set -eu

scriptError()
{
  echo "Error on line $1" 1>&2
  exit 1
}

trap 'scriptError $LINENO' ERR

gcc       -Wall -Wextra -Wpedantic -O3 ./native-interpreter.c -o ./bf
gcc -ggdb -Wall -Wextra -Wpedantic -O0 ./simplify.c           -o ./simplify

