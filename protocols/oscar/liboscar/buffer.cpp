/***************************************************************************
                          buffer.cpp  -  description
                             -------------------
    begin                : Thu Jun 6 2002

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2004,2005 by Matt Rogers <mattr@kde.org>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "buffer.h"

#include <ctype.h>
#include <kdebug.h>


Buffer::Buffer()
{
	mReadPos=0;
}

Buffer::Buffer( const Buffer& other )
{
	mBuffer =  other.mBuffer;
	mReadPos = other.mReadPos;
	mBlockStack =  other.mBlockStack;
}

Buffer::Buffer(const char *b, int len)
{
	mBuffer = QByteArray::fromRawData( b, len );
	mReadPos = 0;
}

Buffer::Buffer( const QByteArray& data )
{
	mBuffer = data;
	mReadPos = 0;
}


Buffer::~Buffer()
{
}


int Buffer::addByte(Oscar::BYTE b)
{
	expandBuffer(1);
	mBuffer[mBuffer.size()-1] = b;

	return mBuffer.size();
}

int Buffer::addLEByte(Oscar::BYTE b)
{
	expandBuffer(1);
	mBuffer[mBuffer.size()-1] = ((b) & 0xff);

	return mBuffer.size();
}


int Buffer::addWord(Oscar::WORD w)
{
	expandBuffer(2);
	mBuffer[mBuffer.size()-2] = ((w & 0xff00) >> 8);
	mBuffer[mBuffer.size()-1] = (w & 0x00ff);

	return mBuffer.size();
}

int Buffer::addLEWord(Oscar::WORD w)
{
	expandBuffer(2);
	mBuffer[mBuffer.size()-2] = (unsigned char) ((w >> 0) & 0xff);
	mBuffer[mBuffer.size()-1] = (unsigned char) ((w >> 8) & 0xff);

	return mBuffer.size();
}


int Buffer::addDWord(Oscar::DWORD dw)
{
	expandBuffer(4);
	mBuffer[mBuffer.size()-4] = (dw & 0xff000000) >> 24;
	mBuffer[mBuffer.size()-3] = (dw & 0x00ff0000) >> 16;
	mBuffer[mBuffer.size()-2] = (dw & 0x0000ff00) >> 8;
	mBuffer[mBuffer.size()-1] = (dw & 0x000000ff);

	return mBuffer.size();
}

int Buffer::addLEDWord(Oscar::DWORD dw)
{
	expandBuffer(4);
	mBuffer[mBuffer.size()-4] = (unsigned char) ((dw >> 0) & 0xff);
	mBuffer[mBuffer.size()-3] = (unsigned char) ((dw >>  8) & 0xff);
	mBuffer[mBuffer.size()-2] = (unsigned char) ((dw >> 16) & 0xff);
	mBuffer[mBuffer.size()-1] = (unsigned char) ((dw >> 24) & 0xff);

	return mBuffer.size();
}

int Buffer::addString( const QByteArray& s )
{
	mBuffer.append( s );
	return mBuffer.size();
}

int Buffer::addString(QByteArray s, Oscar::DWORD len)
{
	Q_UNUSED( len );
	return addString( s );
}

int Buffer::addString( const char* s, Oscar::DWORD len )
{
	QByteArray qba( s, len );
	return addString( qba );
}

int Buffer::addString(const unsigned char* s, Oscar::DWORD len)
{
	QByteArray qba( (const char*) s, len );
	return addString( qba );
}

int Buffer::addLEString(const char *s, Oscar::DWORD len)
{
	unsigned int pos = mBuffer.size();
	expandBuffer(len);
	//concatenate the new string onto the buffer
	for(unsigned int i=0; i<len; i++)
	{
		mBuffer[pos+i]=((s[i]) & 0xff);
	}
	return mBuffer.size();
}


void Buffer::clear()
{
	mBuffer.truncate( 0 );
	mReadPos=0;
}

int Buffer::addTLV( const TLV& t )
{
	return addTLV( t.type, t.data );
}

int Buffer::addTLV( Oscar::WORD type, const QByteArray& data )
{
	addWord( type );
	addWord( data.length() );
	return addString( data );
}

int Buffer::addLETLV( Oscar::WORD type, const QByteArray& data )
{
	addLEWord( type );
	addLEWord( data.length() );
	return addString( data );
}

Oscar::BYTE Buffer::getByte()
{
	Oscar::BYTE thebyte = 0x00;

	if(mReadPos < mBuffer.size())
	{
		thebyte = mBuffer[mReadPos];
		mReadPos++;
	}
	else
		kDebug(14150) << "Buffer::getByte(): mBuffer empty";

	return thebyte;
}

void Buffer::skipBytes( int bytesToSkip )
{
	if (mReadPos < mBuffer.size())
		mReadPos += bytesToSkip;
}

Oscar::BYTE Buffer::getLEByte()
{
	Oscar::BYTE b = getByte();
	return (b & 0xff);
}

Oscar::WORD Buffer::getWord()
{
	Oscar::WORD theword, theword2, retword;
	theword = getByte();
	theword2 = getByte();
	retword = (theword << 8) | theword2;
	return retword;
}

Oscar::WORD Buffer::getLEWord()
{
	Oscar::WORD theword1, theword2, retword;
	theword1 = getLEByte();
	theword2 = getLEByte();
	retword = (theword2 << 8) | theword1;
	return retword;
}

Oscar::DWORD Buffer::getDWord()
{
	Oscar::DWORD word1, word2;
	Oscar::DWORD retdword;
	word1 = getWord();
	word2 = getWord();
	retdword = (word1 << 16) | word2;
	return retdword;
}

Oscar::DWORD Buffer::getLEDWord()
{
	Oscar::DWORD word1, word2, retdword;
	word1 = getLEWord();
	word2 = getLEWord();
	retdword = (word2 << 16) | word1;
	return retdword;
}

void Buffer::setBuf(char *b, Oscar::WORD len)
{
	mBuffer = QByteArray::fromRawData(b, len);
	mReadPos = 0;
}

QByteArray Buffer::getBlock(Oscar::DWORD len)
{
	if ( len > (Oscar::DWORD)(mBuffer.size() - mReadPos) )
	{
		kDebug(14150) << "Buffer::getBlock(DWORD): mBuffer underflow!!!";
		len = mBuffer.size() - mReadPos;
	}

	QByteArray ch;
	ch.resize( len );

	for ( Oscar::DWORD i = 0; i < len; i++ )
	{
		ch[i] = getByte();
	}

	return ch;
}

QByteArray Buffer::getBBlock(Oscar::WORD len)
{
	QByteArray data = QByteArray::fromRawData( mBuffer.data() + mReadPos, len);
	mReadPos += len;
	return data;
}


Oscar::WORD *Buffer::getWordBlock(Oscar::WORD len)
{
	kDebug(14150) << "of length " << len;
	Oscar::WORD *ch=new Oscar::WORD[len+1];
	for (unsigned int i=0; i<len; i++)
	{
		ch[i]=getWord();
	}
	ch[len]=0;
	return ch;
}


QByteArray Buffer::getLEBlock(Oscar::WORD len)
{
	QByteArray ch;
	for (unsigned int i=0;i<len;i++)
		ch += getLEByte();

	return ch;
}

int Buffer::addTLV32(Oscar::WORD type, Oscar::DWORD data)
{
	addWord(type);
	addWord(0x0004); //4 Oscar::BYTEs long
	return addDWord(data);
}

int Buffer::addLETLV32(Oscar::WORD type, Oscar::DWORD data)
{
	addLEWord(type);
	addLEWord(0x0004); //4 Oscar::BYTEs long
	return addLEDWord(data);
}

int Buffer::addTLV16(Oscar::WORD type, Oscar::WORD data)
{
	addWord(type);
	addWord(0x0002); //2 Oscar::BYTEs long
	return addWord(data);
}

int Buffer::addLETLV16(Oscar::WORD type, Oscar::WORD data)
{
	addLEWord(type);
	addLEWord(0x0002); //2 Oscar::BYTEs long
	return addLEWord(data);
}

int Buffer::addTLV8(Oscar::WORD type, Oscar::BYTE data)
{
	addWord(type);
	addWord(0x0001); //1 Oscar::BYTE long
	return addByte(data);
}

int Buffer::addLETLV8(Oscar::WORD type, Oscar::BYTE data)
{
	addLEWord(type);
	addLEWord(0x0001); //1 Oscar::BYTE long
	return addLEByte(data);
}

TLV Buffer::getTLV()
{
	TLV t;
	if(bytesAvailable() >= 4)
	{
		t.type = getWord();
		t.length = getWord();
		if ( t )
			t.data = getBlock( t.length );
		/*else
			kDebug(OSCAR_RAW_DEBUG) << "Invalid TLV in buffer";*/
	}

	//kDebug(OSCAR_RAW_DEBUG) << "TLV data is " << t.data;
	return t;
}

