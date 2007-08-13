#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "messagesocket.h"

MessageSocket::MessageSocket()
{
	type = None;
	bound = false;
}

MessageSocket::~MessageSocket()
{
}

bool MessageSocket::setHostname( const char *hostname )
{
	if ( ( he = gethostbyname( hostname ) ) == NULL )
	{
		perror( "MessageSocket::setHostname(): gethostbyname() failed" );
		return false;
	}

	return true;
}

void MessageSocket::forcePortNumber( unsigned int newport )
{
	ourport = newport;
}



