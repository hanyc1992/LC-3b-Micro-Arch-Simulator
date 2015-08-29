/*
    Remove all unnecessary lines (including this one)
    in this comment.
    REFER TO THE SUBMISSION INSTRUCTION FOR DETAILS

    Name 1: Full name of the first partner
    Name 2: Full name of the second partner
    UTEID 1: UT EID of the first partner
    UTEID 2: UT EID of the second partner
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
*/

#define WORDS_IN_MEM    0x08000
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
  int address; /* this is a byte address */

  printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
  int k;

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');                         /////////////////////////////////////dont no why////////////////////
  }
  CURRENT_LATCHES.Z = 1;
  NEXT_LATCHES = CURRENT_LATCHES;

  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/



void process_instruction(){
  /*  function: process_instruction
   *
   *    Process one instruction at a time
   *       -Fetch one instruction
   *       -Decode
   *       -Execute
   *       -Update NEXT_LATCHES
   */

   void function_add(int, int);
   void function_and(int, int);
   void function_br(int, int);
   void function_jmp(int, int);
   void function_jsr(int, int);
   void function_ldb(int, int);
   void function_ldw(int, int);
   void function_lea(int, int);
   void function_rti(int, int);
   void function_shf(int, int);
   void function_stb(int, int);
   void function_stw(int, int);
   void function_trap(int, int);
   void function_xor(int, int);


   int instruction_low8, instruction_high8;



   instruction_low8  = MEMORY[CURRENT_LATCHES.PC>>1][0]&0x00ff;
   instruction_high8 = MEMORY[CURRENT_LATCHES.PC>>1][1]&0x00ff;

   switch(instruction_high8 & 0x00F0)
   {
       case 0x0010: function_add(instruction_low8,instruction_high8);break;
       case 0x0050: function_and(instruction_low8,instruction_high8);break;
       case 0x0000: function_br(instruction_low8,instruction_high8);break;
       case 0x00c0: function_jmp(instruction_low8,instruction_high8);break;
       case 0x0040: function_jsr(instruction_low8,instruction_high8);break;
       case 0x0020: function_ldb(instruction_low8,instruction_high8);break;
       case 0x0060: function_ldw(instruction_low8,instruction_high8);break;
       case 0x00e0: function_lea(instruction_low8,instruction_high8);break;
       case 0x0080: function_rti(instruction_low8,instruction_high8);break;
       case 0x00d0: function_shf(instruction_low8,instruction_high8);break;
       case 0x0030: function_stb(instruction_low8,instruction_high8);break;
       case 0x0070: function_stw(instruction_low8,instruction_high8);break;
       case 0x00f0: function_trap(instruction_low8,instruction_high8);break;
       case 0x0090: function_xor(instruction_low8,instruction_high8);break;
       default: printf("cannot process!!!");break;
   }

}


void function_add(int instruction_low8,int instruction_high8)
{
    printf("function_add\n");
    void setcc(int x);
    int sext(int data,int num);
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;

    int dr,sr1,sr2,imm5;
    int dr_store;
    dr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    sr1=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    sr2=instruction_low8&0x0007;        //instruction 2~0
    imm5=instruction_low8&0x001f;       //instruction 4~0

    if (!(instruction_low8&0x0020))
    {
        dr_store=sext(CURRENT_LATCHES.REGS[sr1],16) + sext(CURRENT_LATCHES.REGS[sr2],16);
        NEXT_LATCHES.REGS[dr]=Low16bits(dr_store);
    }
    else
    {
        dr_store=sext(CURRENT_LATCHES.REGS[sr1],16) + sext(imm5,5);
        NEXT_LATCHES.REGS[dr]=Low16bits(dr_store);
    }
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
    setcc(dr_store);
}

