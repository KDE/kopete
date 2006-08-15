#include "libeva.h"
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

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

	ByteArray messageHeader( int sender, int receiver, const ByteArray& transferKey, short const type, short const sequence, int const timestamp, short const face = 0 )
	{
		// CODE DEBT: udp does not have the lenght placeholder!
		ByteArray data(64);
		data += htonl(sender);
		data += htonl(receiver);
		data += htons(Version);
		data += htonl(sender);
		data += htonl(receiver);
		data += transferKey;
		data += htons(type);
		data += htons(sequence);
		data += htonl(timestamp);
		data += htons(face);
		data += '\0';
		data += '\0';
		data += '\0';

		data += '\1'; // font info
		data += (int) 0;

		return data;
	}

	ByteArray encodeMessage( const ByteArray& text )
	{
		// TODO: remove the magic number!
		ByteArray encoded( 65536 );
		// TODO: implement the reply types later:
		// normal, image, auto
		encoded += NormalReply;
		// TODO: convert //img to the image resource image, using convertToSend method.
		encoded += text;
		encoded += char(0x20);
		encoded += char(0x0);

		// fontStyle
		// the layout is like this:
		// MSB --------------------- LSB
		// Underlined, Italic, Bold, size (5 bit)
		encoded += char(0x9); // font size = 9, normal decoration.
		encoded += char(0x0); // r
		encoded += char(0x0); // g
		encoded += char(0x0); // b
		encoded += char(0x0); // alpha? we mighe use 32-bit int to represent rgb later.
		encoded += htons( GBEncoding );

		// font name: Song Ti
		encoded += htonl(0xe5cccecb);

		encoded += char(0xd); // return 

		return encoded;
	}

	void setLength( ByteArray& data )
	{
		data.copyAt(0, htons(data.size()) );
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


	ByteArray encrypt( const ByteArray& text, const ByteArray& key );
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

	ByteArray requestTransferKey( int id, short const sequence, const ByteArray& key )
	{
		ByteArray text(1);
		text += TransferKey;
		
		return buildPacket(id, RequestKey, sequence, key, text );
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

	ByteArray textMessage( int id, short const sequence, ByteArray& key, 
	// Here are the message variables:
		int toId, const ByteArray& transferKey, ByteArray& message )
	{
		ByteArray text( 65536 );
		text += messageHeader( id, toId, transferKey, IMText, sequence, time(NULL));
		text += encodeMessage( message );
		return buildPacket(id, SendMsg, sequence, key, text );
	}
}
