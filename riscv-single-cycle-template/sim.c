#include <stdio.h>
#include <stdlib.h>
#include "shell.h"
#define RISCV_REGS 32 

typedef struct CPU_State { 
  uint32_t PC;                /* program counter */ 
  uint32_t REGS[RISCV_REGS];   /* register file. */ 
  int FLAG_NV;        /* invalid */ 
  int FLAG_DZ;        /* divide by zero */ 
  int FLAG_OF;        /* overflow */ 
  int FLAG_UF;        /* underflow */ 
  int FLAG_NX;        /* inexact */
} CPU_State;  

/* STATE_CURRENT is the current arch. state */ 
/* STATE_NEXT is the resulting arch. state 
   after the current instruction is processed */ 
CPU_State STATE_CURRENT, STATE_NEXT; 
int RUN_BIT; /* initialized to 1; need to be changed to 0 if the HLT instruction is encountered */ 

//Variable Declarations
char instructionInput[100]; //store instruction read from .mem file
char registerTarget[5];
char registerOne[5];
char registerTwo[5];



const char *instructions[] = {
        "add",   // Addition
        "slt",   // Set less than
        "addi",  // Add immediate
        "slli",  // Shift left logical immediate
        "sw",    // Store word
        "bne",   // Branch if not equal
        "auipc", // Add upper immediate to program counter
        "jal"    // Jump and link
    };

void fetch()
{
    //local declarations
    int i = 0;
    FILE *file == fopen("instruction.mem", "r");//open instruction file


    if(file == NULL){ //if null file not opened

        perror("Failed to open file"); //print error
        return 1;

    }

    char hexInput[101];
    
    while(fgets(hexInput, sizeof(hexInput), file) != NULL){ //open file

        int len = 0;
        while(hexInput[len != '\0' && hexInput[len] != '\n']){ //get length of file in char

            len++;

        }

        if (hexInput[len] == '\n'){ //replace every new line

            hexInput[len] = '\0';

        }

        for(int i = 0; i < len ;  i++){ //input every 8 char into the instructionInput array

            instructionInput[i] = hexInput[i];

        }

        instructionInput[len] = '\0'; //null terminate the string




    }
} 

void decode()
{

}

void execute()
{
  
}

void process_instruction()
{
  /* execute one instruction here. You should use CURRENT_STATE and modify
   * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
   * access memory. */
  fetch();
  decode();
  execute();
}
