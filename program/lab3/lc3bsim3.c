/*
    Name 1: YUCHAO HAN
    UTEID 1: yh7483
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

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
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM    0x08000
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {

  eval_micro_sequencer();
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
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
/* Purpose   : Simulate the LC-3b until HALTed.                 */
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
    fflush(dumpsim_file);
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
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
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
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
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

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) {
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

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
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

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

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/
int cycle_memory_count = 0;

int sext(int data,int num)
{
    int sign=(data>>(num-1)&0x0001);
    if (sign) data=data-(0x0001<<num);

    return data;
}

int my_ird;
int my_cond;
int my_j;
int my_ld_mar;
int my_ld_mdr;
int my_ld_ir;
int my_ld_ben;
int my_ld_reg;
int my_ld_cc;
int my_ld_pc;
int my_gate_pc;
int my_gate_mdr;
int my_gate_alu;
int my_gate_marmux;
int my_gate_shf;
int my_pcmux;
int my_drmux;
int my_sr1mux;
int my_addr1mux;
int my_addr2mux;
int my_marmux;
int my_aluk;
int my_mioen;
int my_rw;
int my_data_size;
int my_lshf1;

void get_signal_val()
{
    my_ird = GetIRD(CURRENT_LATCHES.MICROINSTRUCTION);
    my_cond = GetCOND(CURRENT_LATCHES.MICROINSTRUCTION);
    my_j = GetJ(CURRENT_LATCHES.MICROINSTRUCTION);
    my_ld_mar = GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION);
    my_ld_mdr = GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
    my_ld_ir = GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION);
    my_ld_ben = GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION);
    my_ld_reg = GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION);
    my_ld_cc = GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION);
    my_ld_pc = GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION);
    my_gate_pc = GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION);
    my_gate_mdr = GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION);
    my_gate_alu = GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION);
    my_gate_marmux = GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    my_gate_shf = GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION);
    my_pcmux = GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    my_drmux = GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    my_sr1mux = GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    my_addr1mux = GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    my_addr2mux = GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION);
    my_marmux = GetMARMUX(CURRENT_LATCHES.MICROINSTRUCTION);
    my_aluk = GetALUK(CURRENT_LATCHES.MICROINSTRUCTION);
    my_mioen = GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION);
    my_rw = GetR_W(CURRENT_LATCHES.MICROINSTRUCTION);
    my_data_size = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
    my_lshf1 = GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION);
}

int my_marmux_in0;
int my_marmux_out;
int my_addr2mux_out;
int my_adder_in0;
int my_adder_out;
int my_sr1;
int my_sr2;
int my_dr;
int my_sr1_out;
int my_sr2_out;
int my_addr1mux_out;
int my_pcmux_out;
int my_sr2mux_in1;
int my_sr2mux_out;
int my_alu_out;
int my_shf_out;
int logic0_outn;
int logic0_outz;
int logic0_outp;


int my_miomux_in0;
int my_miomux_in1;
int my_miomux_out;

int logic2_mux1_out;
int logic2_mux2_in0;
int logic2_mux2_in1;
int logic2_mux2_out;

int logic1_mux_in0;
int logic1_mux_in1;
int logic1_mux_out;

int my_we0;
int my_we1;


