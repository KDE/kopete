#include "libeva.h"
#include "qbytearray.h"
#include <arpa/inet.h>

namespace Eva {
	QByteArray& header( int id, short const command, short const sequence )
	{
		QByteArray data;
		data += '0x0';
		data += '0x0';
		data += Head;
		data += Version;
		data += htons(command);
		data += htons(sequence);
		data += htonl(id);

		return data;
	}

	QByteArray& loginToken( int id, short const sequence )
	{
		QByteArray data = header( id, RequestLoginToken, sequence );
		// No need to encrypt
		data += '0x0';
		data += Tail;

		return data;
	}
}
		
		

