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
 * ui.c - Main user interface to PIC disassembler. Takes care of program arguments
 *  setting disassembly formatting options, and program file type recognition. 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "file.h"
#include "errorcodes.h"

/* Flags for some long options that don't have a short option equivilant */
static int no_addresses = 0;				/* Flag for --no-addresses */
static int literal_base = 0;				/* Value of the literal base (hex, bin, dec) */
static int literal_ascii_comment = 0;			/* Flag for --literal-ascii */
static int no_destination_comments = 0;			/* Flag for --no-destination-comments */
static int original_opcode = 0;				/* Flag for --original */

static struct option long_options[] = {
	{"address-label", required_argument, NULL, 'l'},
	{"arch", required_argument, NULL, 'a'},
	{"out-file", required_argument, NULL, 'o'},
	{"file-type", required_argument, NULL, 't'},
	{"no-addresses", no_argument, &no_addresses, 1},
	{"literal-bin", no_argument, &literal_base, FORMAT_OPTION_LITERAL_BIN},
	{"literal-dec", no_argument, &literal_base, FORMAT_OPTION_LITERAL_DEC},
	{"literal-hex", no_argument, &literal_base, FORMAT_OPTION_LITERAL_HEX},
	{"literal-ascii", no_argument, &literal_ascii_comment, 1},
	{"original", no_argument, &original_opcode, 1},
	{"no-destination-comments", no_argument, &no_destination_comments, 1},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0}
};

static void printUsage(FILE *stream, const char *programName) {
	fprintf(stream, "Usage: %s <option(s)> <file>\n", programName);
	fprintf(stream, " Disassembles PIC program file <file>. Use - for standard input.\n");
	fprintf(stream, " Written by Vanya A. Sergeev - <vsergeev@gmail.com>.\n\n");
	fprintf(stream, " Additional Options:\n\
  -o, --out-file <output file>	Write to output file instead of standard output.\n\
  -a, --arch <architecture>	Specify the 8-bit PIC architecture to use\n\
				during disassembly.\n\
  -t, --file-type <type>	Specify the file type of the object file.\n\
  -l, --address-label <prefix> 	Create ghetto address labels with \n\
				the specified label prefix.\n\
  --original                    Print original opcode data alongside\n\
				disassembly.\n\
  --no-addresses		Do not display the address alongside\n\
				disassembly.\n\
  --no-destination-comments	Do not display the destination address\n\
				comments of relative branch instructions.\n\
  --literal-hex			Represent literals in hexadecimal (default)\n\
  --literal-bin			Represent literals in binary\n\
  --literal-dec			Represent literals in decimal\n\
  --literal-ascii		Show ASCII value of literal operands in a\n\
				comment\n\
  -h, --help			Display this usage/help.\n\
  -v, --version			Display the program's version.\n\n");
	fprintf(stream, "Supported 8-bit PIC Architectures:\n\
  Baseline 			baseline\n\
  Mid-Range			midrange (default)\n\
  Enhanced Mid-Range		enhanced\n\n");
	fprintf(stream, "Supported file types:\n\
  Intel HEX8 			ihex\n\
  Motorola S-Record 		srecord\n\n");
}

static void printVersion(FILE *stream) {
	fprintf(stream, "vPICdisasm version 1.3 - 04/03/2011.\n");
	fprintf(stream, "Written by Vanya Sergeev - <vsergeev@gmail.com>\n");
}
	
