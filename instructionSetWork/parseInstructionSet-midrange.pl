#!/usr/bin/perl

open(HANDLE, "PICInstructionSet-midrange");
@filedata = <HANDLE>;
foreach $line (@filedata) {
	chomp($line);
	@instruction  = split(/ /, $line);

	$mnemonic = $instruction[0];
	$mnemonic = lc($mnemonic);
	for ($i = 0; $i < scalar @instruction; $i++) {
		if (length($instruction[$i]) eq 14) {
			$opcode = $instruction[$i];
			break;
		}
	}
	$opcode = "00" . $opcode;

	$opcodemask = $opcode;
	$opcodemask =~ s/\p{L}/0/g;
	
	$operand1var = "";
	$operand2var = "";
	$operand1mask = "";
	$operand2mask = "";
	$numOperands = 0;
	for ($i = 0; $i < length($opcode); $i++) { 
		if (substr($opcode, $i, 1) =~ /\p{L}/) {
			if (length($operand1var) == 0) {
				if (substr($opcode, $i, 1) eq "f") {
					$operand1var = "f";
				} elsif (substr($opcode, $i, 1) eq "k") {
					$operand1var = "k";
				}
			}
			if (length($operand2var) == 0) {
				if (substr($opcode, $i, 1) eq "d") {
					$operand2var = "d";
				} elsif (substr($opcode, $i, 1) eq "b") {
					$operand2var = "b";
				} elsif (substr($opcode, $i, 1) eq "x") {
					$operand2var = "x";
				}	
			}
			if (substr($opcode, $i, 1) eq $operand1var) {
				$operand1mask .= "1";
				$operand2mask .= "0";
			} elsif(substr($opcode, $i, 1) eq $operand2var) {
				$operand2mask .= "1";
				$operand1mask .= "0";
			} else {
				$operand1mask .= "0";
				$operand2mask .= "0";
			}
		} else {
			$operand1mask .= "0";
			$operand2mask .= "0";	
		} 
	}
	if (length($operand1var) == 0 && $operand2var eq "x") {
		$operand1var = $operand2var;
		$temp = $operand1mask;
		$operand1mask = $operand2mask;
		$operand2mask = $temp;
		$operand2var = "";
	}
	if ($operand1var eq "f") {
		$operand1type = "OPERAND_REGISTER";
	} elsif ($operand1var eq "k") {
		if ($mnemonic eq "call" || $mnemonic eq "goto") {
			$operand1type = "OPERAND_ABSOLUTE_ADDRESS";
		} else {	
			$operand1type = "OPERAND_LITERAL";
		}
	} elsif ($operand1var eq "x") {
		$operand1type = "OPERAND_NONE";
	} else {
		$operand1type = "OPERAND_NONE";
	}

	if ($operand2var eq "d") {
		$operand2type = "OPERAND_REGISTER_DEST";
	} elsif ($operand2var eq "b") {
		$operand2type = "OPERAND_BIT";
	} elsif ($operand2var eq "x") {
		$operand2type = "OPERAND_NONE";
	} else {
		$operand2type = "OPERAND_NONE";
	}

	if ($operand1var eq "") {
		$numOperands = 0;
	} elsif ($operand2var eq "") {
		$numOperands = 1;
	} else {
		$numOperands = 2;
	}

	if ($operand1var eq "x" || $operand2var eq "x") {
		$numOperands--;
	}

	print "\t";
	print "{\"" . $mnemonic . "\", ";
	print "0x" . bin2hex($opcodemask) . ", " . $numOperands . ", " . "{0x" . bin2hex($operand1mask) . ", 0x" . bin2hex($operand2mask) . ", 0x0000}, ";
	print "{" . $operand1type . ", " . $operand2type . ", OPERAND_NONE}},\n";

	
}

sub bin2hex {
	$bin = shift;
	return scalar reverse unpack"h*",pack"b*",scalar reverse $bin;
}

close(HANDLE);
