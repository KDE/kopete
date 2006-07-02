/***************************************************************************
                          buffer.cpp  -  description
                             -------------------
    begin                : Thu Jun 6 2002

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2004 by Matt Rogers <mattr@kde.org>
    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <kdebug.h>
#include <kapplication.h>
#include "buffer.h"

#include <ctype.h>

Buffer::Buffer()
{
	mReadPos=0;
}

Buffer::Buffer( const Buffer& other )
{
	mBuffer.duplicate( other.mBuffer );
	mReadPos = other.mReadPos;
}

Buffer::Buffer(const char *b, Q_ULONG len)
{
	mBuffer.duplicate(b, len);
	mReadPos=0;
}

Buffer::Buffer( const QByteArray& data )
{
	mBuffer.duplicate( data );
	mReadPos = 0;
}


Buffer::~Buffer()
{
}


int Buffer::addByte(const BYTE b)
{
	expandBuffer(1);
	mBuffer[mBuffer.size()-1] = b;

	return mBuffer.size();
}

int Buffer::addLEByte(const BYTE b)
{
	expandBuffer(1);
	mBuffer[mBuffer.size()-1] = ((b) & 0xff);

	return mBuffer.size();
}


int Buffer::addWord(const WORD w)
{
	expandBuffer(2);
	mBuffer[mBuffer.size()-2] = ((w & 0xff00) >> 8);
	mBuffer[mBuffer.size()-1] = (w & 0x00ff);

	return mBuffer.size();
}

int Buffer::addLEWord(const WORD w)
{
	expandBuffer(2);
	mBuffer[mBuffer.size()-2] = (unsigned char) ((w >> 0) & 0xff);
	mBuffer[mBuffer.size()-1] = (unsigned char) ((w >> 8) & 0xff);

	return mBuffer.size();
}


int Buffer::addDWord(const DWORD dw)
{
	expandBuffer(4);
	mBuffer[mBuffer.size()-4] = (dw & 0xff000000) >> 24;
	mBuffer[mBuffer.size()-3] = (dw & 0x00ff0000) >> 16;
	mBuffer[mBuffer.size()-2] = (dw & 0x0000ff00) >> 8;
	mBuffer[mBuffer.size()-1] = (dw & 0x000000ff);

	return mBuffer.size();
}

int Buffer::addLEDWord(const DWORD dw)
{
	expandBuffer(4);
	mBuffer[mBuffer.size()-4] = (unsigned char) ((dw >> 0) & 0xff);
	mBuffer[mBuffer.size()-3] = (unsigned char) ((dw >>  8) & 0xff);
	mBuffer[mBuffer.size()-2] = (unsigned char) ((dw >> 16) & 0xff);
	mBuffer[mBuffer.size()-1] = (unsigned char) ((dw >> 24) & 0xff);

	return mBuffer.size();
}

int Buffer::addString(QByteArray s)
{
	unsigned int pos = mBuffer.size();
	int len = s.size();
	expandBuffer(len);
	
	for ( int i = 0; i < len; i++ )
		mBuffer[pos + i] = s[i];
	
	return mBuffer.size();
}

int Buffer::addString(QByteArray s, DWORD len)
{
	Q_UNUSED( len );
	return addString( s );
}

int Buffer::addString( const char* s, DWORD len )
{
	QByteArray qba;
	qba.duplicate( s, len );
	return addString( qba );
}

int Buffer::addString(const unsigned char* s, DWORD len)
{
	QByteArray qba;
	qba.duplicate( (const char*) s, len );
	return addString( qba );
}

int Buffer::addLEString(const char *s, const DWORD len)
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
	return addTLV( t.type, t.length, t.data );
}

int Buffer::addTLV(WORD type, WORD len, const char *data)
{

	addWord(type);
	addWord(len);
	return addString(data,len);
}

int Buffer::addLETLV(WORD type, WORD len, const char *data)
{
	addLEWord( type );
	addLEWord( len );
	return addString( data, len );
}

BYTE Buffer::getByte()
{
	BYTE thebyte = 0x00;

	if(mReadPos < mBuffer.size())
	{
		thebyte = mBuffer[mReadPos];
		mReadPos++;
	}
	else
		kdDebug(14150) << "Buffer::getByte(): mBuffer empty" << endl;

	return thebyte;
}

void Buffer::skipBytes( int bytesToSkip )
{
	if (mReadPos < mBuffer.size())
		mReadPos += bytesToSkip;
}

BYTE Buffer::getLEByte()
{
	BYTE b = getByte();
	return (b & 0xff);
}

WORD Buffer::getWord()
{
	WORD theword, theword2, retword;
	theword = getByte();
	theword2 = getByte();
	retword = (theword << 8) | theword2;
	return retword;
}

WORD Buffer::getLEWord()
{
	WORD theword1, theword2, retword;
	theword1 = getLEByte();
	theword2 = getLEByte();
	retword = (theword2 << 8) | theword1;
	return retword;
}

DWORD Buffer::getDWord()
{
	DWORD word1, word2;
	DWORD retdword;
	word1 = getWord();
	word2 = getWord();
	retdword = (word1 << 16) | word2;
	return retdword;
}

DWORD Buffer::getLEDWord()
{
	DWORD word1, word2, retdword;
	word1 = getLEWord();
	word2 = getLEWord();
	retdword = (word2 << 16) | word1;
	return retdword;
}

void Buffer::setBuf(char *b, const WORD len)
{
	kdDebug(14150) << k_funcinfo << "Called." << endl;
	
	mBuffer.duplicate(b, len);
	mReadPos = 0;
}

QByteArray Buffer::getBlock(WORD len)
{
	QByteArray ch( len );
	for ( int i = 0; i < len; i++ )
	{
		ch[i] = getByte();
	}

	return ch;
}

QByteArray Buffer::getBBlock(WORD len)
{
	QByteArray data;
	data.duplicate(mBuffer.data() + mReadPos, len);
	mReadPos += len;
	return data;
}


WORD *Buffer::getWordBlock(WORD len)
{
	kdDebug(14150) << k_funcinfo << "of length " << len << endl;
	WORD *ch=new WORD[len+1];
	for (unsigned int i=0; i<len; i++)
	{
		ch[i]=getWord();
	}
	ch[len]=0;
	return ch;
}


QCString Buffer::getLEBlock(WORD len)
{
	QCString ch;
	for (unsigned int i=0;i<len;i++)
		ch += getLEByte();
	
	return ch;
}

int Buffer::addTLV16(const WORD type, const WORD data)
{
	addWord(type);
	addWord(0x0002); //2 bytes long
	return addWord(data);
}

int Buffer::addLETLV16(const WORD type, const WORD data)
{
	addLEWord(type);
	addLEWord(0x0002); //2 bytes long
	return addLEWord(data);
}

int Buffer::addTLV8(const WORD type, const BYTE data)
{
	addWord(type);
	addWord(0x0001); //1 byte long
	return addByte(data);
}

int Buffer::addLETLV8(const WORD type, const BYTE data)
{
	addLEWord(type);
	addLEWord(0x0001); //1 byte long
	return addLEByte(data);
}

TLV Buffer::getTLV()
{
	TLV t;
	if(length() >= 4)
	{
		t.type = getWord();
		t.length = getWord();
		if ( t )
			t.data = getBlock( t.length );
		/*else
			kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "Invalid TLV in buffer" << endl;*/
	}
	
	//kdDebug(OSCAR_RAW_DEBUG) << k_funcinfo << "TLV data is " << t.data << endl;
	return t;
}

