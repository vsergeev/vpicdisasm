/*
 * vPICdisasm - PIC program disassembler.
 * Written by Vanya A. Sergeev - <vsergeev@gmail.com>
 *
 * Copyright (C) 2007-2011 Vanya A. Sergeev
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA. 
 *
 * pic_disasm.h - Header file for PIC instruction disassembly into 
 *  disassembledInstruction structure.
 *
 */

#ifndef PIC_DISASM_H
#define PIC_DISASM_H

#include <stdint.h>

/* Maximum number of operands */
#define PIC_MAX_NUM_OPERANDS				3

/* Total number of assembly instructions */
#define PIC_TOTAL_BASELINE_INSTRUCTIONS			34
#define PIC_TOTAL_MIDRANGE_INSTRUCTIONS			38
#define PIC_TOTAL_MIDRANGE_ENHANCED_INSTRUCTIONS	56
#define PIC_TOTAL_PIC18_INSTRUCTIONS			74

/* Order of instruction sets held in allInstructionSets struct array */
enum PIC_Instruction_Set_Index {
	PIC_BASELINE,
	PIC_MIDRANGE,
	PIC_MIDRANGE_ENHANCED,
	PIC_PIC18,
};

/* Enumeration for all types of PIC Operands */
enum PIC_Operand_Types {
	OPERAND_NONE, 
	OPERAND_REGISTER,
	OPERAND_REGISTER_DEST,
	OPERAND_BIT,
	OPERAND_LITERAL,
	OPERAND_ABSOLUTE_ADDRESS,
	OPERAND_WORD_DATA,
	/* Enhanced operands */
	OPERAND_RELATIVE_ADDRESS,
	OPERAND_SIGNED_LITERAL,
	OPERAND_FSR_INDEX,
	OPERAND_INCREMENT_MODE,
	OPERAND_INDF_INDEX
};

/* Structure for each instruction in the instruction set */
struct _instructionInfo {
	char mnemonic[7];
	/* Bitwise AND mask for just the instruction bits */
	uint16_t opcodeMask;
	int numOperands;
	/* Bitwise AND mask for each operand in the opcode */
	uint16_t operandMasks[PIC_MAX_NUM_OPERANDS];
	int operandTypes[PIC_MAX_NUM_OPERANDS];
};
typedef struct _instructionInfo instructionInfo;

/* Structure to hold a pointer to and size of available instruction sets. */
typedef struct _instructionSetInfo {
	instructionInfo *instructionSet;
	int numInstructions;
} instructionSetInfo;

/* The raw assembed instruction as extracted from the program file. */
struct _assembledInstruction {
	uint32_t address;
	uint16_t opcode;
};
typedef struct _assembledInstruction assembledInstruction;

/* The disassembled/decoded instruction. */
struct _disassembledInstruction {
	uint32_t address;
	/* A convenient pointer to the instructionSet, so we can refer 
	 * the general details of the instruction stored in there. */
	instructionInfo *instruction;
	/* Notice that operands can be signed!
	 * This is in order to support the decoding of negative
	 * relative branch/jump/call distances. */
	int32_t operands[PIC_MAX_NUM_OPERANDS];
	/* A pointer to an alternate disassembledInstruction,
	 * so we can find all instructions with the same encoding. */
	struct _disassembledInstruction *alternateInstruction;
};
typedef struct _disassembledInstruction disassembledInstruction;

/* Disassembles an assembled instruction, including its operands. */
int disassembleInstruction(disassembledInstruction *dInstruction, const assembledInstruction *aInstruction, int instructionSetIndex);

#endif

