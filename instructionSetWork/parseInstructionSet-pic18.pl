#!/usr/bin/perl

use Switch;

open(HANDLE, "PICInstructionSet-pic18");
@filedata = <HANDLE>;
foreach $line (@filedata) {
	chomp($line);
	@instruction  = split(/ /, $line);

	$mnemonic = $instruction[0];
	$mnemonic = lc($mnemonic);
	$operandOrder = $instruction[1];
	$opcode = $instruction[2];

	@operands = split(/,/, $operandOrder);

	$numOperands = scalar(@operands);
	if ($numOperands == 1) {
		if ($operands[i] eq "-") {
			$numOperands = 0;
		}
	}

	while (scalar(@operands) < 3) {
		push(@operands, "");
	}
	
	@operandMasks = ("", "", "");

	$opcodeMask = $opcode;
	$opcodeMask =~ s/\p{L}/0/g;

	for ($i = 0; $i < length($opcode); $i++) {
		if (substr($opcode, $i, 1) =~ /\p{L}/) {
			for ($j = 0; $j < scalar(@operands); $j++) {
				if ($operands[$j] eq substr($opcode, $i, 1)) {
					$operandMasks[$j] .= "1";
				} elsif (substr($opcode, $i, 1) eq "x" && $operands[$j] eq "") {
					$operandMasks[$j] .= "1";
				} else {
					$operandMasks[$j] .= "0";
				}
			}
		} else {
			for ($j = 0; $j < scalar(@operands); $j++) {
				$operandMasks[$j] .= "0";
			}
		}
	}

#	print "Mnemonic: _" . $mnemonic . "_\n";
#	print "Operands: _" . $operandOrder . "_\n";
#	print "Opcode: _" . $opcode . "_\n";
#	print "Opcode mask: _" . $opcodeMask . "_\n";
#	for ($i = 0; $i < scalar(@operands); $i++) {
#		print "Operand mask $i: " . $operandMasks[$i] . "\n";
#	}
#	print "\n";



	print "\t";
	print "{\"" . $mnemonic . "\", " . "0x" . bin2hex($opcodeMask) . ", " . $numOperands . ", ";
	print "{0x" . bin2hex($operandMasks[0]) . ", 0x" . bin2hex($operandMasks[1]) . ", 0x" . bin2hex($operandMasks[2]) . "}, ";
	print "{";
	for ($i = 0; $i < scalar(@operands); $i++) {
		switch ($operands[$i]) {
			case "f" { print "OPERAND_REGISTER"; }
			case "d" { print "OPERAND_REGISTER_DEST"; }
			case "a" { print "OPERAND_REGISTER_DEST"; }
			case "s" { print "OPERAND_REGISTER_DEST"; }
			case "b" { print "OPERAND_BIT"; }
			case "k" { print "OPERAND_LITERAL"; }
			case "n" { print "OPERAND_RELATIVE_ADDRESS"; }
			case "l" { print "OPERAND_ABSOLUTE_ADDRESS"; }
			case "j" { print "OPERAND_FSR_INDEX"; }
			case "z" { print "OPERAND_OFFSET_ADDRESS"; }
			case "-" { print "OPERAND_NONE"; }
			case "" { print "OPERAND_NONE"; }
			else { die "Unknown operand!: _" . $op . "_\n"; }	
		}
		if ($i < 2) {
			print ", ";
		}
	}

	print "}},\n";
	
}

sub bin2hex {
	$bin = shift;
	return scalar reverse unpack"h*",pack"b*",scalar reverse $bin;
}

close(HANDLE);