void function_and(int instruction_low8,int instruction_high8)
{
    printf("function_and\n");
    void setcc(int x);
    int sext(int data,int num);


    int dr,sr1,sr2,imm5;
    int dr_store;
    dr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    sr1=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    sr2=instruction_low8&0x0007;        //instruction 2~0
    imm5=instruction_low8&0x001f;       //instruction 4~0

    if (!(instruction_low8&0x0020))
    {
        dr_store=sext(CURRENT_LATCHES.REGS[sr1],16) & sext(CURRENT_LATCHES.REGS[sr2],16);
        NEXT_LATCHES.REGS[dr]=Low16bits(dr_store);
    }
    else
    {
        dr_store=sext(CURRENT_LATCHES.REGS[sr1],16)^sext(imm5,5);
        NEXT_LATCHES.REGS[dr]=Low16bits(dr_store);
    }
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
    setcc(dr_store);
}

void function_br(int instruction_low8,int instruction_high8)
{
    printf("function_br\n");
    void setcc(int x);
    int sext(int data,int num);

    int pcoffset9=((instruction_high8&0x0001)<<8)+instruction_low8;      //instruction 8~0
    int n=(instruction_high8>>3)&0x0001;
    int z=(instruction_high8>>2)&0x0001;
    int p=(instruction_high8>>1)&0x0001;
    int ben=(n&&CURRENT_LATCHES.N) || (z&&CURRENT_LATCHES.Z) ||(p&&CURRENT_LATCHES.P);

    if (ben) NEXT_LATCHES.PC=Low16bits(CURRENT_LATCHES.PC+2+(sext(pcoffset9,9)<<1));
    else NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;

}

void function_jmp(int instruction_low8,int instruction_high8)
{
    printf("function_jmp\n");
    void setcc(int x);
    int sext(int data,int num);

    int baser=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    NEXT_LATCHES.PC=CURRENT_LATCHES.REGS[baser];
}

void function_jsr(int instruction_low8,int instruction_high8)
{
    printf("function_jsr\n");
    void setcc(int x);
    int sext(int data,int num);

    int pcoffset11;
    int baser;
    baser=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    pcoffset11=(instruction_high8&0x0007)<<8+instruction_low8;      //instruction 10~0

    NEXT_LATCHES.REGS[7]=CURRENT_LATCHES.PC+2;
    if (!(instruction_high8&0x0008)) NEXT_LATCHES.PC=CURRENT_LATCHES.REGS[baser];
    else NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2+Low16bits(sext(pcoffset11,11)<<1);

}

void function_ldb(int instruction_low8,int instruction_high8)
{
    printf("function_ldb\n");
    void setcc(int x);
    int sext(int data,int num);

    int dr,baser,offset6;
    dr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    baser=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    offset6=instruction_low8&0x003f;       //instruction 5~0

    int mem_byte_addr=sext(offset6,6)+CURRENT_LATCHES.REGS[baser];
    NEXT_LATCHES.REGS[dr]=Low16bits(sext((MEMORY[mem_byte_addr>>1][mem_byte_addr&0x0001])&0x00ff,8));
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
}

void function_ldw(int instruction_low8,int instruction_high8)
{
    printf("function_ldw\n");
    void setcc(int x);
    int sext(int data,int num);

    int dr,baser,offset6;
    dr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    baser=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    offset6=instruction_low8&0x003f;       //instruction 5~0

    int mem_byte_addr=(sext(offset6,6)<<1)+CURRENT_LATCHES.REGS[baser];
    NEXT_LATCHES.REGS[dr]=(MEMORY[mem_byte_addr>>1][0]&0x00ff)+((MEMORY[mem_byte_addr>>1][1]<<8)&0xff00);
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
    setcc(NEXT_LATCHES.REGS[dr]);
}

void function_lea(int instruction_low8,int instruction_high8)
{
    printf("function_lea\n");
    void setcc(int x);
    int sext(int data,int num);
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;

    int dr;
    int pcoffset9;
    dr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    pcoffset9=((instruction_high8&0x0001)<<8)+instruction_low8;      //instruction 8~0

    NEXT_LATCHES.REGS[dr]=CURRENT_LATCHES.PC+2+(sext(pcoffset9,9)<<1);
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
}

void function_rti(int instruction_low8,int instruction_high8)
{
    printf("function_rti\n");
    void setcc(int x);
    int sext(int data,int num);
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
}

