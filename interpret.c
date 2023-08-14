/*
C code to interpret brainfuck. Written in a way that the interpreter can easely be converted to
 a brainfuck program, so that we end up with a brainfuck interpreter in brainfuck.
It is a goal to support arbitary large programs that need arbitary large amounts of data.

The input is only given by stdin.
The input should be a brainfuck program.
Then a '\0' / 0 to mark the end of the brainfuck program
Then the data for the brainfuck program.

The complete brainfuck program is read before be we start executing


There are 2 ways to "compile" (on of them is only preprocessing) the code, controlled by
 GENERATE_SIMPLE.
When GENERATE_SIMPLE is set, we can use a c preprocessor to generate a "simpler" (means closer to
 brainfuck) version.
When GENERATE_SIMPLE is not set, it generates C code, that supports more features and allows
 debugging the interpreter.

It should written in a way that it is easy to convert to brainfuck.

That means:
 - The only data we have is a very large array and one pointer / index to it. Access to that array
    is only allowed via this pointer / index
 - We can only increment or decrement the pointer by a specific amount. We can not set the pointer
 - There are no function calls
 - There are no pointers
 - There are no enums nor structs
 - There are no if, else, for, do{}while(), switch nor goto. Only while loops are allowed
 - Only while ( <some element of the arra> ) but without any ==, &&, || !=, .... nor negation (!)
 - There are no strings
 - No multiplaction, division, |, &, ^, ...
 - No array access by a non fixed value except the single pointer / index

But this is still allowed:
 - Setting a element in the array (but only via the single pointer / index that exist )
 - Character literals
 - Adding and subtracting a fixed value from a cell or from the pointer / index
 - Accessing a element in the array with a fixed offset from the pointer
 - Copy a element of the array to a other element
 - A cell has enough space for a single char/ byte

The resulting code should be able to handle a infinite array (The C version obviously doesn't have
 a infinite array, but you could convert it 1:1 in brainfuck and handle other brainfuck programs
 that need an arbitary amount of data, given the the first interpreter, that interprets the
 brainfuck who is interpreting a other brainfuck program, supports a infinite array.




It was inspired by the code that c2bf (from here https://github.com/arthaud/c2bf) can handle
But it turns out c2bf can't probably deal with a infinite array.

*/

#define STEP 8

//Set this to 1 when we want to generate code that is easier to convert
//Set this to 0 when we want to have normal C code with the possibility to debug
#ifndef GENERATE_SIMPLE
  #define GENERATE_SIMPLE 0
#endif //GENERATE_SIMPLE


#if GENERATE_SIMPLE

#define DEBUG 0
#define DEBUG_ADDRESS 0

#else //!GENERATE_SIMPLE

#include <unistd.h>
#include <execinfo.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>


//#define SIZE (1024*1024*16)
#define SIZE (1024*1024*16)
#define START 80
#define START_EXTRA 480

int p=START+START_EXTRA;





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
|  ?  | data[n*STEP+7] used to copy variables. see convert.py
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
| 0 | 0 | 0 | 0 | 0 | n*STEP+4 / OS_GODL
+---+---+---+---+---+
| 0 | 1 | 1 | 1 | 0 | n*STEP+5 / OS_GOCL
+---+---+---+---+---+
              ^
              | Data pointer

State B: Data pointer and code pointer point to the same block

      | Code pointer
      V
+---+---+---+---+---+
##################### < Cells with offset 0 - 3 and 6-7. We don't care about them for now
+---+---+---+---+---+
| 0 | 0 | 0 | 0 | 0 | n*STEP+4 / OS_GODL
+---+---+---+---+---+
| 0 | 0 | 0 | 0 | 0 | n*STEP+5 / OS_GOCL
+---+---+---+---+---+
      ^
      | Data pointer


State C: Data pointer is to the left of the code pointer:

              | Code pointer
              V
