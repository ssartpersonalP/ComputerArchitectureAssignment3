// Cesar Guevara
// Sean Sart
#include <stdio.h>
#include "shell.h"

// Global variables for the fetched instruction and its decoded fields
static uint32_t instruction;
static int opcode;
static int rd, rs1, rs2;
static int funct3, funct7;
static int imm;

// Fetch: Read a 32-bit instruction from memory using the current PC
void fetch() {
    // Read the instruction using a 64-bit address
    instruction = mem_read_32((uint64_t)CURRENT_STATE.PC);
    // Debug: print the fetched instruction.
    printf("Fetched instruction 0x%08X from PC = 0x%08X\n", instruction, CURRENT_STATE.PC);
    // Update PC (non-pipelined, so just add 4).
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
            rd  = (instruction >> 7) & 0x1F;
            imm = instruction & 0xFFFFF000;
            break;
            
        case 0x17:  // AUIPC (U-type): rd = PC + immediate.
            rd  = (instruction >> 7) & 0x1F;
            imm = instruction & 0xFFFFF000;
            break;
            
        case 0x13:  // I-type instructions (like ADDI and SLLI)
            rd     = (instruction >> 7) & 0x1F;
            funct3 = (instruction >> 12) & 0x07;
            rs1    = (instruction >> 15) & 0x1F;
            if (funct3 == 0x1) {  // SLLI: shift amount is in bits [24:20]
                imm    = (instruction >> 20) & 0x1F;  // shift amount
                funct7 = (instruction >> 25) & 0x7F;
            } else {
                imm = (instruction >> 20) & 0xFFF;
                // Sign-extend the 12-bit immediate
                if (imm & 0x800)
                    imm |= 0xFFFFF000;
            }
            break;
            
        case 0x33:  // R-type: arithmetic/logic instructions
            rd     = (instruction >> 7) & 0x1F;
            funct3 = (instruction >> 12) & 0x07;
            rs1    = (instruction >> 15) & 0x1F;
            rs2    = (instruction >> 20) & 0x1F;
            funct7 = (instruction >> 25) & 0x7F;
            break;
            
        case 0x23:  // S-type: store instructions (like  SW)
            funct3 = (instruction >> 12) & 0x07;
            rs1    = (instruction >> 15) & 0x1F;
            rs2    = (instruction >> 20) & 0x1F;
            {
                int imm11_5 = (instruction >> 25) & 0x7F;
                int imm4_0  = (instruction >> 7)  & 0x1F;
                imm = (imm11_5 << 5) | imm4_0;
                if (imm & 0x800)
                    imm |= 0xFFFFF000;
            }
            break;
            
        case 0x63:  // B-type: branch instructions
            funct3 = (instruction >> 12) & 0x07;
            rs1    = (instruction >> 15) & 0x1F;
            rs2    = (instruction >> 20) & 0x1F;
            {
                int bit12    = (instruction >> 31) & 0x1;
                int bit11    = (instruction >> 7)  & 0x1;
                int bits10_5 = (instruction >> 25) & 0x3F;
                int bits4_1  = (instruction >> 8)  & 0xF;
                imm = (bit12 << 12) | (bit11 << 11) | (bits10_5 << 5) | (bits4_1 << 1);
                if (imm & 0x1000)
                    imm |= 0xFFFFE000;  // sign-extend 13-bit immediate
            }
            break;
            
        default:
            printf("Decode: Unknown or unimplemented opcode: 0x%02X\n", opcode);
            break;
    }

    // Debug: print decoded opcode and fields
    printf("Decoded: opcode=0x%02X, rd=%d, rs1=%d, rs2=%d, funct3=0x%X, funct7=0x%X, imm=0x%X\n",
           opcode, rd, rs1, rs2, funct3, funct7, imm);
}

