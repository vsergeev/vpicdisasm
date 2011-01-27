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
 * format.c - Formatting of disassembled instructions, with regard to the
 *  several formatting features this disassembler supports.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "format.h"

/* Formats a disassembled operand with its prefix (such as 'R' to indicate a register) into the
 * pointer to a C-string strOperand, which must be free'd after it has been used.
 * I decided to format the disassembled operands individually into strings for maximum flexibility,
 * and so that the printing of the formatted operand is not hard coded into the format operand code.
 * If an addressLabelPrefix is specified in formattingOptions (option is set and string is not NULL), 
 * it will print the relative jump/call with this prefix and the destination address as the label. */
static int formatDisassembledOperand(char **strOperand, int operandNum, const disassembledInstruction dInstruction, formattingOptions fOptions);


/* Prints a disassembled instruction, formatted with options set in the formattingOptions structure. */
int printDisassembledInstruction(FILE *out, const disassembledInstruction dInstruction, formattingOptions fOptions) {
	int retVal, i;
	char *strOperand;
	
	/* If address labels are enabled, then we use an address label prefix as set in the
	 * string addressLabelPrefix, because labels need to start with non-numerical character
	 * for best compatibility with PIC assemblers. */	
	if (fOptions.options & FORMAT_OPTION_ADDRESS_LABEL) 
		retVal = fprintf(out, "%s%0*X\t%s ", fOptions.addressLabelPrefix, fOptions.addressFieldWidth, dInstruction.address, dInstruction.instruction->mnemonic);
	/* Otherwise print the address that the instruction is located at, without address labels. */
	else if (fOptions.options & FORMAT_OPTION_ADDRESS) 
		retVal = fprintf(out, "%4X:\t%s ", dInstruction.address, dInstruction.instruction->mnemonic);
	else 
		retVal = fprintf(out, "\t%s ", dInstruction.instruction->mnemonic);

	if (retVal < 0)
		return ERROR_FILE_WRITING_ERROR;

	for (i = 0; i < dInstruction.instruction->numOperands; i++) {
		/* Format the disassembled operand into the string strOperand, and print it */
		retVal = formatDisassembledOperand(&strOperand, i, dInstruction, fOptions);
		if (retVal < 0)
			return retVal;
		/* Print the operand and free if it's not NULL */
		if (strOperand != NULL) {
			/* If we're not on the first operand, but not on the last one either, print a comma separating
			 * the operands. */
			if (i > 0 && i != dInstruction.instruction->numOperands) {
				if (fprintf(out, ", ") < 0)
					return ERROR_FILE_WRITING_ERROR;
			}

			fprintf(out, "%s", strOperand);
			free(strOperand);
		}
	}

	if (fOptions.options & FORMAT_OPTION_LITERAL_ASCII_COMMENT) {
		for (i = 0; i < dInstruction.instruction->numOperands; i++) {
			if (dInstruction.instruction->operandTypes[i] == OPERAND_LITERAL) {
				if (fprintf(out, "%1s\t; '%c'", "", dInstruction.operands[i]) < 0)
					return ERROR_FILE_WRITING_ERROR;
				break;
			}
		}
	}

	if (fOptions.options & FORMAT_OPTION_DESTINATION_ADDRESS_COMMENT) {
		for (i = 0; i < dInstruction.instruction->numOperands; i++) {
			/* This is only done for operands with relative
			 * addresses. */
			if (dInstruction.instruction->operandTypes[i] == OPERAND_RELATIVE_ADDRESS) {
				if (fprintf(out, "%1s\t; %s%X", "  ", OPERAND_PREFIX_ABSOLUTE_ADDRESS, dInstruction.address+dInstruction.operands[i]+1) <0)
					return ERROR_FILE_WRITING_ERROR;
				break;
			}
		}
	}

	fprintf(out, "\n");
	return 0;
}

