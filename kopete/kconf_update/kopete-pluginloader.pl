#!/usr/bin/perl
while( my $line = <> )
{
  if( $line =~ /^Modules=/ )
  {
    $line =~ s/^Modules/Plugins/;
    $line =~ s/\.plugin/\.desktop/g;
    print $line;
  }
}

print "# DELETE Modules\n";

