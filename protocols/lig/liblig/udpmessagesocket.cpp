#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>

#include "udpmessagesocket.h"


UDPMessageSocket::UDPMessageSocket()
{
	type = SocketUDP;
	if ( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
		perror( "UDPMessageSocket::UDPMessageSocket(): socket() failed" );
	}
	didcomplain = false;
}


UDPMessageSocket::UDPMessageSocket( int newfd )
{
	type = SocketUDP;
	socketfd = newfd;
	didcomplain = false;
}


UDPMessageSocket::~UDPMessageSocket()
{
	close( socketfd );
}


int UDPMessageSocket::connect( unsigned int portnum )
{
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_port = htons( portnum );
	socketaddress.sin_addr = *( (struct in_addr *) he->h_addr );
	bzero( &(socketaddress.sin_zero), 8 ); // is this portable?
	remoteaddress = socketaddress;
	return 0;
}


int UDPMessageSocket::SetTOS()
{
	unsigned char tos;
	socklen_t optlen;
#ifdef WIN32
      // Don't use IPTOS_PREC_CRITIC_ECP on Unix platforms as then need to be root
	tos=IPTOS_PREC_CRITIC_ECP|IPTOS_LOWDELAY;
#else 
	tos=IPTOS_LOWDELAY;
#endif
	optlen=1;
	if(setsockopt(socketfd,SOL_IP,IP_TOS,&tos,optlen) != 0){
		perror("UDPMessageSocket::SetTOS");
                return -1 ;
	}
	return 0;
}


int UDPMessageSocket::send( const char *sendbuffer, unsigned int length )
{
	int numbytes;
	if( ( numbytes = sendto( socketfd, sendbuffer, length, 0,
	    (struct sockaddr *) &remoteaddress, sizeof( struct sockaddr ) ) ) == -1 ) {
		if( !didcomplain ) {
			perror( "UDPMessageSocket::send(): sendto() failed" );
			didcomplain = true;
		}
		return -1;
	}
	return 0;
}


int UDPMessageSocket::receive( char *recvbuffer, unsigned int maxlength )
{
	socklen_t addrlength;
	int numbytes;
	addrlength = sizeof( struct sockaddr );
	if( ( numbytes = recvfrom( socketfd, recvbuffer, maxlength, 0, \
	    (struct sockaddr *) &remoteaddress, &addrlength ) ) == -1 ) {
		perror( "UDPMessageSocket::recieve(): recvfrom() failed" );
		return -1;
	}
	return numbytes;
}


unsigned int UDPMessageSocket::listen( unsigned int portnum )
{
	struct sockaddr name;
	socklen_t namesize;
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_port = htons( portnum );
	socketaddress.sin_addr.s_addr = INADDR_ANY;
	bzero( &(socketaddress.sin_zero), 8 ); // is this portable?

	/*
	 * if already bound then close this one and open
	 * a new socket so we can bind it to different port
	 * number
	 */
	if (bound) {
		close(socketfd);
		bound = false;
		if( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
			perror( "UDPMessageSocket::listen: socket() failed" );
			return 0;
		}
	}

	int count = 0;
	while ( bind( socketfd, (struct sockaddr *) &socketaddress, sizeof( struct sockaddr) ) == -1 && count <= 10 ) {
		count++;
		portnum += 2;
		socketaddress.sin_port = htons( portnum );
	}
	if ( count > 10 ) {
		perror( "UDPMessageSocket::listen(): bind() failed" );
		return 0;
	}
	bound = true;
	namesize = sizeof( struct sockaddr );
	bzero( &name, sizeof( struct sockaddr ) );
	if ( getsockname( socketfd, &name, &namesize ) == -1 ) {
		perror( "UDPMessageSocket::listen(): getsockname() failed" );
		return 0;
	}
	ourport = ntohs( ((struct sockaddr_in *) &name)->sin_port );
	return ourport;
}

/*
 * FIXME: There is no guarantee that the subsequent call
 * to socket and bind will return previous port number + 1 !
 * This function can potentially allocate several file
 * descriptors and never close them
 * comment made by jan@iptel.org
 */
