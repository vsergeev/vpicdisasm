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
 * file.c - Routines to take care of Intel HEX, and Motorola S-Record file
 *  disassembly. Interface to pic_disasm.c to and format.c to disassemble
 *  print a disassembled instruction.
 *
 */

#include <stdio.h>
#include "libGIS-1.0.5/ihex.h"
#include "libGIS-1.0.5/srecord.h"
#include "pic_disasm.h"
#include "format.h"
#include "file.h"

/* Reads a record from an Intel Hex formatted file, formats the assembled
 * instruction data into an assembledInsruction structure, then passes the
 * assembled instruction data to disassembleAndPrint() for disassembly and printing.
 * Loops until all records have been read and processed. */
int disassembleIHexFile(FILE *fileOut, FILE *fileIn, formattingOptions fOptions, int archSelect) {
	assembledInstruction aInstruction;
	IHexRecord irec;
	int i;
	int retVal = 0;
	uint16_t dataFromPreviousOddRecord = 0;
	int dataFromPreviousOddRecordAvailable = 0;


	while (retVal == 0) {
		retVal = Read_IHexRecord(&irec, fileIn);
		/* Skip any newlines (there might be onat the end of the file) */
		if (retVal == IHEX_ERROR_NEWLINE)
			continue;
		else if (retVal == IHEX_ERROR_EOF)
			break;

		switch (retVal) {
			case IHEX_OK:
				break;
			case IHEX_ERROR_FILE:
				perror("Error reading Intel HEX formatted file");
				return ERROR_FILE_READING_ERROR;
			case IHEX_ERROR_INVALID_RECORD:
				fprintf(stderr, "Invalid Intel HEX formatted file!\n");
				return ERROR_FILE_READING_ERROR;
			case IHEX_ERROR_INVALID_ARGUMENTS:
			default:
				fprintf(stderr, "Encountered an irrecoverable error during reading of Intel HEX formatted file!\n");
				return ERROR_IRRECOVERABLE;
		}

		/* Skip the record if it's not a data record */
		if (irec.type != IHEX_TYPE_00)
			continue;

		aInstruction.address = irec.address/2;
		for (i = 0; i < irec.dataLen; i += 2) { 
			/* Make sure there is a data byte after this,
			 * (we need both because each opcode is 16-bits) */
			if (i+1 >= irec.dataLen) {
				/* Check if we have an even byte length record */
				if (irec.dataLen % 2 == 0) {
					fprintf(stderr, "Intel HEX formatted file does not hold valid PIC binary!\n");
					return ERROR_FILE_READING_ERROR;
				} else {
					/* Otherwise, we have an odd byte length
					 * record, so we need to save this data
					 * byte and fetch the next one. */
					dataFromPreviousOddRecord = irec.data[i];
					dataFromPreviousOddRecordAvailable = 1;
					continue;
				}
			}
			
			if (dataFromPreviousOddRecordAvailable != 1) {
				/* Assembled PIC program is stored in little-endian. */
				aInstruction.opcode = ((uint16_t)irec.data[i+1] << 8);
				aInstruction.opcode += ((uint16_t)irec.data[i]);
			} else {
				/* Use the left over data from the previous odd
				 * byte length record. */
				aInstruction.opcode = ((uint16_t)irec.data[i] << 8);
				aInstruction.opcode += dataFromPreviousOddRecord;
				dataFromPreviousOddRecordAvailable = 0;

				/* Decrement the counter by one so the outside
				 * for loop counts correctly (it adds 2 to the
				 * counter each iteration) */
				i--;
			}
	
			retVal = disassembleAndPrint(fileOut, &aInstruction, fOptions, archSelect);
			if (retVal < 0)
				return retVal;
			/* Increment the address for the correct address
			 * location of the next instruction. */
			aInstruction.address++;
		}
	}

	finishDisassembly(fileOut, fOptions);
	
	return 0;		
}

/* Reads a record from an Motorola S-Record formatted file, formats the assembled
 * instruction data into an assembledInsruction structure, then passes the
 * assembled instruction data to disassembleAndPrint() for disassembly and printing.
 * Loops until all records have been read and processed. */
