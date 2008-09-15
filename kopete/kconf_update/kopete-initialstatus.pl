#!/usr/bin/perl

while( my $line = <> )
{	
	$line =~ s/^autoConnect=true/initialStatus=Online/;
	$line =~ s/^autoConnect=false/initialStatus=Offline/;
	print $line;
}

print "# DELETE [Behavior]autoConnect\n";