QList<TLV> Buffer::getTLVList()
{
	QList<TLV> ql;

	while (mReadPos < mBuffer.size())
	{
		TLV t;

		t = getTLV();
		if ( !t )
		{
			kDebug(14150) << "Invalid TLV found";
			continue;
		}

		//kDebug(14150) << "got TLV(" << t.type << ")";
		ql.append(t);
	}

	return ql;
}

int Buffer::addChatTLV(Oscar::WORD type, Oscar::WORD exchange,
	const QString &roomname, Oscar::WORD instance)
{
	addWord(type);
	addWord(0x0005 + roomname.length());
	addWord(exchange);
	addByte(roomname.length());
	addString(roomname.toLatin1()); // TODO: check encoding

	return addWord(instance);
}

void Buffer::expandBuffer(unsigned int inc)
{
	mBuffer.resize(mBuffer.size()+inc);
}

QByteArray Buffer::getLNTS()
{
	Oscar::WORD len = getWord();
	QByteArray qcs;
	if ( len > 0 )
	{
		qcs = getBlock(len - 1);
		skipBytes( 1 );
	}

	return qcs;
}

QByteArray Buffer::getLELNTS()
{
	Oscar::WORD len = getLEWord();
	QByteArray qcs;
	if ( len > 0 )
	{
		qcs = getBlock(len - 1);
		skipBytes( 1 );
	}

	return qcs;
}

