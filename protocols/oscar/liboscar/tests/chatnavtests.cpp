/*
   Kopete Oscar Protocol - Chat Navigation parsing tests
   Copyright (c) 2005 Matt Rogers <mattr@kde.org>

   Kopete (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#include "chatnavtests.h"

#include <iostream>

#include "buffer.h"
#include "oscartypeclasses.h"

using namespace std;
using namespace Oscar;


ChatNavTests::ChatNavTests()
{
	m_buffer = 0;
}


ChatNavTests::~ChatNavTests()
{
}

void ChatNavTests::setupExchangeTestBuffer()
{
	delete m_buffer;
	m_buffer = 0;

	m_buffer = new Buffer();
	//TLV 0x02
	m_buffer->addDWord(0x00020001);
	m_buffer->addByte(0x03);
	//TLV 0x03
	m_buffer->addDWord(0x0003003C);
	m_buffer->addDWord(0x0001000a);
	m_buffer->addDWord(0x00030001);
	m_buffer->addDWord(0x14000400);
	m_buffer->addDWord(0x02200000);
	m_buffer->addDWord(0xC9000200);
	m_buffer->addDWord(0x4400CA00);
	m_buffer->addDWord(0x04000000);
	m_buffer->addDWord(0x0000D000);
	m_buffer->addDWord(0x0000D100);
	m_buffer->addDWord(0x0207D000);
	m_buffer->addDWord(0xD2000200);
	m_buffer->addDWord(0x2F00D400);
	m_buffer->addDWord(0x0000D500);
	m_buffer->addDWord(0x010100DA);
	m_buffer->addDWord(0x00020066);
}

void ChatNavTests::setupRoomInfoTestBuffer()
{

	delete m_buffer;
	m_buffer = 0;

	m_buffer = new Buffer();
	//TLV 0x04
	m_buffer->addDWord(0x000400F8);
	m_buffer->addWord(0x0004); //exchange
	m_buffer->addByte(0x28); //cookie length
	m_buffer->addByte(0x21); //start of cookie
	m_buffer->addDWord(0x616F6C3A);
	m_buffer->addDWord(0x2F2F3237);
	m_buffer->addDWord(0x31393A31);
	m_buffer->addDWord(0x302D342D);
	m_buffer->addDWord(0x63686174);
	m_buffer->addDWord(0x37343739);
	m_buffer->addDWord(0x33333134);
	m_buffer->addDWord(0x30313137);
	m_buffer->addDWord(0x37393435);
	m_buffer->addDWord(0x36363500);
	m_buffer->addDWord(0x00020016);
	m_buffer->addDWord(0x00660002);
	m_buffer->addDWord(0x00000068);
	m_buffer->addDWord(0x00040000);
	m_buffer->addDWord(0x0000006A);
	m_buffer->addDWord(0x00176368);
	m_buffer->addDWord(0x61743734);
	m_buffer->addDWord(0x37393333);
	m_buffer->addDWord(0x31343031);
	m_buffer->addDWord(0x31373739);
	m_buffer->addDWord(0x34353636);
	m_buffer->addDWord(0x35006D00);
	m_buffer->addDWord(0x02000000);
	m_buffer->addDWord(0x6E000200);
	m_buffer->addDWord(0x00006F00);
	m_buffer->addDWord(0x02000000);
	m_buffer->addDWord(0x71000200);
	m_buffer->addDWord(0x00007500);
	m_buffer->addDWord(0x04000000);
	m_buffer->addDWord(0x0000C900);
	m_buffer->addDWord(0x02004000);
	m_buffer->addDWord(0xCA000442);
	m_buffer->addDWord(0xBEF90500);
	m_buffer->addDWord(0xD0000200);
	m_buffer->addDWord(0x0300D100);
	m_buffer->addDWord(0x0207D000);
	m_buffer->addDWord(0xD2000200);
	m_buffer->addDWord(0x2600D300);
	m_buffer->addDWord(0x17636861);
	m_buffer->addDWord(0x74373437);
	m_buffer->addDWord(0x39333331);
	m_buffer->addDWord(0x34303131);
	m_buffer->addDWord(0x37373934);
	m_buffer->addDWord(0x35363635);
	m_buffer->addDWord(0x00D40000);
	m_buffer->addDWord(0x00D50001);
	m_buffer->addDWord(0x0100D600);
	m_buffer->addDWord(0x0875732D);
	m_buffer->addDWord(0x61736369);
	m_buffer->addDWord(0x6900D700);
	m_buffer->addDWord(0x02656E00);
	m_buffer->addDWord(0xD8000875);
	m_buffer->addDWord(0x732D6173);
	m_buffer->addDWord(0x63696900);
	m_buffer->addDWord(0xD9000265);
	m_buffer->addDWord(0x6E00DB00);
	m_buffer->addDWord(0x0D756578);
	m_buffer->addDWord(0x742F782D);
	m_buffer->addDWord(0x616F6C72);
	m_buffer->addDWord(0x746600DA);
	m_buffer->addDWord(0x000200E8);
}

void ChatNavTests::allTests()
{
	exchangeParsingTest();
	roominfoParsingTest();
}

void ChatNavTests::exchangeParsingTest()
{
	setupExchangeTestBuffer();
	Buffer testBuffer(*m_buffer);
	CHECK( testBuffer.length() != 0, true );
	while ( testBuffer.length() != 0 )
	{
		TLV t = testBuffer.getTLV();
		if ( t.type == 0x0002 )
		{
// 			cout << "Max concurrent rooms: " << t.data << endl;
		}
		
		t = testBuffer.getTLV();
		if ( t.type == 0x0003 )
		{
// 			cout << "TLV of type 3 with length " << t.length << endl;
			Buffer b(t.data);
			WORD id = b.getWord();
			CHECK( id > 0, true );
			int tlvCount = b.getWord();
			int realCount = 0;
// 			cout << "Expecting " << tlvCount << " TLVs" << endl;
			while ( b.length() > 0 )
			{
				TLV t = b.getTLV();
				CHECK( t.type != 0, true );
				switch (t.type)
				{
				case 0x02:
					CHECK( t.length == 2, true );
					CHECK( t.data.count() == 2, true );
					break;
				case 0x03:
					CHECK( t.length == 1, true );
					CHECK( t.data.count() == 1, true );
					CHECK( t.data[0] > 0, true );
					break;
				case 0x04:
					CHECK( t.length == 2, true );
					CHECK( t.data.count() == 2, true );
					//CHECK( t.data[0] > 0, true );
					break;
				case 0x05:
					CHECK( t.length > 1, true );
					break;
				case 0x06:
					CHECK( t.length > 2, true );
					break;
				case 0xCA:
					CHECK( t.length == 4, true );
					break;
				case 0xD1:
					CHECK( t.length == 2, true );
					break;
				case 0xD2:
					CHECK( t.length == 2, true );
					break;
				case 0xD3:
					CHECK( t.length > 0, true );
					CHECK( t.data.count() == t.length, true );
					break;
				case 0xD5:
					CHECK( t.length == 1, true );
					break;
				default:
// 					ios::fmtflags origFlags = cout.flags(ios::hex|ios::showbase);
// 					cout << "unknown TLV type " << t.type << endl;
// 					cout.flags(origFlags);
					break;
				}
				realCount++;
			}
			CHECK( tlvCount == realCount, true );
		}
		CHECK( testBuffer.length() == 0, true );
	}
}

void ChatNavTests::roominfoParsingTest()
{
	setupRoomInfoTestBuffer();
	Buffer testBuffer(*m_buffer);
	CHECK( testBuffer.length() != 0, true );
	while ( testBuffer.length() != 0 )
	{
		TLV t = testBuffer.getTLV();
		
// 		cout << "TLV of type " << t.type << " with length " << t.length << endl;
		
		
		CHECK( t.type == 0x04, true );
		CHECK( t.length > 8, true );
		Buffer b( t.data );
		CHECK( b.getWord() > 0, true );
		BYTE cookieLength = b.getByte();
		b.skipBytes( cookieLength );
		CHECK( b.getWord() == 0, true );
		CHECK( b.getByte() > 0, true );
		int tlvCount = b.getWord();
		int realCount = 0;
// 		cout << "Expecting " << tlvCount << " TLVs" << endl;
		while ( b.length() > 0 )
		{
			TLV t = b.getTLV();
// 			ios::fmtflags origFlags = cout.flags(ios::hex|ios::showbase);
// 			cout << "TLV of type " << t.type << endl;
// 			cout.flags(origFlags);
			CHECK( t.type != 0, true );
			switch (t.type)
			{
			case 0x02:
				CHECK( t.length == 2, true );
				CHECK( t.data.count() == 2, true );
				break;
			case 0x03:
				CHECK( t.length == 1, true );
				CHECK( t.data.count() == 1, true );
				CHECK( t.data[0] > 0, true );
				break;
			case 0x04:
				CHECK( t.length == 2, true );
				CHECK( t.data.count() == 2, true );
				//CHECK( t.data[0] > 0, true );
				break;
			case 0x05:
				CHECK( t.length > 1, true );
				break;
			case 0x06:
				CHECK( t.length > 2, true );
				break;
			case 0xCA:
				CHECK( t.length == 4, true );
				break;
			case 0xD1:
				CHECK( t.length == 2, true );
				break;
			case 0xD2:
				CHECK( t.length == 2, true );
				break;
			case 0xD3:
				CHECK( t.length > 0, true );
				CHECK( t.data.count() == t.length, true );
				break;
			case 0xD5:
				CHECK( t.length == 1, true );
				break;
			default:
				break;
			}
			realCount++;
		}
		CHECK( tlvCount == realCount, true );
	}
}

//kate: indent-mode csands; tab-width 4;