QValueList<TLV> Buffer::getTLVList()
{
	QValueList<TLV> ql;

	while (mReadPos < mBuffer.size())
	{
		TLV t;

		t = getTLV();
		if ( !t )
		{
			kdDebug(14150) << k_funcinfo << "Invalid TLV found" << endl;
			continue;
		}

		//kdDebug(14150) << k_funcinfo << "got TLV(" << t.type << ")" << endl;
		ql.append(t);
	}

	return ql;
}

int Buffer::addChatTLV(const WORD type, const WORD exchange,
	const QString &roomname, const WORD instance)
{
	addWord(type);
	addWord(0x0005 + roomname.length());
	addWord(exchange);
	addByte(roomname.length());
	addString(roomname.latin1(), roomname.length()); // TODO: check encoding

	return addWord(instance);
}

void Buffer::expandBuffer(unsigned int inc)
{
	mBuffer.resize(mBuffer.size()+inc, QGArray::SpeedOptim);
}

QCString Buffer::getLNTS()
{
	WORD len = getLEWord();
	QCString qcs;
	qcs.duplicate( getBlock(len) );
	return qcs;
}

QCString Buffer::getLELNTS()
{
	WORD len = getLEWord();
	QCString qcs;
	qcs.duplicate( getBlock(len) );
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
	WORD len = getWord();
	QByteArray qba;
	qba.duplicate( getBlock(len) );
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
	BYTE len = getByte();
	QByteArray qba;
	qba.duplicate( getBlock(len) );
	return qba;
}

char *Buffer::buffer() const
{
	return mBuffer.data();
}

int Buffer::length() const
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
			hex=QString::null;
			ascii=QString::null;
		}
	}

	if(!hex.isEmpty())
		output += hex.leftJustify(48, ' ') + "   |" + ascii.leftJustify(16, ' ') + '|';
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

Buffer::operator QByteArray() const
{
	return mBuffer;
}
//kate: tab-width 4; indent-mode csands;
