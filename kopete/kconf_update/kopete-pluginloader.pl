#!/usr/bin/perl
my $logging = "false";
my $moduleLine;

while( my $line = <> )
{
  if( $line =~ /LogAll/ )
  {
	$logging = "true";
  }
  if( $line =~ /^Modules=/ )
  {
	$moduleLine = $line;
  }
}
	
$moduleLine =~ s/^Modules/Plugins/;
$moduleLine =~ s/\.plugin/\.desktop/g;
$moduleLine =~ s/oscar/aim/;
if ( $logging == "true" )
{
		chomp $moduleLine;
		$moduleLine = $moduleLine . ",history.desktop\n";
}
print $moduleLine;

print "# DELETE Modules\n";

