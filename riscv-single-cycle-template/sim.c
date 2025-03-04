#include <stdio.h>
#include "shell.h"

// Global variables for the fetched instruction and its decoded fields.
static uint32_t instruction;
static int opcode;
static int rd, rs1, rs2;
static int funct3, funct7;
static int imm;

// Fetch: Read a 32-bit instruction from memory using the current ProgramCpnuter.
void fetch() {
    // Read the instruction using a 64-bit address.
    instruction = mem_read_32((uint64_t)CURRENT_STATE.PC);
    // Debug: print the fetched instruction.
    printf("Fetch instruction 0x%08X from PC = 0x%08X\n", instruction, CURRENT_STATE.PC);
    // Update PC (just add 4).
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

// Decode: Extract the fields from the instruction.
void decode() {
    // Clear all fields.
    rd = rs1 = rs2 = funct3 = funct7 = imm = 0;
    // Extract the opcode from bits [6:0].
    opcode = instruction & 0x7F;

    // If the instruction is all zeros, treat it as HLT.
    if (opcode == 0x00) {
        RUN_BIT = 0;
        return;
    }

    switch (opcode) {
        case 0x37:  // LUI (U-type): rd = immediate.
            break;
 
        case 0x17:  // AUIPC (U-type): rd = PC + immediate.
            break;

        case 0x13:  // I-type instructions (e.g., ADDI, SLLI).
            break;

        case 0x33:  // R-type: arithmetic/logic instructions.
            break;

        case 0x23:  // S-type: store instructions (e.g., SW).
            break;

        case 0x63:  // B-type: branch instructions.
            break;

        default:
            printf("Decode: Unknown or unimplemented opcode: 0x%02X\n", opcode);
            break;
    }

    // Debug: print decoded opcode and fields.
    printf("Decoded: opcode=0x%02X, rd=%d, rs1=%d, rs2=%d, funct3=0x%X, funct7=0x%X, imm=0x%X\n",
           opcode, rd, rs1, rs2, funct3, funct7, imm);
}

// Execute: Update the state according to the decoded instruction.
void execute() {
    switch (opcode) {
        case 0x37:  // LUI: rd = immediate.
            break;

        case 0x17:  // AUIPC: rd = CURRENT_STATE.PC + immediate.
            break;

        case 0x13:  // I-type instructions.
            break;

        case 0x33:  // R-type instructions.
            break;

        case 0x23:  // S-type: store word.
            break;

        case 0x63:  // B-type: branch instructions.
            break;

        default:
            printf("Execute: Opcode 0x%02X not implemented.\n", opcode);
            break;
    }
}

// process_instruction: Fetch, decode, and execute one instruction.
void process_instruction() {
    // Begin with a fresh copy of the current state.
	NEXT_STATE = CURRENT_STATE;
	fetch();
	decode();
    // If HLT (opcode==0) was encountered, stop the simulation.
	if (opcode == 0x00) {
		printf("HLT encountered. Halting simulation.\n");
		RUN_BIT = 0;
		return;
		}
		execute();
}
