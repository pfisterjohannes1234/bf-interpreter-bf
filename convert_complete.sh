#!/bin/bash

#Script to generates the brainfuck code, so that there is a brainfuck interpreter written in
# brainfuck. And it compiles interpret.c to a native running interpreter
#The output files are interpret.bf and interpret_minimal.bf

set -eu

scriptError()
{
  echo "Error on line $1" 1>&2
  exit 1
}

trap 'scriptError $LINENO' ERR


bash ./generate_simple.sh

#Format the code. Not needed but can it make easier to debug the resulting brainfuck code in
# case we use a brainfuck interpreter that can output debug information including line number
astyle --style=gnu --indent=spaces=2 --add-brackets --indent-preproc-cond ./interpret_.c || true
rm ./interpret_.c.orig || true

python3 ./convert.py < ./interpret_.c > interpret.bf

#create interpret_minimal.bf with only valid code characters. Note: tr doesn't accept every order
# but '][+,.><-' seems to work
tr -cd '][+,.><-'  < interpret.bf > interpret_minimal.bf

bash ./compile.sh

