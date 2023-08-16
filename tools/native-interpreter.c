/*
A simpler native brainfuck interpreter.

It should be faster than ../bf-interpreter-c but isn't that optimized.
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//Maximum size of code and data
#define SIZE (1024ULL*1024*64)

//Which type do we use to store a index to either code or data?
#define P uint32_t

unsigned char code[SIZE];
unsigned char data[SIZE];

//Jumpmap contains the index of the corresponding [ or ] at the locations of [ and ]
// This way we can jump fast between [ and ]
P jumpmap[SIZE];
P jumpstack[SIZE]; //used to store locations of [, used only in createJumpMap()

// Fill jumpmap, so we can easely jump between [ and ]
//Use jumpstack to store locations of [, use it as a FILA-stack
int createJumpMap(P size)
  {
    P stackdeep = 0;
    for(P i=0;i<size;i++)
      {
        if( code[i]=='[' )
          {
            jumpstack[stackdeep] = i;
            stackdeep++;
          }
        else if( code[i]==']' )
          {
            if( !stackdeep )
              { return -1; }
            stackdeep--;
            jumpmap[i] = jumpstack[stackdeep];
            jumpmap[jumpstack[stackdeep]] = i;
          }
      }
    if( stackdeep )
      { return -1; }
    return 1;
  }

int main(int argc, char **argv)
  {
    if(argc<2)
      {
        fprintf(stderr,"Ussage: %s <code file>\n",argv[0]);
        return 1;
      }
    FILE *codeFile = fopen(argv[1],"rb");
    if(!codeFile)
      {
        perror("Can't open code file");
        return 1;
      }
    size_t codeSize = fread(code,1,SIZE-1,codeFile);
    if( !codeSize )
      {
        if( ferror(codeFile) )
          {
            perror("Can not read from code file");
            fclose(codeFile);
            return 1;
          }
        else //no code -> no output -> exit successfully
          {
            fclose(codeFile);
            return 0;
          }
      }
    fclose(codeFile);
    if( createJumpMap(codeSize)<0 )
      {
        fprintf(stderr,"[ doesn't match with ]\n");
        return 1;
      }

    P dataPointer=SIZE/2;
    for(P i=0;i<codeSize;i++)
      {
        switch(code[i])
          {
            case '>':  if(dataPointer++>=SIZE-1){ fprintf(stderr,"Data overflow\n");  return 1; } break;
            case '<':  if(!dataPointer--)       { fprintf(stderr,"Data underflow\n"); return 1; } break;
            case '+':  data[dataPointer]++;                                                       break;
            case '-':  data[dataPointer]--;                                                       break;
            case '[':  i =  data[dataPointer] ? i : jumpmap[i] ;                                  break;
            case ']':  i = !data[dataPointer] ? i : jumpmap[i] ;                                  break;
            case ',':  { int C = getchar(); data[dataPointer] = C==-1 ? 0 : C; }                  break;
            case '.':  putchar( data[dataPointer] );                                              break;
          }
      }
    return 0;
  }

