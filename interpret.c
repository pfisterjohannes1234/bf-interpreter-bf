/*
C code to interpret brainfuck.
It should be writting that ../../c2bf/ can convert it to brainfuck, in a way that makes the
 code data aribtary large

And it should written in a way that it is easy to convert to brainfuck.
We need to do it by hand to have an infinite array for code and data.
*/

#define STEP 8
//#define SIZE (1024*1024*16)
#define SIZE (1024*1024*16)

#define GENERATE_SIMPLE 0 //Set this when we want to generate code for c2bf
#define START 80
#define START_EXTRA 480

int p=START+START_EXTRA;

#if !GENERATE_SIMPLE

#include <unistd.h>
#include <execinfo.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>





#define write_char putchar
//#define read_char  getchar

#define DEBUG 0
#define DEBUG_ADDRESS 1


unsigned char data[SIZE];


void printContent(unsigned char *data, size_t n, unsigned offset)
  {
    fprintf(stderr,"D: %2u ",offset);
    for(size_t i=0;i<n;i++)
      {
        unsigned char c = data[i*STEP+offset+START_EXTRA];
        unsigned char p = isprint(c) ? c : ' ';
        fprintf(stderr,"%c_%02X  ", p, c );
      }
    fprintf(stderr,"\n");
  }

void printData(unsigned char *data, size_t n)
  {
    printContent(data,n,2);
  }

void printCode(unsigned char *data, size_t n)
  {
    printContent(data,n,0);
  }

unsigned char read_char()
  {
    int i = getchar();
    if(i<0)
      {
        //return 0; //-1;
        return -0;
      }
    return i;
  }

void *symbol_array[32];
void catch_pagefault(int signal, siginfo_t *i, void *ignored)
  {
    (void)signal;
    (void)ignored;
    char buffer[4096];
    write
      (
        STDERR_FILENO,
        buffer,
        snprintf
          (
            buffer,
            sizeof buffer,
            "ERROR %p %p %i %i %i %i\n",
            i->si_lower,
            i->si_upper,
            p,
            signal,
            i->si_signo,
            i->si_errno
          )
      );
    int r = backtrace(symbol_array,32);
    backtrace_symbols_fd(symbol_array,r,STDERR_FILENO);
    exit(EXIT_FAILURE);
  }




#else // GENERATE_SIMPLE

#define DEBUG 0
#define DEBUG_ADDRESS 0

int data[2];

#endif // !GENERATE_SIMPLE

/*
data contains almost all data.
It is split up into blocks.
A block is STEP elements big
It is bascially a 1D array that is used as 2D array.

+-----+
|code | data[n*STEP+0] contains code
+-----+
| ==0 | data[n*STEP+1] intendet to convert while(...==0) to while(...!=0).
|     |  Sometimes combined with n*STEP+6
+-----+
|data | data[n*STEP+2] contains user data
+-----+
| []  | data[n*STEP+3] used to jump between [ and ]
+-----+
|<data| data[n*STEP+4] filled with 1 from data pointer to the code pointer
| 1/0 |  when both point to the same block, all are 0
|     |  when code pointer is before the data pointer, all are 0
+-----+
|code>| data[n*STEP+5] filled with 1 from code pointer to the data pointer
| 1/0 |  when both point to the same block, all are 0.
|     |  when data pointer is before the code pointer, all are 0
+-----+
|  ?  | data[n*STEP+6] intendet to convert if to while. All while( data[p*STEP+6] ) means
|     |  if( data[p*STEP+6] ).  Sometimes combined with n*STEP+1
+-----+
|  ?  | data[n*STEP+7] not used yet.
+-----+


############
# Pointers #
############

right: higher address
left:  lower address

We need to keep track of 2 pointer. One pointer for data and one pointer for for code.
But we only have access to a single array as data.

Maybe it would be possible to store a index in a cell and use a specific amount of < or >
 to go to the correct position. But that sounds complicated for infinite arrays so we use
 2 cells per block to indicate if the current data position is to the right of the current
 position or not, i.e. if the data pointer points to a block that is further to the right.
Similar, there is 1 cell per block that indicates if the the current code pointer is to the right.

n*STEP + 4 is filled with some 1 when the data pointer points left of where the code pointer points
 to. And all 0 in every other case

n*STEP + 5 is filled with some 1 when the code pointer points left of where the data pointer points
 to. And all 0 in every other case

There are 3 different possibilites which is drawn in the following ASCII art.
One column is for one block with STEP cells.


State A: Data pointer is to the right of the code pointer:

      | Code pointer
      V
+---+---+---+---+---+
##################### < Cells with offset 0 - 3 and 6-7. We don't care about them for now
+---+---+---+---+---+
| 0 | 0 | 0 | 0 | 0 | n*STEP+4
+---+---+---+---+---+
| 0 | 1 | 1 | 1 | 0 | n*STEP+5
+---+---+---+---+---+
              ^
              | Data pointer

State B: Data pointer and code pointer point to the same block

      | Code pointer
      V
+---+---+---+---+---+
##################### < Cells with offset 0 - 3 and 6-7. We don't care about them for now
+---+---+---+---+---+
| 0 | 0 | 0 | 0 | 0 | n*STEP+4
+---+---+---+---+---+
| 0 | 0 | 0 | 0 | 0 | n*STEP+5
+---+---+---+---+---+
      ^
      | Data pointer


State C: Data pointer is to the left of the code pointer:

              | Code pointer
              V
+---+---+---+---+---+
##################### < Cells with offset 0 - 3 and 6-7. We don't care about them for now
+---+---+---+---+---+
| 0 | 1 | 1 | 1 | 0 | n*STEP+4
+---+---+---+---+---+
| 0 | 0 | 0 | 0 | 0 | n*STEP+5
+---+---+---+---+---+
      ^
      | Data pointer


*/