void get_internal_signal()
{
    my_marmux_in0=(CURRENT_LATCHES.IR & 0x00ff)<<1;
    switch (my_addr2mux)
    {
        case 0:{my_addr2mux_out=0;break;}
        case 1:{my_addr2mux_out=sext(CURRENT_LATCHES.IR&0x003f,6);break;}
        case 2:{my_addr2mux_out=sext(CURRENT_LATCHES.IR&0x01ff,9);break;}
        case 3:{my_addr2mux_out=sext(CURRENT_LATCHES.IR&0x07ff,11);break;}
        default:{printf("addr2mux error!");break;}
    }
    my_adder_in0=my_addr2mux_out<<(my_lshf1);

    my_sr1=my_sr1mux?((CURRENT_LATCHES.IR>>6)&0x0007):((CURRENT_LATCHES.IR>>9)&0x0007);
    my_sr1_out=CURRENT_LATCHES.REGS[my_sr1];
    my_sr2=CURRENT_LATCHES.IR&0x0007;
    my_sr2_out=CURRENT_LATCHES.REGS[my_sr2];
    my_addr1mux_out=my_addr1mux?my_sr1_out:CURRENT_LATCHES.PC;
    my_adder_out=my_adder_in0+my_addr1mux_out;
    my_marmux_out=my_marmux?my_adder_out:my_marmux_in0;
    switch (my_pcmux)
    {
        case 0:{my_pcmux_out=CURRENT_LATCHES.PC+2;break;}
        case 1:{my_pcmux_out=BUS;break;}
        case 2:{my_pcmux_out=my_adder_out;break;}
        default:{printf("pcmux error!");break;}
    }
    my_sr2mux_in1=sext(CURRENT_LATCHES.IR&0x001f,5);
    my_sr2mux_out=((CURRENT_LATCHES.IR>>5)&0x0001)?my_sr2mux_in1:my_sr2_out;
    switch(my_aluk)
    {
        case 0:{my_alu_out=Low16bits(my_sr1_out+my_sr2mux_out);break;}
        case 1:{my_alu_out=Low16bits(my_sr1_out&my_sr2mux_out);break;}
        case 2:{my_alu_out=Low16bits(my_sr1_out^my_sr2mux_out);break;}
        case 3:{my_alu_out=my_sr1_out;break;}
        default:{printf("alu error!");break;}
    }
    switch((CURRENT_LATCHES.IR>>4)&0x0003)
    {
        case 0:{my_shf_out=Low16bits(my_sr1_out<<(CURRENT_LATCHES.IR&0x000f));break;}/*lshf*/
        case 1:{my_shf_out=Low16bits(my_sr1_out>>(CURRENT_LATCHES.IR&0x000f));break;}/*rshfl*/
        case 3:{my_shf_out=Low16bits(sext(my_sr1_out,16)>>(CURRENT_LATCHES.IR&0x000f));break;}/*rshfa*/
        default:{break;}
    }
    if(sext(BUS,16)>0){logic0_outn=0;logic0_outp=1;logic0_outz=0;}
    if(sext(BUS,16)==0){logic0_outn=0;logic0_outp=0;logic0_outz=1;}
    if(sext(BUS,16)<0){logic0_outn=1;logic0_outp=0;logic0_outz=0;}



    if (my_rw==0 && my_mioen==1)    my_miomux_in1=(MEMORY[CURRENT_LATCHES.MAR>>1][0]&0x00ff)+((MEMORY[CURRENT_LATCHES.MAR>>1][1]<<8)&0xff00);

    logic2_mux1_out=(CURRENT_LATCHES.MAR&0x0001)?((CURRENT_LATCHES.MDR>>8)&0x00ff):(CURRENT_LATCHES.MDR&0x00ff);
    logic2_mux2_in0=sext(logic2_mux1_out,8);
    logic2_mux2_in1=sext((CURRENT_LATCHES.MDR&0xffff),16);
    logic2_mux2_out=Low16bits(my_data_size?(logic2_mux2_in1):(logic2_mux2_in0));

    logic1_mux_in1=BUS&0xffff;
    logic1_mux_in0=(BUS&0x00ff)+((BUS&0x00ff)<<8);
    logic1_mux_out=my_data_size?logic1_mux_in1:logic1_mux_in0;

    my_miomux_out=my_mioen?my_miomux_in1:logic1_mux_out;

    if (my_rw==1 && my_data_size==1 && (CURRENT_LATCHES.MAR&0x0001)==0) {my_we0=1;my_we1=1;}
    if (my_rw==1 && my_data_size==1 && (CURRENT_LATCHES.MAR&0x0001)==1) {printf("store error!");}
    if (my_rw==1 && my_data_size==0 && (CURRENT_LATCHES.MAR&0x0001)==0) {my_we0=1;my_we1=0;}
    if (my_rw==1 && my_data_size==0 && (CURRENT_LATCHES.MAR&0x0001)==1) {my_we0=0;my_we1=1;}
/*
    if((my_we0==1) && (CURRENT_LATCHES.READY==1))
        MEMORY[CURRENT_LATCHES.MAR>>1][0]=CURRENT_LATCHES.MDR&0x00ff;
    if((my_we1==1) && (CURRENT_LATCHES.READY==1))
        MEMORY[CURRENT_LATCHES.MAR>>1][1]=(CURRENT_LATCHES.MDR>>8)&0x00ff;
*/

    my_dr=my_drmux?(7):((CURRENT_LATCHES.IR>>9)&0x0007);
}