void function_shf(int instruction_low8,int instruction_high8)
{
    printf("function_shf\n");
    void setcc(int x);
    int sext(int data,int num);


    int dr,sr,amount4;
    dr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    sr=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    amount4=instruction_low8&0x000f;       //instruction 3~0
    int signal=(instruction_low8&0x0030)>>4;

    switch(signal)
    {
        case 0:NEXT_LATCHES.REGS[dr]=Low16bits(CURRENT_LATCHES.REGS[sr]<<amount4);break;
        case 1:NEXT_LATCHES.REGS[dr]=Low16bits(CURRENT_LATCHES.REGS[sr])>>amount4;break;
        case 3:NEXT_LATCHES.REGS[dr]=Low16bits(sext(CURRENT_LATCHES.REGS[sr],16)>>amount4);break;
        default: printf("shf wrong!!!");break;
    }
    setcc(sext(NEXT_LATCHES.REGS[dr],16));
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
}

void function_stb(int instruction_low8,int instruction_high8)
{
    printf("function_stb\n");
    void setcc(int x);
    int sext(int data,int num);

    int sr,baser,offset6;
    sr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    baser=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    offset6=instruction_low8&0x003f;       //instruction 5~0

    int mem_byte_addr=sext(offset6,6)+CURRENT_LATCHES.REGS[baser];

    MEMORY[mem_byte_addr>>1][mem_byte_addr&0x0001]=CURRENT_LATCHES.REGS[sr]&0x000f;
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
}

void function_stw(int instruction_low8,int instruction_high8)
{
    printf("function_stw\n");
    void setcc(int x);
    int sext(int data,int num);

    int sr,baser,offset6;
    sr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    baser=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    offset6=instruction_low8&0x003f;       //instruction 5~0

    int mem_byte_addr=(sext(offset6,6)<<1)+CURRENT_LATCHES.REGS[baser];

    MEMORY[mem_byte_addr>>1][0]=CURRENT_LATCHES.REGS[sr]&0x00ff;
    MEMORY[mem_byte_addr>>1][1]=(CURRENT_LATCHES.REGS[sr]>>8)&0x00ff;
    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;

}

void function_trap(int instruction_low8,int instruction_high8)
{
    printf("function_trap\n");

    int trapvect8 = Low16bits( instruction_low8 + instruction_high8<<8 );

    NEXT_LATCHES.REGS[7] = CURRENT_LATCHES.PC+2;
    NEXT_LATCHES.PC = Low16bits( MEMORY[trapvect8][0] + MEMORY[trapvect8][1]<<8 );
}

void function_xor(int instruction_low8,int instruction_high8)
{
    printf("function_xor\n");
    void setcc(int x);
    int sext(int data,int num);

    int dr,sr1,sr2,imm5;
    int dr_store;
    dr=(instruction_high8>>1)&0x0007;   //Instruction 11~9
    sr1=((instruction_high8&0x0001)<<2)+(instruction_low8&0x00c0)>>6;      //instruction 8~6
    sr2=instruction_low8&0x0007;        //instruction 2~0
    imm5=instruction_low8&0x001f;       //instruction 4~0

    if (!(instruction_low8&0x0020))
    {
        dr_store=sext(CURRENT_LATCHES.REGS[sr1],16)^sext(CURRENT_LATCHES.REGS[sr2],16);
        NEXT_LATCHES.REGS[dr]=Low16bits(dr_store);
    }
    else
    {
        dr_store=sext(CURRENT_LATCHES.REGS[sr1],16)^sext(imm5,5);
        NEXT_LATCHES.REGS[dr]=Low16bits(dr_store);
    }

    NEXT_LATCHES.PC=CURRENT_LATCHES.PC+2;
    setcc(dr_store);
}

void setcc(int x)           //x must in 32 digits
{
    if(x>0) {NEXT_LATCHES.P=1;NEXT_LATCHES.N=0;NEXT_LATCHES.Z=0;}
    if(x==0) {NEXT_LATCHES.P=0;NEXT_LATCHES.N=0;NEXT_LATCHES.Z=1;}
    if(x<0) {NEXT_LATCHES.P=0;NEXT_LATCHES.N=1;NEXT_LATCHES.Z=0;}
}

int sext(int data,int num)
{
    int sign=(data>>(num-1)&0x0001);
    if (sign) data=data-(0x0001<<num);

    return data;
}