/* Formats a disassembled operand with its prefix (such as 'R' to indicate a register) into the
 * pointer to a C-string strOperand, which must be free'd after it has been used.
 * I decided to format the disassembled operands individually into strings for maximum flexibility,
 * and so that the printing of the formatted operand is not hard coded into the format operand code.
 * If an addressLabelPrefix is specified in formattingOptions (option is set and string is not NULL), 
 * it will print the relative branch/jump/call with this prefix and the destination address as the label. */
int formatDisassembledOperand(char **strOperand, int operandNum, const disassembledInstruction dInstruction, formattingOptions fOptions) {
	char binary[9];
	int retVal;

	switch (dInstruction.instruction->operandTypes[operandNum]) {
		case OPERAND_NONE:
			*strOperand = NULL;
			retVal = 0;
			break;
		case OPERAND_REGISTER:
			retVal = asprintf(strOperand, "%s%02X%s", OPERAND_PREFIX_REGISTER, dInstruction.operands[operandNum], OPERAND_SUFFIX_REGISTER);
			break;
		case OPERAND_REGISTER_DEST:
			if (dInstruction.operands[operandNum] == 0)
				retVal = asprintf(strOperand, "%s", OPERAND_REGISTER_DEST_W);
			else
				retVal = asprintf(strOperand, "%s", OPERAND_REGISTER_DEST_F);
			break;
		case OPERAND_BIT:
			retVal = asprintf(strOperand, "%s%d%s", OPERAND_PREFIX_BIT, dInstruction.operands[operandNum], OPERAND_SUFFIX_BIT);
			break;
		case OPERAND_ABSOLUTE_ADDRESS:
			/* If we have an address label, print it, otherwise just print the
			 * absolute address. */
			if ((fOptions.options & FORMAT_OPTION_ADDRESS_LABEL) && fOptions.addressLabelPrefix != NULL) { 
				retVal = asprintf(strOperand, "%s%0*X%s", fOptions.addressLabelPrefix, fOptions.addressFieldWidth, dInstruction.operands[operandNum], OPERAND_SUFFIX_ABSOLUTE_ADDRESS);
			} else {
				retVal = asprintf(strOperand, "%s%0*X%s", OPERAND_PREFIX_ABSOLUTE_ADDRESS, fOptions.addressFieldWidth, dInstruction.operands[operandNum], OPERAND_SUFFIX_ABSOLUTE_ADDRESS);
			}
			break;
		case OPERAND_LITERAL:
			/* Support printing in binary, decimal, or hexadecimal for literal operands */
			if (fOptions.options & FORMAT_OPTION_LITERAL_BIN) {
				int i;
				for (i = 7; i >= 0; i--) {
					if (dInstruction.operands[operandNum] & (1<<i))
						binary[7-i] = '1';
					else
						binary[7-i] = '0';
				}
				binary[8] = '\0'; 
				retVal = asprintf(strOperand, "%s%s%s", OPERAND_PREFIX_LITERAL_BIN, binary, OPERAND_SUFFIX_LITERAL_BIN);
			} else if (fOptions.options & FORMAT_OPTION_LITERAL_DEC) {
				retVal = asprintf(strOperand, "%s%d%s", OPERAND_PREFIX_LITERAL_DEC, dInstruction.operands[operandNum], OPERAND_SUFFIX_LITERAL_DEC);
			} else {
				retVal = asprintf(strOperand, "%s%X%s", OPERAND_PREFIX_LITERAL_HEX, dInstruction.operands[operandNum], OPERAND_SUFFIX_LITERAL_HEX);
			}
			break;
		/********************************/
		/* Mid-range Enhanced Operands */
		/******************************/
		case OPERAND_RELATIVE_ADDRESS:
			/* If we have an address label, print it, otherwise just print the
			 * relative distance to the destination address. */
			if ((fOptions.options & FORMAT_OPTION_ADDRESS_LABEL) && fOptions.addressLabelPrefix != NULL) { 
				retVal = asprintf(strOperand, "%s%0*X", fOptions.addressLabelPrefix, fOptions.addressFieldWidth, dInstruction.address+dInstruction.operands[operandNum]+1);
			} else {
				if (dInstruction.operands[operandNum] > 0)
					retVal = asprintf(strOperand, ".+%d", dInstruction.operands[operandNum]);
				else
					retVal = asprintf(strOperand, ".%d", dInstruction.operands[operandNum]);
			}
			break;
		case OPERAND_FSR_INDEX:
			retVal = asprintf(strOperand, "%d", dInstruction.operands[operandNum]);
			break;
		case OPERAND_INDF_INDEX:
			if (operandNum == 0 && dInstruction.instruction->numOperands == 2) {
				/* This may be a pre/post increment or a indirect indexed instruction according to
				 * the type of the second operand.  */
				if (dInstruction.instruction->operandTypes[1] == OPERAND_INCREMENT_MODE) {
					switch (dInstruction.operands[1]) {
						case 0:
							retVal = asprintf(strOperand, "++%s%d%s", OPERAND_PREFIX_INDF_INDEX, dInstruction.operands[operandNum], OPERAND_SUFFIX_INDF_INDEX);
							break;
						case 1:
							retVal = asprintf(strOperand, "--%s%d%s", OPERAND_PREFIX_INDF_INDEX, dInstruction.operands[operandNum], OPERAND_SUFFIX_INDF_INDEX);
							break;
						case 2:
							retVal = asprintf(strOperand, "%s%d%s++", OPERAND_PREFIX_INDF_INDEX, dInstruction.operands[operandNum], OPERAND_SUFFIX_INDF_INDEX);
							break;
						case 3:
							retVal = asprintf(strOperand, "%s%d%s--", OPERAND_PREFIX_INDF_INDEX, dInstruction.operands[operandNum], OPERAND_SUFFIX_INDF_INDEX);
							break;
						default:
							return ERROR_UNKNOWN_OPERAND;
					}
				} else if (dInstruction.instruction->operandTypes[1] == OPERAND_SIGNED_LITERAL) {
					retVal = asprintf(strOperand, "%d[%s%d%s]", dInstruction.operands[1], OPERAND_PREFIX_INDF_INDEX, dInstruction.operands[operandNum], OPERAND_SUFFIX_INDF_INDEX);
				} else {
					return ERROR_UNKNOWN_OPERAND;
				}
			} else {
				return ERROR_UNKNOWN_OPERAND;
			}
			break;
		case OPERAND_WORD_DATA:
			retVal = asprintf(strOperand, "%s%0*X%s", OPERAND_PREFIX_WORD_DATA, fOptions.addressFieldWidth, dInstruction.operands[operandNum], OPERAND_SUFFIX_INDF_INDEX);
			break;
		
		case OPERAND_SIGNED_LITERAL:
			if (operandNum == 1) {
				/* If this signed literal belongs to the addfsr instruction */
				if (dInstruction.instruction->operandTypes[0] == OPERAND_FSR_INDEX) {
					retVal = asprintf(strOperand, "%d", dInstruction.operands[operandNum]);	

				/* Otherwise, it belongs to the moviw/movwi instructions and was handled in OPERAND_INDF_INDEX */
				} else if (dInstruction.instruction->operandTypes[0] == OPERAND_INDF_INDEX) {
					*strOperand = NULL;
					retVal = 0;
				} else {
					return ERROR_UNKNOWN_OPERAND;
				}	
			} else {
				return ERROR_UNKNOWN_OPERAND;	
			}
			break;	
		case OPERAND_INCREMENT_MODE:
			*strOperand = NULL;
			retVal = 0;
			break;

		/* This is impossible by normal operation. */
		default:
			return ERROR_UNKNOWN_OPERAND; 
	}

	if (retVal < 0)
		return ERROR_MEMORY_ALLOCATION_ERROR;
	return 0;
}
