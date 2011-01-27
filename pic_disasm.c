/*
 * vPICdisasm - PIC program disassembler.
 * Version 1.2 - July 2010.
 * Written by Vanya A. Sergeev - <vsergeev@gmail.com>
 *
 * Copyright (C) 2007 Vanya A. Sergeev
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
 * pic_disasm.c - PIC instruction disassembly into disassembledInstruction structure.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include "pic_disasm.h"
#include "errorcodes.h"

/* Array of PIC instruction sets as defined in pic_instructionset.c,
 * enumerated by PIC_Instruction_Set_Index enum in pic_disasm.h */
extern instructionSetInfo allInstructionSets[];

/* Disassembles/decodes operands back to their original form. */
static int disassembleOperands(disassembledInstruction *dInstruction);
/* Extracts certain bits of data from a mask, used to extract operands from their encoding in the opcode. */
static uint16_t extractDataFromMask(uint16_t data, uint16_t mask);
/* Look up an instruction by it's opcode in the instructionSet,
 * starting from index offset. Always returns a valid instruction
 * index because the last instruction in the instruction set database
 * is set to be a generic data word (data). */
static int lookupInstruction(uint16_t opcode, int offset, int instructionSetIndex);


/* Disassembles an assembled instruction, including its operands. */
int disassembleInstruction(disassembledInstruction *dInstruction, const assembledInstruction aInstruction, int instructionSetIndex) {
	int instructionIndex, i;
	
	if (dInstruction == NULL)
		return ERROR_INVALID_ARGUMENTS;

	if (instructionSetIndex != PIC_BASELINE &&
	    instructionSetIndex != PIC_MIDRANGE &&
	    instructionSetIndex != PIC_MIDRANGE_ENHANCED)
		return ERROR_INVALID_ARGUMENTS;
	
	/* Look up the instruction */
	instructionIndex = lookupInstruction(aInstruction.opcode, 0, instructionSetIndex);

	/* Copy over the address, and reference to the instruction, set
	 * the equivilant-encoded but different instruction to NULL for now. */
	dInstruction->address = aInstruction.address;
	dInstruction->instruction = &(allInstructionSets[instructionSetIndex].instructionSet[instructionIndex]);
	dInstruction->alternateInstruction = NULL;
	
	/* Copy out each operand, extracting the operand data from the original
	 * opcode using the operand mask. */
	for (i = 0; i < allInstructionSets[instructionSetIndex].instructionSet[instructionIndex].numOperands; i++) 
		dInstruction->operands[i] = extractDataFromMask(aInstruction.opcode, dInstruction->instruction->operandMasks[i]);
	
	/* Disassemble operands */
	if (disassembleOperands(dInstruction) < 0)
		return ERROR_INVALID_ARGUMENTS; /* Only possible error for disassembleOperands() */

	return 0;
}

/* Extracts certain bits of data from a mask, used to extract operands from their encoding in the opcode. */
static uint16_t extractDataFromMask(uint16_t data, uint16_t mask) {
	int i, j;
	uint16_t result = 0;
	
	/* i counts through every bit of the data,
	 * j counts through every bit of the data we're copying out. */
	for (i = 0, j = 0; i < 16; i++) {
		/* If the mask has a bit in this position */
		if (mask & (1<<i)) {
			/* If there is a data bit with this mask bit,
			 * then toggle that bit in the extracted data (result).
			 * Notice that it uses its own bit counter j. */
			if (((mask & (1<<i)) & data) != 0)
				result |= (1<<j);
			/* Increment the extracted data bit count. */
			j++;
		}
	}
	
	return result;
}

/* Look up an instruction by it's opcode in the instructionSet,
 * starting from index offset. Always returns a valid instruction
 * index because the last instruction in the instruction set database
 * is set to be a generic data word (data). */
static int lookupInstruction(uint16_t opcode, int offset, int instructionSetIndex) {
	uint16_t opcodeSearch;
	int instructionIndex, i;
	
	for (instructionIndex = offset; instructionIndex < allInstructionSets[instructionSetIndex].numInstructions; instructionIndex++) {
		opcodeSearch = opcode;
		/* We want to mask out all of the operands. We don't count up to
		 * instructionSet[instructionIndex].numOperands because in some instructions
		 * we have the "x" don't-care bits that we want to mask out. */
		for (i = 0; i < PIC_MAX_NUM_OPERANDS; i++) 
			opcodeSearch &= ~(allInstructionSets[instructionSetIndex].instructionSet[instructionIndex].operandMasks[i]);

		if (opcodeSearch == allInstructionSets[instructionSetIndex].instructionSet[instructionIndex].opcodeMask) 
			break;
	}
	/* It's impossible not to find an instruction, because the last instruction "data",
	 * specifies a word of data at the addresses, instead of an instruction. 
	 * Its operand 2 mask, 0x0000, will set opcode search to 0x0000, and this will always
	 * match with the opcodeMask of 0x0000. */
	return instructionIndex;
}

/* Disassembles/decodes operands back to their original form. */
static int disassembleOperands(disassembledInstruction *dInstruction) {
	int i;
	uint16_t msb;

	/* This should never happen */
	if (dInstruction == NULL)
		return ERROR_INVALID_ARGUMENTS;
	if (dInstruction->instruction == NULL)
		return ERROR_INVALID_ARGUMENTS;
	
	/* For each operand, decode its original value. */
	for (i = 0; i < dInstruction->instruction->numOperands; i++) {
		switch (dInstruction->instruction->operandTypes[i]) {
			case OPERAND_SIGNED_LITERAL:
			case OPERAND_RELATIVE_ADDRESS:
				/* We got lucky, because it turns out that in all of the masks
				 * for relative jumps / signed literals, the bits occupy the
				 * lowest positions continuously (no breaks in the bit string). */

				/* Calculate the most significant bit of this signed data */
				msb = (dInstruction->instruction->operandMasks[i] + 1) >> 1;
				/* Check if the most significant bit is set (the number is negative) */
				if ((dInstruction->operands[i] & msb) != 0) {
					/* If so, recover the data and set the operand negative. */
					dInstruction->operands[i] = (~dInstruction->operands[i]+1)&(dInstruction->instruction->operandMasks[i]);
					dInstruction->operands[i] = -dInstruction->operands[i];
				}
			default:
				break;
		}
	}
	return 0;
}