//OS means offset
#define OS_CODE  0 //Contains code
#define OS_EQU0  1 //to convert ==0 to !=0
#define OS_DATA  2 //Contains data
#define OS_DEEP  3 //How deep inside [] we are
#define OS_GODL  4 //1 if data is to the left
#define OS_GOCL  5 //1 if code is to the left
#define OS_IF_W  6 //To replace if with while


//Goes from the code position to the data position. NOOP when we are already at the data location
#define GO_DATA()                  \
  /* Handle State A */             \
  while( data[p+STEP+OS_GOCL]!=0 ) \
    { p = p+STEP; }                \
  /* Handle State C */             \
  while( data[p-STEP+OS_GODL]!=0 ) \
          { p = p-STEP; }          \

//Goes from the data position to the code position. NOOP when we are already at the code location
#define GO_CODE()                  \
  /* Handle State A */             \
  while( data[p-STEP+OS_GOCL]!=0 ) \
    { p = p-STEP; }                \
  /* Handle State C */             \
  while( data[p+STEP+OS_GODL]!=0 ) \
          { p = p+STEP; }          \

//Move the data pointer 1 to the right (increment). Use it when we are already at the data pointer location
#define MOVE_DATA_R()                      \
  if( data[p+OS_GODL]==0 ) /* State A||B */\
    {                                      \
      data[p+OS_GOCL]=1;                   \
      p=p+STEP;                            \
      data[p+OS_GOCL]=1;                   \
    }                                      \
  else /* State C */                       \
    {                                      \
      data[p+OS_GODL]=0;                   \
      p=p+STEP;                            \
      if( data[p+STEP+OS_GODL]==0 )        \
        { /* new state is B */             \
          data[p+OS_GODL]=0;               \
        }                                  \
    }                                      \

//Move the data pointer 1 to the left (decrement). Use it when we are already at the data pointer location
#define MOVE_DATA_L()                      \
  if( data[p+OS_GOCL]==0 ) /* State B||C */\
    {                                      \
      data[p+OS_GODL]=1;                   \
      p=p-STEP;                            \
      data[p+OS_GODL]=1;                   \
    }                                      \
  else /* State A */                       \
    {                                      \
      data[p+OS_GOCL]=0;                   \
      p=p-STEP;                            \
      if( data[p-STEP+OS_GOCL]==0 )        \
        { /* new state is B */             \
          data[p+OS_GOCL]=0;               \
        }                                  \
    }                                      \

//Move the code pointer 1 to the right (increment). Use it when we are already at the code pointer location
//Very similar to MOVE_DATA_R but for the code pointer
#define MOVE_CODE_R()                      \
  if( data[p+OS_GOCL]==0 ) /* State B||C */\
    {                                      \
      data[p+OS_GODL]=1;                   \
      p=p+STEP;                            \
      data[p+OS_GODL]=1;                   \
    }                                      \
  else /* State A */                       \
    {                                      \
      data[p+OS_GOCL]=0;                   \
      p=p+STEP;                            \
      if( data[p+STEP+OS_GOCL]==0 )        \
        { /* new state is B */             \
          data[p+OS_GOCL]=0;               \
        }                                  \
    }                                      \