int UDPMessageSocket::listenOnEvenPortOS()
{
	struct sockaddr name;
	socklen_t namesize;
	int oldsocketfd = 0;
	int portnum = 0;

	/*
	 * if already bound then close this one and open
	 * a new socket so we can bind it to different port
	 * number
	 */
	if (bound) {
		close(socketfd);
		bound = false;
		if( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
			perror( "UDPMessageSocket::listenOnEvenPortOS: socket() failed" );
			return -1;
		}
	}

getaddress:
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_port = htons( portnum );
	socketaddress.sin_addr.s_addr = INADDR_ANY;
	bzero( &(socketaddress.sin_zero), 8 ); // is this portable?
	if ( bind( socketfd, (struct sockaddr *) &socketaddress, sizeof( struct sockaddr) ) == -1 ) {
		perror( "UDPMessageSocket::listen(): bind() failed" );
		return -1;
	}
	bound = true;
	namesize = sizeof( struct sockaddr );
	bzero( &name, sizeof( struct sockaddr ) );
	if ( getsockname( socketfd, &name, &namesize ) == -1 ) {
		perror( "UDPMessageSocket::listen(): getsockname() failed" );
		return -1;
	}
	ourport = ntohs( ((struct sockaddr_in *) &name)->sin_port );
	printf( "UDPMessageSocket: Listening on %d\n", ourport );
	if( ( portnum == 0 ) && ( ourport % 2 ) != 0 ) {
		printf( "UDPMessageSocket: Retrying...\n" );
		oldsocketfd = socketfd;
		if( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
			perror( "UDPMessageSocket::UDPMessageSocket(): socket() failed" );
		}
		goto getaddress;
	}
	if( oldsocketfd != 0 )
		close( oldsocketfd );
	return 0;
}


int UDPMessageSocket::listenOnEvenPort(int min, int max)
{
	/* User didn't specify any range, let the OS
	 * assign source port number
	 */
	if ((min == 0) && (max == 0)) {
		return listenOnEvenPortOS();
	}

	/* Only max specified, set min to the lowest limit */
	if (min == 0) min = 1024;

	/* Only min specified, set max to the highest port number */
	if (max == 0) max = 65535;

	/* min not odd, increase */
	if (min % 2) min++;

	/* Out of range ? Signal error */
	if (min > max) goto error;

	/*
	 * if already bound then close this one and open
	 * a new socket so we can bind it to different port
	 * number
	 */
	if (bound) {
		close(socketfd);
		bound = false;
		if( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
			perror( "UDPMessageSocket::listenOnEvenPor: socket() failed" );
			return -1;
		}
	}

loop:
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_port = htons(min);
	socketaddress.sin_addr.s_addr = INADDR_ANY;
	bzero(&(socketaddress.sin_zero), 8);
	if (bind(socketfd, (struct sockaddr*)&socketaddress, sizeof(struct sockaddr)) == -1) {
		min += 2;
		if (min <= max) goto loop;
		else goto error;
	}
	
	bound = true;

	/*
	 * FIXME: Do some test for even port number here
	 */

	ourport = min;
	printf("UDPMessageSocket: Listening on %d\n", min);
	return 0;

error:
	perror("UDPMessageSocket::listen(): Can't find a free port in specified range");
	return -1;
	
}


int UDPMessageSocket::listenOnEvenPortForVideo()
{
	struct sockaddr name;
	socklen_t namesize;
	int oldsocketfd = 0;
	int portnum = 0;

	if ( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
		perror( "UDPMessageSocket::UDPMessageSocket(): socket() failed" );
	}

getaddress:
	socketaddress.sin_family = AF_INET;
	socketaddress.sin_port = htons( portnum );
	socketaddress.sin_addr.s_addr = INADDR_ANY;
	bzero( &(socketaddress.sin_zero), 8 ); // is this portable?

	int one = 1;
	if ( setsockopt( socketfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof( one ) ) == -1 ) {
		perror( "UDPMessageSocket::setsockopt SO_REUSEADDR" );
	}

	if ( bind( socketfd, (struct sockaddr *) &socketaddress, sizeof( struct sockaddr) ) == -1 ) {
		perror( "UDPMessageSocket::listen(): bind() failed" );
		return -1;
	}

	bound = true;

	namesize = sizeof( struct sockaddr );
	bzero( &name, sizeof( struct sockaddr ) );
	if ( getsockname( socketfd, &name, &namesize ) == -1 ) {
		perror( "UDPMessageSocket::listen(): getsockname() failed" );
		return -1;
	}
	ourport = ntohs( ((struct sockaddr_in *) &name)->sin_port );
	printf( "UDPMessageSocket (video): Listening on %d\n", ourport );
	if( portnum == 0 && ( ourport % 4 ) != 0 ) {
		printf( "UDPMessageSocket: Retrying...\n" );
		oldsocketfd = socketfd;
		if( ( socketfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) == -1 ) {
			perror( "UDPMessageSocket::UDPMessageSocket(): socket() failed" );
		}
		goto getaddress;
	}
	if( oldsocketfd != 0 )
		close( oldsocketfd );

	return 0;
}


int UDPMessageSocket::accept( void )
{
	return socketfd;
}
