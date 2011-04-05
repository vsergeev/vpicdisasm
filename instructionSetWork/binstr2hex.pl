#!/usr/bin/perl

sub bin2hex {
	$bin = shift;
	return scalar reverse unpack"h*",pack"b*",scalar reverse $bin;
}

while (1) {
	$binstr = <>;
	chomp($binstr);
	print "0x" . bin2hex($binstr);
	print "\n\n";
}
