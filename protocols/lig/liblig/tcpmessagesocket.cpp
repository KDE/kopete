#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>

#include "tcpmessagesocket.h"


TCPMessageSocket::TCPMessageSocket()
{
	type = SocketTCP;
	if ( ( socketfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		perror( "TCPMessageSocket(): socket() failed" );
	}
}

TCPMessageSocket::TCPMessageSocket( int newfd )
{
	type = SocketTCP;

	socketfd = newfd;
}

TCPMessageSocket::~TCPMessageSocket()
{
	close( socketfd );
}

int TCPMessageSocket::connect( unsigned int portnum )
{
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_port = htons( portnum );
	socketaddress.sin_addr = *( (struct in_addr *) he->h_addr );
	bzero( &( socketaddress.sin_zero ), 8 );

	if( ::connect( socketfd, (struct sockaddr *) &socketaddress, \
	    sizeof( struct sockaddr ) ) == -1 ) {
		perror( "TCPMessageSocket::connect(): connect() failed" );
		return -1;
	}

	return 0;
}

int TCPMessageSocket::send( const char *sendbuffer, unsigned int length )
{
	if ( ::send( socketfd, sendbuffer, length, 0 ) == -1 ) {
		perror( "TCPMessageSocket::send(): send() failed" );
		return -1;
	}

	return 0;
}

int TCPMessageSocket::receive( char *recvbuffer, unsigned int maxlength )
{
	int numbytes;

	if ( ( numbytes = recv( socketfd, recvbuffer, maxlength, 0 ) ) == -1 ) {
		perror( "TCPMessageSocket::recieve(): recv() failed" );
		return -1;
	}

	return numbytes;
}

unsigned int TCPMessageSocket::listen( unsigned int portnum )
{
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_port = htons( portnum );
	socketaddress.sin_addr.s_addr = INADDR_ANY;
	bzero( &( socketaddress.sin_zero ), 8 ); // is this portable?

	int count = 0;
	while ( ::bind( socketfd, (struct sockaddr *) &socketaddress, sizeof( struct sockaddr ) ) == -1 && count <= 10 ) {
		count++;
		portnum += 2;
		socketaddress.sin_port = htons( portnum );
	}
	if ( count > 10 ) {
		perror( "TCPMessageSocket::listen(): bind() failed" );
		return 0;
	}
	if ( ::listen( socketfd, 10 ) == -1 ) {
		perror( "TCPMessageSocket::listen(): listen() failed" );
		return 0;
	}

	return portnum;
}

int TCPMessageSocket::accept()
{
	int connectfd;
	socklen_t sockaddr_in_size;

	sockaddr_in_size = sizeof( struct sockaddr_in );

	if ( ( connectfd = ::accept( socketfd, (struct sockaddr *) &socketaddress, \
					&sockaddr_in_size ) ) == -1 ) {
		perror( "TCPMessageSocket::accept(): accept() failed" );
		return -1;
	}

	return connectfd;
}

int TCPMessageSocket::listenOnEvenPort( int min, int max )
{
	return -1;
}

bool TCPMessageSocket::setHostnamePort( const char *hostname, unsigned int port )
{
	if ( setHostname( hostname ) ) {
		addr = ((struct in_addr *) he->h_addr)->s_addr;
		portnum = port;
		return true;
	}
	return false;
}

bool TCPMessageSocket::cmpSocket( const char *hostname, unsigned int port )
{
	if ( ( he = gethostbyname( hostname ) ) == NULL ) {
		perror( "TCPMessageSocket::cmpSocket(): gethostbyname() failed" );
		return false;
	}

	if( addr == ((struct in_addr *) he->h_addr)->s_addr &&
			portnum == port ) {
		return true;
	} else {
		return false;
	}
}
