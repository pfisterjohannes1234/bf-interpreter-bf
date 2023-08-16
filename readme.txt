This is a brainfuck interpreter written in brainfuck.

Goal was it to have a brainfuck interpreter written in brainfuck that can interpret itself.
It can interpret itself, but very slow (as somewhat expected)


#Requirements for the interpreter / compiler

The interpreter / compiler that interprets this brainfuck interpreter (the interpreter of the
 interpreter) needs at least this requirements:

- Support the 8 brainfuck commands ( . , [ ] < > + - )
- A cell needs to be able to hold at least all characters used for brainfuck in ASCII
- A cell needs to have a limitied maximum. 8 bit cells is recommended. The brainfuck code [-]
   must be able to set a cell of any value to 0, including when the cell was "negative"
- Support for enough cells and data. Depending in the program we are interpret, we only need cells
   to the rigth (using more > than <)
- Initial value of all cells has to be 0


The minimum amount of cells needed are:
Per byte in the program we interpret we need 8 cells, and 16 cells on top of that.
This are all to the right (>) dirrection from the start.

And then there are 8 cells needed for every cell the interpreted program touches. This can overlap
 with the cells used for the program code (so you only have to count them once).


#Input

This brainfuck interpreter expects a valid brainfuck program given via stdin (or whatever , is using
 followed by a 0-byte ('\0') to mark the end of the code. After that comes data the interpreted
 can use.


#Example input

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

To let this interpreter interpret itself (use interpret_minimal.bf since it is a bit shorter and
 every tiny bit helps. But it is slow anyway). This outputs one byte of value 2.

    cat interpret_minimal.bf <(echo -en '\0++.\0\0') | someBrainfuckInterpreter interpret_minimal.bf | xxd


You can also try this (takes about 23 min on my AMD Zen 2 CPU). It should output "Hello World!\n":

    cat interpret_minimal.bf <(echo -en '\0') ./data/hello.bf <( echo -en '\0') | someBrainfuckInterpreter interpret_minimal.bf


#Licence

The interpreter is public domain.
The example brainfuck programs in the folder data have various licences, some of them where found
 without any licence information.

