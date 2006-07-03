#include "libeva.h"
#include "md5.h"
#include <arpa/inet.h>

namespace Eva {
	ByteArray header( int id, short const command, short const sequence )
	{
		// CODE DEBT: udp does not have the lenght placeholder!
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
	
	ByteArray doMd5( const ByteArray& text )
	{
		ByteArray code( Md5KeyLength );
		md5_state_t ctx;
		md5_init( &ctx );
		md5_append( &ctx, (md5_byte_t*) text.data(), text.size() );
		md5_finish( &ctx, (md5_byte_t*)code.data() );
		code.setSize( Md5KeyLength ); 
		return code;
	}
	
	// Interface for application
	// Utilities
	ByteArray loginToken( char const* buffer ) 
	{
		int length = buffer[1];
		length -= 3; // length - 1(head) - 1(length) - 1(tail) 
		ByteArray data(length);

		if( buffer[0] != LoginTokenOK )
			return data;
		data.append( buffer+2, length );
		return data;
	}

	ByteArray QQHash( const ByteArray& text )
	{
		return doMd5( doMd5( text ) );
	}

	// Core functions
	ByteArray requestLoginToken( int id, short const sequence )
	{
		ByteArray data(16);
		data += header(id, RequestLoginToken, sequence);
		data += '\0';
		data += Tail;
		setLength( data );

		return data;
	}

	ByteArray login( int id, short const sequence )
	{
		// some allowance :-)
		ByteArray data(HeaderLength+KeyLength+32);
		data += header( id, Login, sequence );

		// initial key
		for( int i = 0; i< KeyLength; i++ )
			data += char(1);

		// TODO: TO BE CONTINUED
		return data;
	}

		
		
}
		
		