// Execute: Update the state according to the decoded instruction
void execute() {
    switch (opcode) {
	case 0x00:
            printf("HLT encountered. Halting simulation.\n");
            RUN_BIT = 0;
            return; 

        case 0x37:  // LUI: rd = immediate
            NEXT_STATE.REGS[rd] = imm;
            break;
            
        case 0x17:  // AUIPC: rd = CURRENT_STATE.PC + immediate
            NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + imm;
            break;
            
        case 0x13:  // I-type instructions
            if (funct3 == 0x0) {  // ADDI
                NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] + imm;
            } else if (funct3 == 0x1) {  // SLLI
                if (funct7 == 0x00)
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] << imm;
                else
                    printf("Execute: Unsupported funct7 (0x%X) for SLLI\n", funct7);
            } else {
                printf("Execute: Unsupported funct3 (0x%X) for I-type instruction\n", funct3);
            }
            break;
            
        case 0x33:  // R-type instructions
            if (funct3 == 0x0) {
                if (funct7 == 0x00) {  // ADD
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] + CURRENT_STATE.REGS[rs2];
                } else if (funct7 == 0x20) {  // SUB
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] - CURRENT_STATE.REGS[rs2];
                } else {
                    printf("Execute: Unsupported funct7 (0x%X) for R-type ADD/SUB\n", funct7);
                }
            } else if (funct3 == 0x1) {  // SLL
                NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] << (CURRENT_STATE.REGS[rs2] & 0x1F);
            } else if (funct3 == 0x2) {  // SLT
                NEXT_STATE.REGS[rd] = (((int)CURRENT_STATE.REGS[rs1]) < ((int)CURRENT_STATE.REGS[rs2])) ? 1 : 0;
            } else if (funct3 == 0x4) {  // XOR
                NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] ^ CURRENT_STATE.REGS[rs2];
            } else if (funct3 == 0x5) {
                if (funct7 == 0x00) {  // SRL
                    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] >> (CURRENT_STATE.REGS[rs2] & 0x1F);
                } else if (funct7 == 0x20) {  // SRA
                    NEXT_STATE.REGS[rd] = ((int)CURRENT_STATE.REGS[rs1]) >> (CURRENT_STATE.REGS[rs2] & 0x1F);
                } else {
                    printf("Execute: Unsupported funct7 (0x%X) for R-type shift\n", funct7);
                }
            } else if (funct3 == 0x6) {  // OR
                NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] | CURRENT_STATE.REGS[rs2];
            } else if (funct3 == 0x7) {  // AND
                NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] & CURRENT_STATE.REGS[rs2];
            } else {
                printf("Execute: Unsupported funct3 (0x%X) for R-type instruction\n", funct3);
            }
            break;

         case 0x23:  // S-type: store word
            if (funct3 == 0x2) {
                uint64_t addr = (uint64_t)CURRENT_STATE.REGS[rs1] + imm;
                mem_write_32(addr, CURRENT_STATE.REGS[rs2]);
            } else {
                printf("Execute: Unsupported funct3 (0x%X) for S-type instruction\n", funct3);
            }
            break;
            
        case 0x63:  // B-type: branch instructions
            {
                int take_branch = 0;
                if (funct3 == 0x0) {  // BEQ
                    take_branch = (CURRENT_STATE.REGS[rs1] == CURRENT_STATE.REGS[rs2]);
                } else if (funct3 == 0x1) {  // BNE
                    take_branch = (CURRENT_STATE.REGS[rs1] != CURRENT_STATE.REGS[rs2]);
                } else if (funct3 == 0x4) {  // BLT
                    take_branch = (((int)CURRENT_STATE.REGS[rs1]) < ((int)CURRENT_STATE.REGS[rs2]));
                } else if (funct3 == 0x5) {  // BGE
                    take_branch = (((int)CURRENT_STATE.REGS[rs1]) >= ((int)CURRENT_STATE.REGS[rs2]));
                } else {
                    printf("Execute: Unsupported branch funct3 (0x%X)\n", funct3);
                }
                if (take_branch) {
                    NEXT_STATE.PC = CURRENT_STATE.PC + imm;
                }
            }
            break;

        default:
            printf("Execute: Opcode 0x%02X not implemented.\n", opcode);
            break;
    }
}

// process_instruction: Fetch, decode, and execute one instruction
void process_instruction() {
    // Begin with a fresh copy of the current state
    fetch();
    decode();
    execute();
}
