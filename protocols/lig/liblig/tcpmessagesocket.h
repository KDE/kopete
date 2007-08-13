#ifndef TCPMESSAGESOCKET_H
#define TCPMESSAGESOCKET_H

#include <qptrlist.h>

#include "messagesocket.h"

class TCPMessageSocket : public MessageSocket
{
public:
	TCPMessageSocket();
	TCPMessageSocket( int newfd );
	virtual ~TCPMessageSocket();

	int connect( unsigned int portnum );
	int send( const char *sendbuffer, unsigned int length );
	int receive( char *recvbuffer, unsigned int maxlength );
	unsigned int listen( unsigned int portnum );
	int accept();
	int listenOnEvenPort( int min = 0, int max = 0 );

	bool setHostnamePort( const char *hostname, unsigned int port );
	bool cmpSocket( const char *hostname, unsigned int port );

private:
	in_addr_t addr;
	unsigned int portnum;
};


typedef QPtrListIterator<TCPMessageSocket> TCPMessageSocketIterator;


#endif // TCPMESSAGESOCKET_H_INCLUDED
