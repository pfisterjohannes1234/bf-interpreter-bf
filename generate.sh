#!/bin/bash

#Script to generates the brainfuck code, so that there is a brainfuck interpreter written in
# brainfuck. And it compiles interpret.c to a native running interpreter
#
#The output files are bf-interpreter-c interpret_.c, interpret.bf and interpret_minimal.bf

set -eu

scriptError()
{
  echo "Error on line $1" 1>&2
  exit 1
}

trap 'scriptError $LINENO' ERR

#Generate a "simpler" version of interpret.c, so that we can convert it to brainfuck code
gcc -DGENERATE_SIMPLE=1 -E interpret.c | grep -v '^#' > interpret_.c

#Format the code. Not needed but can it make easier to debug the resulting brainfuck code in
# case we use a brainfuck interpreter that can output debug information including line number
#Since it it optional, we ignore any error (including command not found)
astyle --style=gnu --indent=spaces=2 ./interpret_.c -n || true

#Create the brainfuck interpreter written in brainfuck, by converting the "simpler" c code
python3 ./convert.py < ./interpret_.c > interpret.bf

#create interpret_minimal.bf with only valid code characters. Note: tr doesn't accept every order
# but '][+,.><-' seems to work
#This can help performance.
tr -cd '][+,.><-'  < interpret.bf > interpret_minimal.bf

#Generate compiled version of the C source code, for debugging the interpreter logic
gcc -ggdb -Wall -Wextra interpret.c -o bf-interpreter-c

