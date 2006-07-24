#include "libeva.h"
#include "md5.h"
#include "crypt.h"
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

namespace Eva {
	static const char login_16_51 [] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x29, 0xc0, 0xf8, 0xc4, 0xbe, 
		0x3b, 0xee, 0x57, 0x92, 0xd2, 0x42, 0xa6, 0xbe, 
		0x41, 0x98, 0x97, 0xb4 };
	static const char login_53_68 []= {
		0xce, 0x11, 0xd5, 0xd9, 0x97, 0x46, 0xac, 0x41, 
		0xa5, 0x01, 0xb2, 0xf5, 0xe9, 0x62, 0x8e, 0x07 };

	static const char login_94_193 []= {
		0x01, 0x40, 0x01, 0xb6, 0xfb, 0x54, 0x6e, 0x00, 
		0x10, 0x33, 0x11, 0xa3, 0xab, 0x86, 0x86, 0xff,
		0x5b, 0x90, 0x5c, 0x74, 0x5d, 0xf1, 0x47, 0xbf,
		0xcf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00 };
	const char init_key[] = {
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

	const char* getInitKey() 
	{
		return init_key;
	}


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

	int rand(void)
	{
		return 0xdead;
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

	inline void encrypt64( unsigned char* plain, unsigned char* plain_pre, 
			unsigned char* key, unsigned char* crypted, unsigned char* crypted_pre, 
			bool& isHeader )
	{
		int i;

		for( i = 0; i< 8; i++ )
			plain[i] ^= isHeader ? plain_pre[i] : crypted_pre[i];

		TEA::encipher( (unsigned int*) plain, (unsigned int*) key, 
				(unsigned int*) crypted );

		for( i = 0; i< 8; i++ )
			crypted[i] ^= plain_pre[i];

		memcpy( plain_pre, plain, 8 );
		memcpy( crypted_pre, crypted, 8 );
			
		isHeader = false;
	}

	inline void decrypt64( unsigned char* crypt, unsigned char* crypt_pre, 
			unsigned char* key, unsigned char* decrypted)
	{
		for( int i = 0; i< 8; i++ )
			decrypted[i] ^= crypt[i];

		TEA::decipher( (unsigned int*) decrypted,
				(unsigned int*) key, (unsigned int*) decrypted );
		/*
		fprintf( stderr, "decrypt64 : " );
		for( int i = 0; i< 8; i++ )
			fprintf( stderr, "%x ", decrypted[i] );
		fprintf( stderr, "\n" );
		*/
	}

	
	// Interface for application
	// Utilities
	ByteArray loginToken( const ByteArray& packet ) 
	{
		char reply = packet.data()[0];
		char length = packet.data()[1];
		ByteArray data(length);

		if( reply != LoginTokenOK )
			return data;

		data.append( packet.data()+2, length );
		return data;
	}

	ByteArray QQHash( const ByteArray& text )
	{
		return doMd5( doMd5( text ) );
	}

	ByteArray encrypt( const ByteArray& text, const ByteArray& key )
	{

		unsigned char 
			plain[8],         /* plain text buffer*/
			plain_pre[8],   /* plain text buffer, previous 8 bytes*/
			crypted[8],        /* crypted text*/
			crypted_pre[8];  /* crypted test, previous 8 bytes*/
	
		int pos, len, i;
		bool isHeader = true;      /* header is one byte*/
		ByteArray encoded( text.size() + 32 );
		
		pos = ( text.size() + 10 ) % 8;
		if( pos )
			pos = 8 - pos;

		// Prepare the first 8 bytes:
		plain[0] = ( rand() & 0xf8 ) | pos;
		memset( plain_pre, 0, 8 );
		memset( crypted_pre, 0, 8 );
		memset( plain+1, rand()& 0xff, pos++ );

		// pad 2 bytes
		for( i = 0; i< 2; i++ )
		{
			if( pos < 8 )
				plain[pos++] = rand() & 0xff;

			if( pos == 8 )
			{
				encrypt64( plain, plain_pre, (unsigned char*)key.data(), crypted, crypted_pre, isHeader );
				pos = 0;
				encoded.append( (char*)crypted, 8 );
			}
		}

		for( i = 0; i< text.size(); i++ )
		{
			if( pos < 8 )
				plain[pos++] = text.data()[i];

			if( pos == 8 )
			{
				encrypt64( plain, plain_pre, (unsigned char*)key.data(), crypted, crypted_pre, isHeader );
				pos = 0;
				encoded.append( (char*)crypted, 8 );
			}
		}

		for( i = 0; i< 7; i++ )
		{
			if( pos < 8 )
				plain[pos++] = 0;

			if( pos == 8 )
			{
				encrypt64( plain, plain_pre, (unsigned char*)key.data(), crypted, crypted_pre, isHeader );
				encoded.append( (char*)crypted, 8 );
				break;
			}
		}

		return encoded;
	}

	ByteArray decrypt( const ByteArray& code, const ByteArray& key )
	{
		unsigned char
			decrypted[8], m[8],
			*crypt_pre, *crypt;
		char* outp;
		
		int pos, len, i;

		if( code.size() < 16 || code.size() % 8 )
			return ByteArray(0);

		TEA::decipher( (unsigned int*) code.data(), 
				(unsigned int*) key.data(), (unsigned int*) decrypted );
		pos = decrypted[0] & 0x7;
		len = code.size() - pos - 10;
		if( len < 0 )
			return ByteArray(0);

		ByteArray text(len);
		memset( m, 0, 8 );
		crypt = (unsigned char*)code.data() + 8;
		crypt_pre = m;
		pos ++;

		for( i = 0; i< 2; )
		{
			if( pos < 8 )
			{
				pos ++;
				i++;
			}
			if( pos == 8 )
			{
				crypt_pre = (unsigned char*) code.data();
				decrypt64( crypt, crypt_pre, (unsigned char*) key.data(), 
						decrypted ); 
				crypt += 8;
				pos = 0;
			}
		}
		for( i = 0; i< len;  )
		{
			if( pos < 8 )
			{
				text += (char) (crypt_pre[pos] ^ decrypted[pos]);
				pos ++;
				i ++;
			}
			if( pos == 8 )
			{
				crypt_pre = crypt - 8;
				decrypt64( crypt, crypt_pre, (unsigned char*) key.data(), decrypted );
				crypt += 8;
				pos = 0;
			}
		}
		
		for( i = 0; i< 7; i++ )
		{
			if( pos < 8 )
			{
				if( crypt_pre[pos] ^ decrypted[pos] )
					return ByteArray(0);
				pos ++;
			}
			if( pos == 8 )
			{
				crypt_pre = crypt;
				decrypt64( crypt, crypt_pre, (unsigned char*) key.data(), decrypted );
				break;
			}
				
		}

		return text;
	}

	ByteArray buildPacket( int id, short const command, short const sequence, const ByteArray& key, const ByteArray& text )
	{
		ByteArray packet(MaxPacketLength);
		packet += header( id, command, sequence );
		packet += encrypt( text, key );
		packet += Tail;
		setLength( packet );
		return packet;
	}

	ContactInfo Packet::contactInfo( char* buffer, int& len )
	{
		ContactInfo ci;
		buffer += len;
		ci.id = ntohl( Eva::type_cast<int> (buffer) );
		ci.face = ntohs( Eva::type_cast<short> (buffer+4) ); 
		ci.age = buffer[6];
		ci.gender = buffer[7];
		int nl = (int) buffer[8];
		ci.nick = std::string( strndup( buffer+9, nl) );
		// 2 bytes are unknown.
		// 2 bytes are extFlag, commonFlag, ignored here.
		len += 9+nl+4;
		return ci;
	}

	std::list< std::string > Packet::groupNames(const ByteArray& text )
	{
		// starts from 7, and each field is 17 bytes long.
		std::list< std::string > list;
		int offset = 7;
		while( offset < text.size() )
		{
			std::string name( text.data() + offset );
			list.push_back( name );
			offset += 17;
		}
		return list;
	}

	std::list< CGT > Packet::cgts( const ByteArray& text )
	{
		int offset = 10;
		std::list< CGT > list;
		while( offset < text.size() )
		{
			list.push_back( 
				CGT( ntohl( type_cast<int>( text.data()+offset ) ), 
				type_cast<char>(text.data()+offset+4), ( type_cast<short>(text.data()+5) >> 2) & 0x3f ) );
			offset += 6;
		}
		return list;
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

	ByteArray login( int id, short const sequence, const ByteArray& key, 
			const ByteArray& token, char const loginMode )
	{
		ByteArray login(LoginLength);
		ByteArray data(MaxPacketLength);
		ByteArray initKey( (char*)init_key, 16 );

		ByteArray nil(0);
		login += encrypt( nil, key );
		login.append( login_16_51, 36 );
		login += loginMode;
		login.append( login_53_68, 16 );
		login += (char) (token.size());
		login += token;
		login.append( login_94_193, 100 );
		memset( login.data()+login.size(), 0, login.capacity()-login.size() );
		login.setSize( login.capacity() );

		data += header( id, Login, sequence );
		data += initKey;
		data += encrypt( login, initKey );
		data += Tail;
		setLength( data );

		initKey.release(); // static data, no need to free

		return data;
	}

	ByteArray contactList( int id, short const sequence, const ByteArray& key, short pos )
	{
		ByteArray text(5);
		text += pos;
		// FIXME: DO not use hardcoded sort/unsorted.
		text += ContactListSorted;
		text += '\0';
		text += '\1';

		return buildPacket(id, ContactList, sequence, key, text );
	}

	ByteArray changeStatus( int id, short const sequence, ByteArray& key, char status )
	{
		ByteArray text(5);
		text += status;
		text += (int) 0;
		return buildPacket(id, ChangeStatus, sequence, key, text );
	}

	ByteArray getGroupNames( int id, short const sequence, ByteArray& key )
	{
		ByteArray text(6);
		text += DownloadGroupNames;
		text += '\2' ;
		text += (int) 0;

		return buildPacket(id, GroupNames, sequence, key, text );
	}

	ByteArray downloadGroups( int id, short const sequence, ByteArray& key, int pos )
	{
		ByteArray text(10);
		text += '\1';
		text += '\2' ;
		text += (int) 0;
		text += htonl(pos);

		return buildPacket(id, DownloadGroups, sequence, key, text );
	}

}
