#!/bin/bash
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

python3 ./convert.py < ./interpret_.c > crazy.bf
#create crazy_minimal.bf with only valid code characters. Note: tr doesn't accept every order but
# '][+,.><-' seems to work
tr -cd '][+,.><-'  < crazy.bf > crazy_minimal.bf


