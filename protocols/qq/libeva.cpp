#include "libeva.h"
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

namespace Eva {
	static const uchar login_16_51 [] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x29, 0xc0, 0xf8, 0xc4, 0xbe, 
		0x3b, 0xee, 0x57, 0x92, 0xd2, 0x42, 0xa6, 0xbe, 
		0x41, 0x98, 0x97, 0xb4 };
	static const uchar login_53_68 []= {
		0xce, 0x11, 0xd5, 0xd9, 0x97, 0x46, 0xac, 0x41, 
		0xa5, 0x01, 0xb2, 0xf5, 0xe9, 0x62, 0x8e, 0x07 };

	static const uchar login_94_193 []= {
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
	static const uchar init_key[] = {
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };



	const uchar* Packet::getInitKey() 
	{
		return init_key;
	}


	ByteArray header( uint id, short const command, ushort sequence )
	{
		// CODE DEBT: udp does not have the length placeholder!
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

	ByteArray messageHeader( int sender, int receiver, const ByteArray& transferKey, short const type, ushort sequence, int const timestamp, short const face = 0 )
	{
		// CODE DEBT: udp does not have the length placeholder!
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
		encoded += htonl(0xcbcecce5);

		encoded += char(0xd); // return 

		return encoded;
	}

	void setLength( ByteArray& data )
	{
		data.copyAt(0, htons(data.size()) );
	}

	// Utilities functions from Packet
	// Get information from the raw packet.
	ByteArray Packet::loginToken( const ByteArray& packet ) 
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
	ByteArray Packet::create( uint id, ushort command, ushort sequence, const ByteArray& key, const ByteArray& text )
	{
		ByteArray packet(MaxPacketLength);
		packet += header( id, command, sequence );
		packet += encrypt( text, key );
		packet += Tail;
		setLength( packet );
		return packet;
	}

	// FIXME: use list instead.
	ContactInfo Packet::contactInfo( char* buffer, int& len )
	{
		ContactInfo ci;
		buffer += len;
		ci.id = ntohl( Eva::type_cast<int> (buffer) );
		ci.face = ntohs( Eva::type_cast<short> (buffer+4) ); 
		ci.age = buffer[6];
		ci.gender = buffer[7];
		int nl = (int) buffer[8];
		ci.nick = std::string( buffer+9, nl );
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
			std::string name( text.c_str() + offset );
			list.push_back( name );
			offset += 17;
		}
		return list;
	}

	std::list< GroupInfo > Packet::groupInfos( const ByteArray& text )
	{
		int offset = 10;
		std::list< GroupInfo > list;
		while( offset < text.size() )
		{
			list.push_back( 
				GroupInfo( ntohl( type_cast<int>( text.data()+offset ) ), 
				type_cast<char>(text.data()+offset+4), ( type_cast<short>(text.data()+5) >> 2) & 0x3f ) );
			offset += 6;
		}
		return list;
	}

	std::list< ContactStatus > Packet::onlineContacts( const ByteArray& text, uchar& pos )
	{
		int offset = 1;
		std::list< ContactStatus > list;
		pos = text.data()[0];

		while( offset < text.size() )
		{
			list.push_back( 
				ContactStatus( text.data()+offset ) );
			offset += 31;
		}
		return list;
	}

	std::map<const char*, std::string, Eva::ltstr> Packet::contactDetail( const ByteArray& text )
	{
		std::map<const char*, std::string, ltstr> dict;
		static const uchar Divider = 0x1e;
		int start = 0;
		int index = 0;

		for( int i = 0; i< text.size(); i++ )
		{
			if( text.data()[i] != Divider )
				continue;

			dict[ contactDetailIndex[index++] ] = std::string(text.c_str()+start, i-start);
			start = i+1;
		}
		// the last field
		dict[ contactDetailIndex[index++] ] = std::string(text.c_str()+start, text.size()-start);
		return dict;
	}
				
	// Core functions
	ByteArray loginToken( uint id, ushort sequence )
	{
		ByteArray data(16);
		data += header(id, Command::RequestLoginToken, sequence);
		data += '\0';
		data += Tail;
		setLength( data );

		return data;
	}

	ByteArray login( uint id, ushort sequence, const ByteArray& key, 
			const ByteArray& token, uchar loginMode )
	{
		ByteArray login(LoginLength);
		ByteArray data(MaxPacketLength);
		ByteArray initKey( (char*)init_key, 16 );

		ByteArray nil(0);
		login += Packet::encrypt( nil, key );
		login.append( login_16_51, 36 );
		login += loginMode;
		login.append( login_53_68, 16 );
		login += (char) (token.size());
		login += token;
		login.append( login_94_193, 100 );
		memset( login.data()+login.size(), 0, login.capacity()-login.size() );
		login.setSize( login.capacity() );

		data += header( id, Command::Login, sequence );
		data += initKey;
		data += Packet::encrypt( login, initKey );
		data += Tail;
		setLength( data );

		initKey.release(); // static data, no need to free

		return data;
	}

	ByteArray transferKey( uint id, ushort sequence, const ByteArray& key )
	{
		ByteArray text(1);
		text += TransferKey;
		
		return Packet::create(id, Command::RequestKey, sequence, key, text );
	}

	ByteArray allContacts( uint id, ushort sequence, const ByteArray& key, short pos )
	{
		ByteArray text(5);
		text += pos;
		// FIXME: DO not use hardcoded sort/unsorted.
		text += ContactListSorted;
		text += '\0';
		text += '\1';

		return Packet::create(id, Command::AllContacts, sequence, key, text );
	}

	ByteArray statusUpdate( uint id, ushort sequence, const ByteArray& key, uchar status )
	{
		ByteArray text(5);
		text += status;
		text += (int) 0;
		return Packet::create(id, Command::ChangeStatus, sequence, key, text );
	}

	ByteArray contactDetail( uint id, ushort sequence, const ByteArray& key, int qqId )
	{
		ByteArray text(32);
		snprintf( text.c_str(), 31, "%d", qqId );
		text.setSize( strlen( text.c_str() ) );
		return Packet::create(id, Command::UserInfo, sequence, key, text );
	}

	ByteArray groupNames( uint id, ushort sequence, const ByteArray& key )
	{
		ByteArray text(6);
		text += DownloadGroupNames;
		text += '\2' ;
		text += (int) 0;

		return Packet::create(id, Command::GroupNames, sequence, key, text );
	}

	ByteArray downloadGroups( uint id, ushort sequence, const ByteArray& key, int pos )
	{
		ByteArray text(10);
		text += '\1';
		text += '\2' ;
		text += (int) 0;
		text += htonl(pos);

		return Packet::create(id, Command::DownloadGroups, sequence, key, text );
	}

	ByteArray textMessage( uint id, ushort sequence, const ByteArray& key, 
	// Here are the message variables:
		int toId, const ByteArray& transferKey, ByteArray& message )
	{
		ByteArray text( 65536 );
		text += messageHeader( id, toId, transferKey, IMText, sequence, time(NULL));
		text += encodeMessage( message );
		return Packet::create(id, Command::SendMsg, sequence, key, text );
	}

	ByteArray messageReply(uint id, ushort sequence, const ByteArray& key, const ByteArray& text )
	{
		return Packet::create(id, Command::ReceiveMsg, sequence, key, text );
	}

	ByteArray heartbeat(uint id, ushort sequence, const ByteArray& key )
	{
		ByteArray text(4);
		text += id;
		return Packet::create(id, Command::Heartbeat, sequence, key, text );
	}

	ByteArray onlineContacts(uint id, ushort sequence, const ByteArray& key, uchar pos )
	{
		ByteArray text(5);
		text += uchar(0x02);
		text += pos;
		text += uchar(0x00);
		text += uchar(0x00);
		text += uchar(0x00);
		return Packet::create(id, Command::ContactsOnline, sequence, key, text );
	}
		
}
