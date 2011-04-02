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
 * format.h - Header file to formatting of disassembled instructions, with
 *  regards to the several formatting features this disasssembler supports.
 *
 */
 
#include "pic_disasm.h"
#include "errorcodes.h"

#ifndef FORMAT_DISASM_H
#define FORMAT_DISASM_H

/* PIC Operand prefixes
 * In order to keep the disassembler as straightforward as possible,
 * but still have some fundamental formatting options, I decided to hardcode
 * the operand prefixes. Feel free to change them here.  */
#define OPERAND_PREFIX_REGISTER			"0x"	/* i.e. clrf 0x25 */
#define OPERAND_SUFFIX_REGISTER			""	
#define OPERAND_PREFIX_BIT			""	/* i.e. bsf 0x32, 0 */
#define OPERAND_SUFFIX_BIT			""
#define OPERAND_PREFIX_LITERAL_HEX 		"0x"	/* movlw 0x6 */
#define OPERAND_SUFFIX_LITERAL_HEX		""
#define OPERAND_PREFIX_LITERAL_BIN 		"b'"	/* movlw b'00000110' */
#define OPERAND_SUFFIX_LITERAL_BIN		"'"
#define OPERAND_PREFIX_LITERAL_DEC 		""	/* movlw 6 */
#define OPERAND_SUFFIX_LITERAL_DEC		""
#define OPERAND_PREFIX_ABSOLUTE_ADDRESS		"0x"	/* call 0xB6 */
#define OPERAND_SUFFIX_ABSOLUTE_ADDRESS		""
#define OPERAND_PREFIX_WORD_DATA		"0x"	/* data 0xABCD */
#define OPERAND_SUFFIX_WORD_DATA		""
#define OPERAND_PREFIX_INDF_INDEX		"INDF"	/* INDF0, INDF1, ... */
#define OPERAND_SUFFIX_INDF_INDEX		""
#define OPERAND_REGISTER_DEST_W			"W"	/* movf 0x25, W */
#define OPERAND_REGISTER_DEST_F			"F"	/* movf 0x25, F */

/* Enumeration for different types of formatting options supported by this disassembler. */
/* Formatting Options Toggle Bits:
 * FORMAT_OPTION_ADDRESS_LABEL: creates address labels with the prefix set in the string addressLabelPrefix.
 * FORMAT_OPTION_ADDRESS: Prints the address of the instruction alongside the instruction
 * FORMAT_OPTION_DESTINATION_ADDRESS_COMMENT: Prints the address of relative jumps/branches as a comment
 * FORMAT_OPTION_LITERAL_HEX: Represent the literal operands in hexadecimal
 * FORMAT_OPTION_LITERAL_BIN: Represent the literal operands in binary
 * FORMAT_OPTION_LITERAL_DEC: Represent the literal operands in decimal
 * FORMAT_OPTION_LITERAL_ASCII_COMMENT: Show the ASCII value of the literal with a comment
 */
enum PIC_Formatting_Options {
	FORMAT_OPTION_ADDRESS_LABEL = 1,
	FORMAT_OPTION_ADDRESS = 2,
	FORMAT_OPTION_DESTINATION_ADDRESS_COMMENT = 4,
	FORMAT_OPTION_LITERAL_HEX = 8,
	FORMAT_OPTION_LITERAL_BIN = 16,
	FORMAT_OPTION_LITERAL_DEC = 32,
	FORMAT_OPTION_LITERAL_ASCII_COMMENT = 64,
};

/* Structure to hold various formatting options supported
 * by this disassembler. */
struct _formattingOptions {
	/* Options with PIC_Formatting_Options bits set. */
	int options;
	/* The prefix for address labels,
	 * if they are enabled in options. */
	char addressLabelPrefix[8];
	/* Space field width for address, i.e. "001C"
 	 * has an address field width of 4. */
	int addressFieldWidth;
};
typedef struct _formattingOptions formattingOptions;


/* Prints a disassembled instruction, formatted with options set in the formattingOptions structure. */
int printDisassembledInstruction(FILE *out, const disassembledInstruction *dInstruction, formattingOptions fOptions);

#endif
