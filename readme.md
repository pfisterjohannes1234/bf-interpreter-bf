This is a brainfuck interpreter written in brainfuck.

Goal was it to have a brainfuck interpreter written in brainfuck that can interpret itself.
It can interpret itself, but very slow (as somewhat expected)

## Create brainfuck code

Run the bash script `./generate.sh` to generate the brainfuck interpreter written in brainfuck:
 `interpret.bf` and `interpret_minimal.bf`.
You need bash, tr, python3 and gcc to create this files. Optional astyle to reformat the gcc
 preprocessor output, after macros where expanded.


## Requirements for the interpreter / compiler

The interpreter / compiler that interprets this brainfuck interpreter (the interpreter of the
 interpreter) needs at least this requirements:

- Support for the 8 brainfuck commands ( `.` `,` `[` `]` `<` `>` `+` `-` )
- A cell needs to be able to hold at least all characters used for brainfuck in ASCII. i.e. Large
   enough to hold the value 62 which is used to represent a `]`.
- A cell needs to have a limitied maximum. 7 bit or 8 bit cells is recommended. The brainfuck
   code `[-]` must be able to set a cell of any value to 0, including when the cell was "negative".
   i.e. When the current cell has the value 0 and we execute this code `-[-]`, the loop must end and
   should not be stuck forever counting towards negative infinity.
   A smaller cell maximum is probably faster.
- Decreasing a cell value of 0 should wrap arround and end in the maximum cell value. i.e. Executing
   `-` N times on a cell should only result in a 0 if the initial value of the cell was N, assuming
   N is not larger than 62 (Or `']'`)
- Support for enough cells and code length. Depending in the program we are interpret, we only need
   cells to the rigth (using more > than <). We need max( (2+N)\*8, M\*8 ) cells for data where N
   is the length of the program we are interpreting and M is the amount of cells the interpreted
   Program is using, assuming the interpreted program only uses cells to the right.
- Initial value of all cells has to be 0


## Features provided for the brainfuck program we interpret

The brainfuck interpreter in brainfuck provides this features:

- Support for the 8 brainfuck commands ( `.` `,` `[` `]` `<` `>` `+` `-` )
- Ignores code characters that are not a brainfuck command (but that hurts performance)
- Same cell limits as the interpreter of the interpreter provides.
- Arbitary code length and data length is supported, if the interpreter of the interpreter supports
   enough data / cells.
- Same EOF (end of file) behaviour as the interpreter of the interpreter.


## Input

This brainfuck interpreter expects a valid brainfuck program given via stdin (or whatever , is
 reading from) followed by a 0-byte ('\0') to mark the end of the code. After that comes data the
 interpreted can use.
If there is no 0-byte and the the EOF-value is also not 0, the interpreter will be stuck in a
 infinite loop.

## Example input

Examples how you may use this interpreter in bash or similar shell.
For that you can use the example brainfuck programs in the folder data

You have to find, write ... a brainfuck interpreter first.


Interpret a very simple program that adds 3 to a cell and then outputs it. Use xxd to see the
 (binary) output of a byte with value 3

    echo -ne '+++.\0\0' | someBrainfuckInterpreter interpret.bf | xxd

The brainfuck program reverse.bf reads a input and print it in reverse. This should output "GFEDCBA\n"

    cat ./data/reverse.bf <(echo -en '\0\nABCDEFG\0') | someBrainfuckInterpreter interpret.bf

Output 99 bottles of beer

    cat ./data/99.bf <(echo -en '\0\0') | someBrainfuckInterpreter interpret.bf

To let this interpreter interpret itself (use `interpret_minimal.bf` since it is a bit shorter and
 every tiny bit helps. But it is slow anyway). This outputs one byte of value 2.

    cat interpret_minimal.bf <(echo -en '\0++.\0\0') | someBrainfuckInterpreter interpret_minimal.bf | xxd


You can also try this. It should output "Hello World!\n":

    cat interpret_minimal.bf <(echo -en '\0') ./data/hello.bf <( echo -en '\0') | someBrainfuckInterpreter interpret_minimal.bf


## Licence

The interpreter is public domain.
The example brainfuck programs in the folder data have various licences, some of them where found
 without any licence information.

