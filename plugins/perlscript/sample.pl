sub OutgoingMessage
{
	#The first paramater is the first word of the message
	my $firstWord = shift;

	#The second paramater is the remainer of the message
	my $messageText = shift;

	#The third paramater is who the message is from
	my $from = shift;

	#The remaining paramaters are who the message is to (could be a list)
	my @to = @_;

	#Sample - reverse the text of a message prefixed with "/reverse"
	if( $firstWord eq "/reverse" )
	{
		$messageText = reverse( $messageText );
		$messageBg = "blue";
		$messageFg = "green";
	}
	
	#Return array format is ("Background Color", "Foreground Color", "Message Text")
	return ($messageBg, $messageFg, $messageText);
}

sub ContactContextMenu
{
	#The first paramater is the displayName of the MC
	my $contactName = shift;
	
	#Do some stuff
	`xmms-shell -e play`;
}