int main(int argc, const char *argv[]) {
	int optc;
	FILE *fileIn, *fileOut;
	char arch[9], fileType[8];
	int archSelect;
	int (*disassembleFile)(FILE *, FILE *, formattingOptions, int);
	formattingOptions fOptions;

	/* Recent flag options */
	fOptions.options = 0;
	/* Set default address field width for this version. */
	fOptions.addressFieldWidth = 3;
	/* Default output file to stdout */
	fileOut = stdout;

	arch[0] = '\0';
	fileType[0] = '\0';	
	while (1) {
		optc = getopt_long(argc, (char * const *)argv, "o:a:t:l:hv", long_options, NULL);
		if (optc == -1)
			break;
		switch (optc) {
			/* Long option */
			case 0:
				break;
			case 'l':
				fOptions.options |= FORMAT_OPTION_ADDRESS_LABEL;
				strncpy(fOptions.addressLabelPrefix, optarg, sizeof(fOptions.addressLabelPrefix));
				break;
			case 't':
				strncpy(fileType, optarg, sizeof(fileType));
				break;
			case 'a':
				strncpy(arch, optarg, sizeof(arch));
				break;
			case 'o':
				if (strcmp(optarg, "-") != 0)
					fileOut = fopen(optarg, "w");
				break;
			case 'h':
				printUsage(stderr, argv[0]);
				exit(EXIT_SUCCESS);	
			case 'v':
				printVersion(stderr);
				exit(EXIT_SUCCESS);
			default:
				printUsage(stdout, argv[0]);
				exit(EXIT_SUCCESS);
		}
	}

	if (!no_addresses)
		fOptions.options |= FORMAT_OPTION_ADDRESS;
	if (!no_destination_comments)
		fOptions.options |= FORMAT_OPTION_DESTINATION_ADDRESS_COMMENT;	

	if (literal_base == FORMAT_OPTION_LITERAL_DEC)
		fOptions.options |= FORMAT_OPTION_LITERAL_DEC;
	else if (literal_base == FORMAT_OPTION_LITERAL_BIN)
		fOptions.options |= FORMAT_OPTION_LITERAL_BIN;
	else
		fOptions.options |= FORMAT_OPTION_LITERAL_HEX;

	if (literal_ascii_comment)
		fOptions.options |= FORMAT_OPTION_LITERAL_ASCII_COMMENT;

	if (original_opcode)
		fOptions.options |= FORMAT_OPTION_ORIGINAL_OPCODE;
	
	if (fileOut == NULL) {
		perror("Error: Cannot open output file for writing");
		exit(EXIT_FAILURE);
	}

	/* If there are no more arguments left */
	if (optind == argc) {
		fprintf(stderr, "Error: No program file specified! Use - for standard input.\n\n");
		printUsage(stderr, argv[0]);
		if (fileOut != stdout)
			fclose(fileOut);
		exit(EXIT_FAILURE);
	}

	/* Support reading from stdin with filename "-" */
	if (strcmp(argv[optind], "-") == 0) {
		fileIn = stdin;
	} else {
		fileIn = fopen(argv[optind], "r");
		if (fileIn == NULL) {
			perror("Error: Cannot open program file for disassembly");
			if (fileOut != stdout)
				fclose(fileOut);
			exit(EXIT_FAILURE);
		}
	}

	/* If no file type was specified, try to auto-recognize the first character of the file */
	if (fileType[0] == '\0') {
		int c;
		c = fgetc(fileIn);
		/* Intel HEX record statements start with : */
		if ((char)c == ':')
			strcpy(fileType, "ihex");
		/* Motorola S-Record record statements start with S */
		else if ((char)c == 'S')
			strcpy(fileType, "srecord");
		else {
			fprintf(stderr, "Unable to auto-recognize file type by first character.\n");
			fprintf(stderr, "Please specify file type with -t,--file-type option.\n");
			if (fileOut != stdout)
				fclose(fileOut);
			if (fileIn != stdin)
				fclose(fileIn);
			exit(EXIT_FAILURE);
		}
		ungetc(c, fileIn);
	}

	/* If no architecture was specified, use midrange by default */
	if (arch[0] == '\0') {
		archSelect = PIC_MIDRANGE;
	} else {
		if (strcasecmp(arch, "baseline") == 0)
			archSelect = PIC_BASELINE;
		else if (strcasecmp(arch, "midrange") == 0)
			archSelect = PIC_MIDRANGE;
		else if (strcasecmp(arch, "enhanced") == 0)
			archSelect = PIC_MIDRANGE_ENHANCED;
		else {
			fprintf(stderr, "Unknown 8-bit PIC architecture %s.\n", arch);
			fprintf(stderr, "See program help/usage for supported PIC architectures.\n");
			if (fileOut != stdout)
				fclose(fileOut);
			if (fileIn != stdin)
				fclose(fileIn);
			exit(EXIT_FAILURE);
		}
	}
	
	if (strcasecmp(fileType, "ihex") == 0)
		disassembleFile = disassembleIHexFile;
	else if (strcasecmp(fileType, "srecord") == 0)
		disassembleFile = disassembleSRecordFile;
	else {
		if (fileType[0] != '\0')
			fprintf(stderr, "Unknown file type %s.\n", fileType);
		else
			fprintf(stderr, "Unspecified file type.\n");
		fprintf(stderr, "See program help/usage for supported file types.\n");
		if (fileOut != stdout)
			fclose(fileOut);
		if (fileIn != stdin)
			fclose(fileIn);
		exit(EXIT_FAILURE);
	}

	disassembleFile(fileOut, fileIn, fOptions, archSelect);

	if (fileOut != stdout)	
		fclose(fileOut);
	if (fileOut != stdin)
		fclose(fileIn);

	exit(EXIT_SUCCESS);
}