int disassembleSRecordFile(FILE *fileOut, FILE *fileIn, formattingOptions fOptions, int archSelect) {
	assembledInstruction aInstruction;
	SRecord srec;
	int i;
	int retVal = 0;
	uint16_t dataFromPreviousOddRecord = 0;
	int dataFromPreviousOddRecordAvailable = 0;

	while (retVal == 0) {
		retVal = Read_SRecord(&srec, fileIn);
		/* Skip any new lines (there might be on at the end of the file) */
		if (retVal == SRECORD_ERROR_NEWLINE)
			continue;
		else if (retVal == SRECORD_ERROR_EOF)
			break;
		switch (retVal) {
			case SRECORD_OK:
				break;
			case SRECORD_ERROR_FILE:
				perror("Error reading Motorola S-Record formatted file");
				return ERROR_FILE_READING_ERROR;
			case SRECORD_ERROR_INVALID_RECORD:
				fprintf(stderr, "Invalid Motorola S-Record formatted file!\n");
				return ERROR_FILE_READING_ERROR;
			case SRECORD_ERROR_INVALID_ARGUMENTS:
			default:
				fprintf(stderr, "Encountered an irrecoverable error during reading of Motorola S-Record formatted file!\n");
				return ERROR_IRRECOVERABLE;
		}

		/* Skip the record if it's not a data record */
		if (srec.type != SRECORD_TYPE_S1 && srec.type != SRECORD_TYPE_S2 && srec.type != SRECORD_TYPE_S3)
			continue;

		aInstruction.address = srec.address/2;
		for (i = 0; i < srec.dataLen; i += 2) { 
			/* Make sure there is a data byte after this,
			 * (we need both because each opcode is 16-bits) */
			if (i+1 >= srec.dataLen) {
				/* Check if we have an even byte length record */
				if (srec.dataLen % 2 == 0) {
					fprintf(stderr, "Motorola S-Record formatted file does not hold a valid PIC binary!\n");
					return ERROR_FILE_READING_ERROR;
				} else {
					/* Otherwise, we have an odd byte length record, so we need to save this data byte and fetch the next one. */
					dataFromPreviousOddRecord = srec.data[i];
					dataFromPreviousOddRecordAvailable = 1;
					continue;
				}
			}
		
			if (dataFromPreviousOddRecordAvailable != 1) {	
				/* Assembled PIC program is stored in little-endian. */
				aInstruction.opcode = ((uint16_t)srec.data[i+1] << 8);
				aInstruction.opcode += ((uint16_t)srec.data[i]);
			} else {
				/* Use the left over data from the previous odd
				 * byte length record. */
				aInstruction.opcode = ((uint16_t)srec.data[i] << 8);
				aInstruction.opcode += dataFromPreviousOddRecord;
				dataFromPreviousOddRecordAvailable = 0;
				
				/* Decrement the counter by one so the outside
				 * for loop counts correclty (it adds 2 to the
				 * counter each iteration). */
				i--;
			}
	
			retVal = disassembleAndPrint(fileOut, &aInstruction, fOptions, archSelect);
			if (retVal < 0)
				return retVal;
			/* Increment the address for the correct address
			 * location of the next instruction. */
			aInstruction.address++;
		}
	}

	finishDisassembly(fileOut, fOptions);
	
	return 0;
}

static int currentAddress = -5;

/* Disassemble an assembled instruction, and print its disassembly
 * to fileOut. Alert user of errors. */
int disassembleAndPrint(FILE *fileOut, const assembledInstruction *aInstruction, formattingOptions fOptions, int archSelect) {
	disassembledInstruction dInstruction;
	int retVal;

	/* If we are printing address labels (assemble-able code) */
	if ((fOptions.options & FORMAT_OPTION_ADDRESS_LABEL) != 0) {
		/* Increment the current address to follow along with the disassembly */
		currentAddress++;
		/* If the current address is less than zero (meaning it hasn't been set yet, since
		 * currentAddress is initialized to -5), or the current address isn't consistent with
		 * the address of the next disassembled instruction, we need to mark a new program origin
		 * with the org directive. */
		if (currentAddress < 0 || currentAddress != aInstruction->address) {
			currentAddress = aInstruction->address;
			fprintf(fileOut, "\norg 0x%0*X\n", fOptions.addressFieldWidth, currentAddress);
		}
	}

	/* First disassemble the instruction, and check for errors. */
	retVal = disassembleInstruction(&dInstruction, aInstruction, archSelect);
	switch (retVal) {
		case 0:
			break;
		default:
			fprintf(stderr, "Encountered an irrecoverable error during disassembly!\n");
			return ERROR_IRRECOVERABLE;
	}
			
	/* Next print the disassembled instruction, check for errors. */
	retVal = printDisassembledInstruction(fileOut, aInstruction, &dInstruction, fOptions);
	switch (retVal) {
		case 0:
			break;
		case ERROR_FILE_WRITING_ERROR:
			fprintf(stderr, "Error writing formatted disassembly to file!\n");
			return ERROR_FILE_WRITING_ERROR;
		case ERROR_MEMORY_ALLOCATION_ERROR:
			fprintf(stderr, "Error allocating sufficient memory for formatted disassembly!\n");
			return ERROR_MEMORY_ALLOCATION_ERROR;
		default:
			fprintf(stderr, "Encountered an irrecoverable error during disassembly!\n");
			return ERROR_IRRECOVERABLE;
	}
	
	return 0;		
}

/* Finish off the disassemby - print "end" if we have address labels enabled */
int finishDisassembly(FILE *fileOut, formattingOptions fOptions) {
	if ((fOptions.options & FORMAT_OPTION_ADDRESS_LABEL) != 0) {
		if (fprintf(fileOut, "end\n") < 0) {
			fprintf(stderr, "Error writing formatted disassembly to file!\n");
			return ERROR_FILE_WRITING_ERROR;
		}
	}

	return 0;
}