void eval_micro_sequencer()
{

  /*
   * Evaluate the address of the next state according to the
   * micro sequencer logic. Latch the next microinstruction.
   */
    get_signal_val();

    int my_ben = CURRENT_LATCHES.BEN;
    int my_ready = CURRENT_LATCHES.READY;
    int my_ir11 = (CURRENT_LATCHES.IR >> 11) & 0x0001;
    int my_ir15_12 = (CURRENT_LATCHES.IR >> 12) & 0x000f;

    if (my_ird)
        NEXT_LATCHES.STATE_NUMBER = my_ir15_12;
    else
        switch (my_cond)
        {
        case 0:
            NEXT_LATCHES.STATE_NUMBER = my_j;break;
        case 1:
            NEXT_LATCHES.STATE_NUMBER = my_j + (my_ready << 1);break;
        case 2:
            NEXT_LATCHES.STATE_NUMBER = my_j + (my_ben << 2);break;
        case 3:
            NEXT_LATCHES.STATE_NUMBER = my_j + my_ir11;break;
        default:
            printf("cond error!");break;
        }

    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

}


void cycle_memory() {

  /*
   * This function emulates memory and the WE logic.
   * Keep track of which cycle of MEMEN we are dealing with.
   * If fourth, we need to latch Ready bit at the end of
   * cycle to prepare microsequencer for the fifth cycle.
   */

    NEXT_LATCHES.READY = 0;

    if ((my_cond == 1) && (cycle_memory_count < 4))
        {cycle_memory_count++;      NEXT_LATCHES.READY = 0;}
    if ((my_cond == 1) && (cycle_memory_count >= 4))
        {cycle_memory_count = -1;    NEXT_LATCHES.READY = 1;}
}



void eval_bus_drivers() {

  /*
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */
    get_signal_val();
    get_internal_signal();
}


void drive_bus() {

  /*
   * Datapath routine for driving the bus from one of the 5 possible
   * tristate drivers.
   */

    BUS=0;
    if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        BUS=Low16bits(my_marmux_out);
    }

    if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        BUS=Low16bits(CURRENT_LATCHES.PC);
    }

    if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        BUS=Low16bits(my_alu_out);
    }

    if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        BUS=Low16bits(my_shf_out);
    }

    if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        BUS=Low16bits(logic2_mux2_out);
    }

}


void latch_datapath_values() {

  /*
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come
   * after drive_bus.
   */
    get_internal_signal();

    if(my_ld_mar)   NEXT_LATCHES.MAR=Low16bits(BUS);
    if(my_ld_mdr&&((my_cond==1 && CURRENT_LATCHES.READY==1)||(my_cond==0)))   NEXT_LATCHES.MDR=my_miomux_out;

    if(my_ld_ir)    NEXT_LATCHES.IR=Low16bits(BUS);
    if(my_ld_ben)   NEXT_LATCHES.BEN=((CURRENT_LATCHES.N&((CURRENT_LATCHES.IR>>11)&0x0001)) + (CURRENT_LATCHES.Z&((CURRENT_LATCHES.IR>>10)&0x0001)) + (CURRENT_LATCHES.P&((CURRENT_LATCHES.IR>>9)&0x0001)));
    if(my_ld_reg)   NEXT_LATCHES.REGS[my_dr]=Low16bits(BUS);
    if(my_ld_cc)    {NEXT_LATCHES.N=logic0_outn;NEXT_LATCHES.Z=logic0_outz;NEXT_LATCHES.P=logic0_outp;}
    if(my_ld_pc)    NEXT_LATCHES.PC=my_pcmux_out;
    /*
    ###################################################################################
    */
    if(my_rw&&my_mioen&&(my_cond==1 && CURRENT_LATCHES.READY==1))
    {
        if(my_we0)
            MEMORY[CURRENT_LATCHES.MAR>>1][0]=CURRENT_LATCHES.MDR&0x00ff;
        if(my_we1)
            MEMORY[CURRENT_LATCHES.MAR>>1][1]=(CURRENT_LATCHES.MDR>>8)&0x00ff;
    }

}