+---+---+---+---+---+
##################### < Cells with offset 0 - 3 and 6-7. We don't care about them for now
+---+---+---+---+---+
| 0 | 1 | 1 | 1 | 0 | n*STEP+4 / OS_GODL
+---+---+---+---+---+
| 0 | 0 | 0 | 0 | 0 | n*STEP+5 / OS_GOCL
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

//Used to change between while(n!=0) and while(n)
//The later is simpler and should in theory be prefered. But c2bf doesn't support that for some
// strange reason. But when we create our own compiler, we may only support while(n) to make it 
// simpler
#define WHILE(n) while(n)

#define IF_START(OFFSET)                         \
  data[p+OS_IF_W] = data[p+OFFSET];              \
  WHILE( data[p+OS_IF_W] )                       \
    {

#define IF_END()                                 \
    data[p+OS_IF_W] = 0;                         \
  }                                              \


#define IF_EQUAL_START(OFFSET,VALUE)             \
  data[p+OS_EQU0] = data[p+OFFSET] - VALUE;      \
  data[p+OS_IF_W] = 1;                           \
  WHILE( data[p+OS_EQU0] )                       \
    { data[p+OS_EQU0]=0; data[p+OS_IF_W]=0; }    \
  WHILE( data[p+OS_IF_W] )                       \
    {

#define IF_0(OFFSET)                             \
  data[p+OS_EQU0] = data[p+OFFSET];              \
  data[p+OS_IF_W] = 1;                           \
  WHILE( data[p+OS_EQU0] )                       \
    { data[p+OS_EQU0]=0; data[p+OS_IF_W]=0; }    \
  WHILE( data[p+OS_IF_W] )                       \
    {


//Goes from the code position to the data position. NOOP when we are already at the data location
#define GO_DATA()                  \
  /* Handle State A */             \
  WHILE( data[p+STEP+OS_GOCL] )    \
    { p = p+STEP; }                \
  /* Handle State C */             \
  WHILE( data[p-STEP+OS_GODL] )    \
          { p = p-STEP; }          \

//Goes from the data position to the code position. NOOP when we are already at the code location
#define GO_CODE()                  \
  /* Handle State A */             \
  WHILE( data[p-STEP+OS_GOCL] )    \
    { p = p-STEP; }                \
  /* Handle State C */             \
  WHILE( data[p+STEP+OS_GODL] )    \
          { p = p+STEP; }          \

//Move the data pointer 1 to the right (increment). Use it when we are already at the data pointer
// location. Use it for command >
#define MOVE_DATA_R()                            \
  data[p+OS_IF_W] = data[p+OS_GODL];             \
  data[p+OS_EQU0]=1;                             \
  /* if State C (OS_GODL is not 0)*/             \
  WHILE( data[p+OS_IF_W] )                       \
    {                                            \
      data[p+OS_GODL]=0;                         \
      p=p+STEP;                                  \
      IF_0( OS_GODL+STEP ) /* new state is B*/   \
          data[p+OS_GODL]=0;                     \
      IF_END()                                   \
      /*Don't execute the next while */          \
      /* to simulate a else statement */         \
      data[p+OS_EQU0]=0;                         \
      /* End while to simulate if*/              \
      data[p+OS_IF_W]=0;                         \
    }                                            \
  /* else (state wasn't C) */                    \
  WHILE( data[p+OS_EQU0] )                       \
    { /*New state is A */                        \
      data[p+OS_GOCL]=1;                         \
      p=p+STEP;                                  \
      data[p+OS_GOCL]=1;                         \
      data[p+OS_EQU0]=0;                         \
    }                                            \

//Move the data pointer 1 to the left (decrement). Use it when we are already at the data pointer
// location. Use it for command <
#define MOVE_DATA_L()                            \
  data[p+OS_IF_W] = data[p+OS_GOCL];             \
  data[p+OS_EQU0]=1;                             \
  /* if State A (OS_GOCL is not 0)*/             \
  WHILE( data[p+OS_IF_W] )                       \
    {                                            \
      data[p+OS_GOCL]=0;                         \
      p=p-STEP;                                  \
      IF_0( OS_GOCL-STEP ) /* new state is B*/   \
          data[p+OS_GOCL]=0;                     \
      IF_END()                                   \
      /*Don't execute the next while */          \
      /* to simulate a else statement */         \
      data[p+OS_EQU0]=0;                         \
      /* End while to simulate if*/              \
      data[p+OS_IF_W]=0;                         \
    }                                            \
  /* else (state wasn't A) */                    \
  WHILE( data[p+OS_EQU0] )                       \
    { /*New state is C */                        \
      data[p+OS_GODL]=1;                         \
      p=p-STEP;                                  \
      data[p+OS_GODL]=1;                         \
      data[p+OS_EQU0]=0;                         \
    }                                            \

//Move the code pointer 1 to the right (increment). Use it when we are already at the data pointer
// location. use it when go from [ back to the ] or when finish the current command
#define MOVE_CODE_R()                            \
  data[p+OS_IF_W] = data[p+OS_GOCL];             \
  data[p+OS_EQU0]=1;                             \
  /* if State A (OS_GOCL is not 0)*/             \
  WHILE( data[p+OS_IF_W] )                       \
    {                                            \
      data[p+OS_GOCL]=0;                         \
      p=p+STEP;                                  \
      IF_0( OS_GOCL+STEP ) /* new state is B*/   \
          data[p+OS_GOCL]=0;                     \
      IF_END()                                   \
      /*Don't execute the next while */          \
      /* to simulate a else statement */         \
      data[p+OS_EQU0]=0;                         \
      /* End while to simulate if*/              \
      data[p+OS_IF_W]=0;                         \
    }                                            \
  /* else (state wasn't A) */                    \
  WHILE( data[p+OS_EQU0] )                       \
    { /*New state is C */                        \
      data[p+OS_GODL]=1;                         \
      p=p+STEP;                                  \
      data[p+OS_GODL]=1;                         \
      data[p+OS_EQU0]=0;                         \
    }                                            \


//Move the code pointer 1 to the left (decrement). Use it when we are already at the data pointer
// location. use it when go from ] back to the [
#define MOVE_CODE_L()                            \
  data[p+OS_IF_W] = data[p+OS_GODL];             \
  data[p+OS_EQU0]=1;                             \
  /* if State C (OS_GODL is not 0)*/             \
  WHILE( data[p+OS_IF_W] )                       \
    {                                            \
      data[p+OS_GODL]=0;                         \
      p=p-STEP;                                  \
      IF_0( OS_GODL-STEP ) /* new state is B*/   \
          data[p+OS_GODL]=0;                     \
      IF_END()                                   \
      /*Don't execute the next while */          \
      /* to simulate a else statement */         \
      data[p+OS_EQU0]=0;                         \
      /* End while to simulate if*/              \
      data[p+OS_IF_W]=0;                         \
    }                                            \
  /* else (state wasn't C) */                    \
  WHILE( data[p+OS_EQU0] )                       \
    { /*New state is A */                        \
      data[p+OS_GOCL]=1;                         \
      p=p-STEP;                                  \
      data[p+OS_GOCL]=1;                         \
      data[p+OS_EQU0]=0;                         \
    }                                            \


#if !GENERATE_SIMPLE
int main()
  {
#endif
    #if !GENERATE_SIMPLE
      memset(data,0,sizeof data);
    #else
      p=p+8;
    #endif // GENERATE_SIMPLE
    #if DEBUG_ADDRESS
      fprintf(stderr,"DATA START %p END %p\n",(void*)data,(void*)(data+sizeof data));
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
    WHILE( data[p+OS_CODE] )
      {
        p = p + STEP;
        data[p+OS_CODE] = read_char();
      }
    p = p - STEP;
    WHILE( data[p+OS_CODE] ) //Go back to cell 0
      { p = p - STEP; }
    p = p + STEP;

    //We loop till we reach the end of the code
    WHILE( data[p+OS_CODE] )
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
        IF_EQUAL_START( OS_CODE, '+' )
            GO_DATA()
            data[p+OS_DATA] = data[p+OS_DATA] + 1;
            GO_CODE()
        IF_END()

        //#############
        //# Handle -  # Decrement current cell
        //#############
        IF_EQUAL_START( OS_CODE, '-' )
            GO_DATA()
            data[p+OS_DATA] = data[p+OS_DATA] - 1;
            GO_CODE()
        IF_END()

        //#############
        //# Handle ,  # Read input to current cell
        //#############
        IF_EQUAL_START( OS_CODE, ',' )
            GO_DATA()
            data[p+OS_DATA] = read_char();
            GO_CODE()
        IF_END()

        //#############
        //# Handle .  # output current cell
        //#############
        IF_EQUAL_START( OS_CODE, '.' )
            GO_DATA()
            write_char( data[p+OS_DATA] );
            GO_CODE()
          IF_END()

        //#############
        //# Handle <  # go a cell to the left (decrement cell pointer)
        //#############
        IF_EQUAL_START( OS_CODE, '<' )
            GO_DATA()
            MOVE_DATA_L()
            GO_CODE()
        IF_END()

        //#############
        //# Handle >  # go a cell to the right (increment cell pointer)
        //#############
        IF_EQUAL_START( OS_CODE, '>' )
            GO_DATA()
            MOVE_DATA_R()
            GO_CODE()
        IF_END()

        //#############
        //# Handle [  # jump to the corresponding ] when current cell is 0
        //#############
        data[p+OS_EQU0] = data[p+OS_CODE] - '[';
        data[p+OS_IF_W] = data[p+OS_EQU0];
        WHILE( data[p+OS_IF_W] )
          { data[p+OS_EQU0]=1; data[p+OS_IF_W]=0; }
        data[p+OS_EQU0] = data[p+OS_EQU0]-1;
        WHILE( data[p+OS_EQU0] ) // command is [
          {
            GO_DATA()

            IF_0( OS_DATA ) //When cell is 0 goto ]
                GO_CODE()
                data[p+OS_DEEP] = 1;
                WHILE( data[p+OS_DEEP] )
                  {
                    data[p+STEP+OS_DEEP] = data[p+OS_DEEP];
                    MOVE_CODE_R()

                    IF_EQUAL_START( OS_CODE, '[' )
                      data[p+OS_DEEP] = data[p+OS_DEEP]+1;
                    IF_END()

                    IF_EQUAL_START( OS_CODE, ']' )
                      data[p+OS_DEEP] = data[p+OS_DEEP]-1;
                    IF_END()
                  }
            IF_END()

            GO_CODE()
            data[p+OS_EQU0] = 0;
            data[p+OS_IF_W] = 1;
          }

        //Only go to the next while loop when we weren't in the one before
        //This is because we end up at ] in case we entered the one before and the value was 0
        //But there is no need to check it again
        //Technically not needed, since we skip this one when cell value is 0 but that takes a long
        // time
        data[p+OS_IF_W] = data[p+OS_IF_W]-1;
        WHILE( data[p+OS_IF_W] )
          {
            //#############
            //# Handle ]  # jump to the corresponding [ when current cell is not 0
            //#############
            IF_EQUAL_START( OS_CODE, ']' )
                GO_DATA()

                IF_START( OS_DATA ) //Jump to [ when cell is not 0
                    GO_CODE()
                    data[p+OS_DEEP] = 1;
                    WHILE( data[p+OS_DEEP] )
                      {
                        data[p-STEP+OS_DEEP] = data[p+OS_DEEP];
                        MOVE_CODE_L()

                        IF_EQUAL_START( OS_CODE, '[' )
                          data[p+OS_DEEP] = data[p+OS_DEEP]-1;
                        IF_END()

                        IF_EQUAL_START( OS_CODE, ']' )
                          data[p+OS_DEEP] = data[p+OS_DEEP]+1;
                        IF_END()
                      }
                 IF_END()
                GO_CODE()
            IF_END()
          }
        MOVE_CODE_R()
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