//Move the code pointer 1 to the left (decrement). Use it when we are already at the code pointer location
//Very similar to MOVE_DATA_L but for the code pointer
#define MOVE_CODE_L()                      \
  if( data[p+OS_GODL]==0 ) /* State A||B */\
    {                                      \
      data[p+OS_GOCL]=1;                   \
      p=p-STEP;                            \
      data[p+OS_GOCL]=1;                   \
    }                                      \
  else /* State C */                       \
    {                                      \
      data[p+OS_GODL]=0;                   \
      p=p-STEP;                            \
      if( data[p-STEP+OS_GODL]==0 )        \
        { /* new state is B */             \
          data[p+OS_GODL]=0;               \
        }                                  \
    }                                      \


#if !GENERATE_SIMPLE
int main()
  {
#endif
    #if !GENERATE_SIMPLE
      memset(data,0,sizeof data);
    #endif // !GENERATE_SIMPLE
    #if DEBUG_ADDRESS
      fprintf(stderr,"DATA START %p END %p\n",(void*)data,(void*)data+sizeof data);
      struct sigaction a;
      memset(&a,0,sizeof a);
      a.sa_sigaction=catch_pagefault;
      a.sa_flags=SA_RESETHAND|SA_SIGINFO;
      sigaction(SIGSEGV,&a,NULL);
    #endif // !GENERATE_SIMPLE


    //#############
    //# Read code #
    //#############
    /*
    Read the code to the cells with offset 0
    We don't fill cells p*STEP+4 nor p*STEP+5 durring this, since we go back to cell 0 after that.
    */

    //We read till we read a 0. 0 marks the end of code. After that we would start to read data.
    // But that is only done when the brainfuck code says so (i.e. uses the , command)
    data[p+OS_CODE] = read_char();
    while( data[p+OS_CODE]!=0 )
      {
        p = p + STEP;
        data[p+OS_CODE] = read_char();
      }
    p = p - STEP;
    while( data[p+OS_CODE]!=0 ) //Go back to cell 0
      { p = p - STEP; }
    p = p + STEP;

    //We loop till we reach the end of the code
    while( data[p+OS_CODE]!=0 )
      {
        #if DEBUG
          fprintf(stderr,"p       %*u\n",(p-START_EXTRA)*6/8,p);
          printContent(data,38,0);
          printContent(data,38,2);
          printContent(data,38,4);
          printContent(data,38,5);
        #endif


        //#############
        //# Handle +  # Increment current cell
        //#############
        data[p+OS_EQU0] = data[p+OS_CODE] - '+';
        data[p+OS_IF_W] = data[p+OS_EQU0];
        while( data[p+OS_IF_W] )
          { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
        data[p+OS_EQU0]--;
        while( data[p+OS_EQU0] )  //if command=='+'
          {
            GO_DATA()
            data[p+OS_DATA] = data[p+OS_DATA] + 1;
            GO_CODE()
            data[p+OS_EQU0] = 0;
          }

        //#############
        //# Handle -  # Decrement current cell
        //#############
        data[p+OS_EQU0] = data[p+OS_CODE] - '-';
        data[p+OS_IF_W] = data[p+OS_EQU0];
        while( data[p+OS_IF_W] )
          { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
        data[p+OS_EQU0]--;
        while( data[p+OS_EQU0] )  //if command=='-'
          {
            GO_DATA()
            data[p+OS_DATA] = data[p+OS_DATA] - 1;
            GO_CODE()
            data[p+OS_EQU0] = 0;
          }

        //#############
        //# Handle ,  # Read input to current cell
        //#############
        data[p+OS_EQU0] = data[p+OS_CODE] - ',';
        data[p+OS_IF_W] = data[p+OS_EQU0];
        while( data[p+OS_IF_W] )
          { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
        data[p+OS_EQU0]--;
        while( data[p+OS_EQU0] )  //if command==','
          {
            GO_DATA()
            data[p+OS_DATA] = read_char();
            GO_CODE()
            data[p+OS_EQU0] = 0;
          }

        //#############
        //# Handle .  # output current cell
        //#############
        data[p+OS_EQU0] = data[p+OS_CODE] - '.';
        data[p+OS_IF_W] = data[p+OS_EQU0];
        while( data[p+OS_IF_W] )
          { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
        data[p+OS_EQU0]--;
        while( data[p+OS_EQU0] )  //if command=='.'
          {
            GO_DATA()
            write_char( data[p+OS_DATA] );
            GO_CODE()
            data[p+OS_EQU0] = 0;
          }

        //#############
        //# Handle <  # go a cell to the left (decrement cell pointer)
        //#############
        data[p+OS_EQU0] = data[p+OS_CODE] - '<';
        data[p+OS_IF_W] = data[p+OS_EQU0];
        while( data[p+OS_IF_W] )
          { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
        data[p+OS_EQU0]--;
        while( data[p+OS_EQU0] )  //if command=='<'
          {
            GO_DATA()
            MOVE_DATA_L()
            GO_CODE()
            data[p+OS_EQU0] = 0;
          }

        //#############
        //# Handle >  # go a cell to the right (increment cell pointer)
        //#############
        data[p+OS_EQU0] = data[p+OS_CODE] - '>';
        data[p+OS_IF_W] = data[p+OS_EQU0];
        while( data[p+OS_IF_W] )
          { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
        data[p+OS_EQU0]--;
        while( data[p+OS_EQU0] ) //if command=='>'
          {
            GO_DATA()
            MOVE_DATA_R()
            GO_CODE()
            data[p+OS_EQU0] = 0;
          }

        //#############
        //# Handle [  # jump to the corresponding ] when current cell is 0
        //#############
        data[p+OS_EQU0] = data[p+OS_CODE] - '[';
        data[p+OS_IF_W] = data[p+OS_EQU0];
        while( data[p+OS_IF_W] )
          { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
        data[p+OS_EQU0]--;
        while( data[p+OS_EQU0] ) // command is [
          {
            GO_DATA()

            //if data[p+OS_DATA] == 0 # i.e. if current cell is 0
            data[p+OS_EQU0] = data[p+OS_DATA];
            data[p+OS_IF_W] = data[p+OS_DATA];
            while( data[p+OS_IF_W] )
              { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
            data[p+OS_EQU0]--;
            while( data[p+OS_EQU0] ) //Enter here is cell is 0
              {
                GO_CODE()
                data[p+OS_DEEP] = 1;
                while( data[p+OS_DEEP]!=0 )
                  {
                    data[p+STEP+OS_DEEP] = data[p+OS_DEEP];
                    MOVE_CODE_R();

                    data[p+OS_EQU0] = data[p+OS_CODE] - '[';
                    if( data[p+OS_EQU0]==0 )       { data[p+OS_DEEP] = data[p+OS_DEEP]+1; }
                    data[p+OS_EQU0] = data[p+OS_EQU0] - ']' + '[';
                    if( data[p+OS_EQU0]==0 )  { data[p+OS_DEEP] = data[p+OS_DEEP]-1; }
                  }
                data[p+OS_EQU0]=0;
              }
            GO_CODE()
            data[p+OS_EQU0] = 0;
            data[p+OS_IF_W] = 1;
          }

        //Only go to the next while loop when we weren't in the one before
        //This is because we end up at ] in case we entered the one before and the value was 0
        //But there is no need to check it again
        //Technically not needed, since we skip this one when cell value is 0 but that takes a long
        // time
        data[p+OS_IF_W]--;
        while( data[p+OS_IF_W] )
          {
            //#############
            //# Handle ]  # jump to the corresponding [ when current cell is not 0
            //#############
            data[p+OS_EQU0] = data[p+OS_CODE] - ']';
            data[p+OS_IF_W] = data[p+OS_EQU0];
            while( data[p+OS_IF_W] )
              { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
            data[p+OS_EQU0]--;
            while( data[p+OS_EQU0] ) //command is ]
              {
                GO_DATA()

                //if data[p+OS_DATA] != 0 # i.e. if current cell is 0
                data[p+OS_IF_W] = data[p+OS_DATA];
                while( data[p+OS_IF_W] ) //Enter here is cell is !=0
                  {
                    GO_CODE()
                    data[p+OS_DEEP] = 1;
                    while( data[p+OS_DEEP]!=0 )
                      {
                        data[p-STEP+OS_DEEP] = data[p+OS_DEEP];
                        MOVE_CODE_L();

                        data[p+OS_EQU0] = data[p+OS_CODE] - '[';
                        if( data[p+OS_EQU0]==0 )       { data[p+OS_DEEP] = data[p+OS_DEEP]-1; }
                        data[p+OS_EQU0] = data[p+OS_EQU0] - ']' + '[';
                        if( data[p+OS_EQU0]==0 )  { data[p+OS_DEEP] = data[p+OS_DEEP]+1; }
                      }
                    data[p+OS_IF_W]=0;
                  }
                GO_CODE()
                data[p+OS_EQU0] = 0;
              }
            data[p+OS_IF_W] = 0;
          }
        MOVE_CODE_R();
      }
    #if DEBUG
      fprintf(stderr,"Q       %*u\n",(p-START_EXTRA)*6/8,p);
      printContent(data,38,0);
      printContent(data,38,2);
      printContent(data,38,4);
      printContent(data,38,5);
    #endif
#if !GENERATE_SIMPLE
  }
#endif // !GENERATE_SIMPLE


