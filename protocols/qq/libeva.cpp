#include "libeva.h"
#include <arpa/inet.h>

namespace Eva {

	ByteArray& setHeader( ByteArray& data, int id, short const command, short const sequence )
	{
		data += '\0';
		data += '\0';
		data += Head;
		data += htons(Version);
		data += htons(command);
		data += htons(sequence);
		data += htonl(id);

		return data;
	}

	ByteArray& setLength( ByteArray& data )
	{
		char hi, lo;
		lo = data.size() & 0xFF;
		hi = (data.size() >> 8 ) & 0xFF;

		// override [] operator ?
		// Add copyAt() ?
		data.data()[0] = hi;
		data.data()[1] = lo;

		return data;
	}

	ByteArray loginToken( int id, short const sequence )
	{
		ByteArray data(15);
		setHeader( data, id, RequestLoginToken, sequence );
		// No need to encrypt
		data += '\0';
		data += Tail;
		setLength( data );

		return data;
	}
}
		
		

