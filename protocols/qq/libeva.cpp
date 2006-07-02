#include "libeva.h"
#include <arpa/inet.h>

namespace Eva {

	ByteArray header( int id, short const command, short const sequence )
	{
		ByteArray data(13);
		data += '\0';
		data += '\0';
		data += Head;
		data += htons(Version);
		data += htons(command);
		data += htons(sequence);
		data += htonl(id);

		return data;
	}

	void setLength( ByteArray& data )
	{
		data.copyAt(0, htons(data.size()) );
	}

	ByteArray requestLoginToken( int id, short const sequence )
	{
		ByteArray data(16);
		data.append( header(id, RequestLoginToken, sequence) );
		data += '\0';
		data += Tail;
		setLength( data );

		return data;
	}

	ByteArray loginToken( char const* buffer ) 
	{
		int length = buffer[1];
		length -= 3; // length - 1(head) - 1(length) - 1(tail) 
		ByteArray data(length);

		if( buffer[0] != LoginTokenOK )
			return data;
		// data.append( buffer+2, length );
		return data;
	}
}
		
		

