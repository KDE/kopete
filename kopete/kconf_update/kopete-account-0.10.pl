#!/usr/bin/perl -w
# Olivier Goffart <ogoffart@tiscalinet.be>
# License: GPL

use strict;

# This script rename old plugin datas key.
# It remove the  PlguinData_PLUGINID_  prefix from keys.

# read the whole config file
my $currentGroup = "";
my %configFile;
while ( <> ) {
  chomp; # eat the trailing '\n'
  next if ( /^$/ ); # skip empty lines
  next if ( /^\#/ ); # skip comments
  if ( /^\[/ ) { # group begin
    $currentGroup = $_;
    next;
  } elsif ( $currentGroup =~ /^\[Account_/  and  /^PluginData\_.+_(.+)=(.+)$/ )
  {
		print "$currentGroup\n$1=$2\n";
      my ($key,$value) = split /=/;
		print "# DELETE $currentGroup$key\n";
   }
}