int Buffer::addLNTS(const char *s)
{
	unsigned int len = strlen(s);

	addLEWord(len+1);
	if(len > 0)
		addString(s, len);
	int ret = addByte(0x00);
	return ret;
}

int Buffer::addLELNTS(const char *s)
{
	unsigned int len = strlen(s);
	int ret = addLEWord(len+1);
	if(len > 0)
		ret = addLEString(s, len);
	ret = addByte(0x00);
	return ret;
}

int Buffer::addBSTR(const char * s)
{
	unsigned int len = strlen(s);
	int ret = addWord(len);
	if(len > 0)
		ret = addString(s, len);
	return ret;
}

QByteArray Buffer::getBSTR()
{
	Oscar::WORD len = getWord();
	QByteArray qba( getBlock(len) );
	return qba;
}

int Buffer::addBUIN(const char * s)
{
	unsigned int len = strlen(s);
	int ret = addByte(len);
	ret = addString(s, len);
	return ret;
}

QByteArray Buffer::getBUIN()
{
	Oscar::BYTE len = getByte();
	QByteArray qba( getBlock(len) );
	return qba;
}

QByteArray Buffer::buffer() const
{
	return mBuffer;
}

int Buffer::length() const
{
	return mBuffer.size();
}

int Buffer::bytesAvailable() const
{
	return (mBuffer.size() - mReadPos);
}



