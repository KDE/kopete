#include "libeva.h"
#include <arpa/inet.h>

namespace Eva {

	ByteArray header( int id, short const command, short const sequence )
	{
		// FIXME: add resize support in the ByteArray !
		ByteArray data(32);
		data += '\0';
		data += '\0';
		data += Head;
		data += Version;
		data += htons(command);
		data += htons(sequence);
		data += htonl(id);

		return data;
	}

	ByteArray loginToken( int id, short const sequence )
	{
		ByteArray data = header( id, RequestLoginToken, sequence );
		// No need to encrypt
		data += '0x0';
		data += Tail;

		return data;
	}
}
		
		

