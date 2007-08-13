/*
 * Copyright (c) 2000 Billy Biggs <bbiggs@div8.net>
 * Copyright (c) 2004 Wirlab <kphone@wirlab.net>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 */

#ifndef MESSAGESOCKET_H
#define MESSAGESOCKET_H

#include <netdb.h>
#include <netinet/in.h>

class MessageSocket
{
public:

	MessageSocket();
	virtual ~MessageSocket();

	virtual int connect( unsigned int portnum ) = 0;
	virtual int send( const char *sendbuffer, unsigned int length ) = 0;
	virtual int receive( char *recvbuffer, unsigned int maxlength ) = 0;
	virtual unsigned int listen( unsigned int portnum ) = 0;
	virtual int accept( void ) = 0;
	virtual int listenOnEvenPort( int min = 0, int max = 0 ) = 0;
	bool setHostname( const char *hostname );

	enum SocketType
	{
		None,
		SocketTCP,
		SocketUDP
	};

	int getFileDescriptor() const { return socketfd; }
	SocketType getSocketType() const { return type; }
	unsigned int getPortNumber() const { return ourport; }
	void forcePortNumber( unsigned int newport );

protected:
	struct hostent *he;
	int socketfd;
	SocketType type;
	unsigned int ourport;
	struct sockaddr_in socketaddress;
	struct sockaddr_in remoteaddress;
	bool bound;

private:
	int buffsize;
	char *ipaddress;
	char *hostname;
	char *callid;
	int port;
};

#endif