QString Buffer::toString() const
{
	// line format:
	//00 03 00 0b 00 00 90 b8 f5 9f 09 31 31 33 37 38   |;tJ�..........|

	int i = 0;
	QString output = "\n";
	QString hex, ascii;

	QByteArray::ConstIterator it;
	for ( it = mBuffer.begin(); it != mBuffer.end(); ++it )
	{
		i++;

		unsigned char c = static_cast<unsigned char>(*it);

		if ( c < 0x10 )
			hex.append("0");
		hex.append(QString("%1 ").arg(c, 0, 16));

		ascii.append(isprint(c) ? c : '.');

		if (i == 16)
		{
			output += hex + "   |" + ascii + "|\n";
			i=0;
			hex.clear();
			ascii.clear();
		}
	}

	if(!hex.isEmpty())
		output += hex.leftJustified(48, ' ') + "   |" + ascii.leftJustified(16, ' ') + '|';
	output.append('\n');

	return output;
}

QString Buffer::peekBSTR()
{
	int lastPos = mReadPos;
	QByteArray qba = getBSTR();
	mReadPos = lastPos;
	return QString( qba );
}
QString Buffer::peekBUIN()
{
	int lastPos = mReadPos;
	QByteArray qba = getBUIN();
	mReadPos = lastPos;
	return QString( qba );
}


int Buffer::addGuid( const Guid & g )
{
	if (g.isValid())
		return addString( g.data() );
	return mBuffer.size();
}


Guid Buffer::getGuid()
{
	return Guid(getBBlock(16)); //block or bblock?
}

int Buffer::addLEBlock( const QByteArray& block )
{
	int ret = addLEWord( block.length() );
	if ( block.length() > 0 )
		ret = addString( block );
	
	return ret;
}

QByteArray Buffer::getLEBlock()
{
	Oscar::DWORD len = getLEWord();
	return getBlock( len );
}

int Buffer::addLEDBlock( const QByteArray& block )
{
	int ret = addLEDWord( block.length() );
	if ( block.length() > 0 )
		ret = addString( block );

	return ret;
}

QByteArray Buffer::getLEDBlock()
{
	Oscar::DWORD len = getLEDWord();
	return getBlock( len );
}

void Buffer::startBlock( BlockType type, ByteOrder byteOrder )
{
	Block block = { type, byteOrder, mBuffer.size() };
	mBlockStack.push( block );

	if ( type == BWord )
		expandBuffer( 2 );
	else if ( type == BDWord )
		expandBuffer( 4 );
}

void Buffer::endBlock()
{
	Q_ASSERT( mBlockStack.size() > 0 );
	Block block = mBlockStack.pop();

	int size = 0;
	if ( block.type == BWord )
		size = mBuffer.size() - block.pos - 2;
	else if ( block.type == BDWord )
		size = mBuffer.size() - block.pos - 4;

	if ( block.byteOrder == BigEndian )
	{
		if ( block.type == BWord )
		{
			mBuffer[block.pos++] = (unsigned char) ((size & 0x0000ff00) >> 8);
			mBuffer[block.pos++] = (unsigned char) ((size & 0x000000ff) >> 0);
		}
		else if ( block.type == BDWord )
		{
			mBuffer[block.pos++] = (unsigned char) ((size & 0xff000000) >> 24);
			mBuffer[block.pos++] = (unsigned char) ((size & 0x00ff0000) >> 16);
			mBuffer[block.pos++] = (unsigned char) ((size & 0x0000ff00) >> 8);
			mBuffer[block.pos++] = (unsigned char) ((size & 0x000000ff) >> 0);
		}
	}
	else
	{
		if ( block.type == BWord )
		{
			mBuffer[block.pos++] = (unsigned char) ((size >> 0) & 0xff);
			mBuffer[block.pos++] = (unsigned char) ((size >> 8) & 0xff);
		}
		else if ( block.type == BDWord )
		{
			mBuffer[block.pos++] = (unsigned char) ((size >> 0) & 0xff);
			mBuffer[block.pos++] = (unsigned char) ((size >> 8) & 0xff);
			mBuffer[block.pos++] = (unsigned char) ((size >> 16) & 0xff);
			mBuffer[block.pos++] = (unsigned char) ((size >> 24) & 0xff);
		}
	}
}

Buffer::operator QByteArray() const
{
	return mBuffer;
}
//kate: tab-width 4; indent-mode csands;
