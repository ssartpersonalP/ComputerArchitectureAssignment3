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

void execute() {
    uint32_t rd = (instruction >> 7) & 0x1F;      // destination register (rd)
    uint32_t funct3 = (instruction >> 12) & 0x7;  // funct3 (3 bits)
    uint32_t rs1 = (instruction >> 15) & 0x1F;    // source register 1 (rs1)
    uint32_t rs2 = (instruction >> 20) & 0x1F;    // source register 2 (rs2)
    uint32_t funct7 = (instruction >> 25) & 0x7F;  // funct7 (7 bits)
    uint32_t immediate = imm;
    switch (opcode) {
        case 0x37:  // LUI: rd = immediate.
            CURRENT_STATE.REGS[rd] = immediate << 12;
            break;

        case 0x17:  // AUIPC: rd = CURRENT_STATE.PC + immediate.

            uint32_t shifted = immediate << 12;
            CURRENT_STATE.REGS[rd] = CURRENT_STATE.PC + shifted;
            break;

        case 0x13:  // I-type instructions.
            if (funct3 == 0x0) {
                // ADDI instruction (funct3 == 000)
                CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] + imm;
            }
            else if (funct3 == 0x1) {
                // SLI (Shift Left Immediate) instruction (funct3 == 001)
                CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] << imm;
            }
            else {
                printf("Unknown I-type instruction (funct3: %d)\n", funct3);
            }
            break;

        case 0x33:  // R-type instructions.
            if (funct3 == 0x0 && funct7 == 0x00) {
                // This is an ADD instruction.
                CURRENT_STATE.REGS[rd]  = rs1 + rs2;
                
            }
            else if (funct3 == 0x2 && funct7 == 0x00) {
                // This is an SLT instruction.
                CURRENT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs1] < CURRENT_STATE.REGS[rs2]) ? 1 : 0;
            }
            else {
                // Other R-type operations
                printf("Other R-type instruction\n");
            }
            break;

        case 0x23:  // S-type: store word.
            if (funct3 == 0x2) {
                // SW instruction (Store Word)
                uint32_t address = CURRENT_STATE.REGS[rs1] + imm;
                mem_write_32(address/4, CURRENT_STATE.REGS[rs2]);  // Store word in memory (dividing by 4 for word address)
                
            } 
            else {
                printf("Unknown store instruction (funct3: %d)\n", funct3);
            }
            break;

        case 0x63:  // B-type: branch instructions.
            if (funct3 == 0x1) {
                // BNE instruction (Branch if Not Equal)
                if (CURRENT_STATE.REGS[rs1] != CURRENT_STATE.REGS[rs2]) {
                    // If registers are not equal, branch
                    uint32_t branch_target = CURRENT_STATE.PC + (imm << 1); //branch address
                    CURRENT_STATE.PC = branch_target;
    
                } else {
                    // If equal, continue to the next instruction
                    CURRENT_STATE.PC += 4;
                }
            } else {
                printf("Unknown branch instruction (funct3: %d)\n", funct3);
            }
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
