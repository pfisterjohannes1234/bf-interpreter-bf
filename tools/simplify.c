/*
Simplifyies brainfuck code to make it easier to understand.
The output is no longer valid brainfuck.

Read from stdin and output to stdout

It combines multiple +, -, < and > operations. It says how how many times a operation is repeated
 followed by the operation
It is mostly here to avoid manual counting
Other commands ( , . [ and ] ) are not combined, because they either don't make much sense to 
 combine or need a coresponding command ( [ vs ] )
Everything that is not a brainfuck command is ignored

It also formats the output

Example:
Convert ++++++++++[>>>+++<<<-]>>>+. to

10+
[
 3>
 3+
 3<
 1-
]
3>
+
.

So it easier to so how much we add and to which cell we go (especally when we add a lot or move a
 lot of cells).

*/
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/*
Print the output for the same character N times.
c should point to a table of 256 elements, where the only element which isn't 0 is the element we
 should print. The value indicates how many times the command is executed in a row. This element
 will be set to 0 before returning.
deep will point to a integer that keeps trakc of the indentation level
*/
void print(size_t *c, int *deep)
  {
    for(unsigned i=0;i<256;i++)
      {
        if(c[i])
          {
            switch(i)
              {
                case '>':  printf("%zu> ",c[i]); break;
                case '<':  printf("%zu< ",c[i]); break;
                case '+':  printf("%zu+ ",c[i]); break;
                case '-':  printf("%zu- ",c[i]); break;
                case '[':
                  {
                    printf("\n%*s",*deep,"");
                    for(size_t j=0;j<c[i];j++)
                      {
                         *deep+=2;
                        printf("[\n%*s",*deep,"");
                      }
                    break;
                  }
                case ']':
                  {
                    printf("\n%*s",*deep-2,"");
                    for(size_t j=0;j<c[i];j++)
                      {
                         *deep-=2;
                        printf("]\n%*s",*deep,"");
                      }
                    break;
                  }
                case '.':
                case ',':
                  { for(size_t j=0;j<c[i];j++) {printf("%c",i); }}  break;
              }
            c[i]=0;
            return;
          }
      }
  }

int main(int argc, char **argv)
  {
    (void)argc;
    (void)argv;
    size_t d[256];
    int deep=0;
    memset(d,0,sizeof d);
    while(1)
      {
        int c=getchar();
        if(c<0)
          {
            print(d,&deep);
            break;
          }
        //Previous character was a different character or we didn't read any character so far (in 
        // this case print() will be a noop
        if( !d[c] )
          { print(d,&deep); }
        d[c]++;
      }
    return 0;
  }


