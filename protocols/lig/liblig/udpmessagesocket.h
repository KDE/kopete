#ifndef UDPMESSAGESOCKET_H
#define UDPMESSAGESOCKET_H

#include "messagesocket.h"

class UDPMessageSocket : public MessageSocket
{
public:
	UDPMessageSocket();
	UDPMessageSocket( int newfd );
	~UDPMessageSocket();
	int connect( unsigned int portnum );
	int SetTOS();
	int send( const char *sendbuffer, unsigned int length );
	int receive( char *recvbuffer, unsigned int maxlength );
	unsigned int listen( unsigned int portnum );
	int accept();
	int listenOnEvenPortOS();
	int listenOnEvenPort( int min = 0, int max = 0 );
	int listenOnEvenPortForVideo();
private:
	bool didcomplain;
};

#endif // UDPMESSAGESOCKET_H_INCLUDED
