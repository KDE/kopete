/***************************************************************************
                          buffer.cpp  -  description
                             -------------------
    begin                : Thu Jun 6 2002

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
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
#include <string.h>
#include "buffer.h"

Buffer::Buffer(QObject *parent, const char *name) : QObject(parent, name)
{
	mLength = 0;
	alloc_length = 0;
	alloc_buf = NULL;
	mBuf = NULL;
	connect(this, SIGNAL(bufError(QString)), this, SLOT(OnBufError(QString)));
}

Buffer::Buffer(char *b, Q_ULONG len, QObject *parent, const char *name)
	: QObject(parent,name)
{
	mLength = 0;
	alloc_length = 0;
	alloc_buf = NULL;
	mBuf = NULL;
	connect(this, SIGNAL(bufError(QString)), this, SLOT(OnBufError(QString)));
	setBuf(b, len);
}

Buffer::~Buffer()
{
}

int Buffer::addByte(const BYTE b)
{
	doResize(1);
	mBuf[mLength] = b;
	return ++mLength;
}

int Buffer::addLEByte(const BYTE b)
{
	doResize(1);
	mBuf[mLength] = ((b) & 0xff);
	return ++mLength;
}

int Buffer::addWord(const WORD w)
{
	doResize(2);
	mBuf[mLength] = (w & 0xff00) >> 8;
	mBuf[mLength+1] = (w & 0x00ff);
	mLength += 2;
	return mLength;
}

int Buffer::addDWord(const DWORD dw)
{
	doResize(4);
	mBuf[mLength] = (dw & 0xff000000) >> 24;
	mBuf[mLength+1] = (dw & 0x00ff0000) >> 16;
	mBuf[mLength+2] = (dw & 0x0000ff00) >> 8;
	mBuf[mLength+3] = (dw & 0x000000ff);
	mLength += 4;
	return mLength;
}

int Buffer::addLEWord(const WORD w)
{
	doResize(2);
	mBuf[mLength] = (unsigned char) ((w >> 0) & 0xff);
	mBuf[mLength+1] = (unsigned char) ((w >> 8) & 0xff);
	mLength += 2;
	return mLength;
}

int Buffer::addLEDWord(const DWORD dw)
{
	doResize(4);
	mBuf[mLength] = (unsigned char) ((dw >> 0) & 0xff);
	mBuf[mLength+1] = (unsigned char) ((dw >>  8) & 0xff);
	mBuf[mLength+2] = (unsigned char) ((dw >> 16) & 0xff);
	mBuf[mLength+3] = (unsigned char) ((dw >> 24) & 0xff);

	mLength += 4;
	return mLength;
}

int Buffer::addString(const char * s, const DWORD len)
{
	doResize(len);
	//concatenate the new string onto the buffer
	for (unsigned int i=0;i<len;i++)
		mBuf[mLength+i] = s[i];
	mLength += len;
	return mLength;
}

int Buffer::addString(const unsigned char *s, const DWORD len)
{
	return addString((const char*)s, len);
}

int Buffer::addLEString(const char * s, const DWORD len)
{
	doResize(len);
	//concatenate the new string onto the buffer
	for (unsigned int i=0;i<len;i++)
		mBuf[mLength+i] = ( (s[i]) & 0xff);
	mLength += len;
	return mLength;
}

void Buffer::clear()
{
	delete [] alloc_buf;
	mBuf = 0L;
	mLength = 0;
	alloc_length = 0;
}

/** Adds a TLV with the given type and data */
int Buffer::addTLV(WORD type, WORD len, const char *data)
{
	addWord(type);
	addWord(len);
	return addString(data,len);
}

int Buffer::addFlap(const BYTE channel, const WORD flapSequenceNum)
{
	doResize(6);
	//create the flap header
	for (int i=mLength-1;i>=0;i--) //copy over the packet
		mBuf[i+6] = mBuf[i];
	mBuf[0] = 0x2a;
	mBuf[1] = channel;
	mBuf[2] = (flapSequenceNum & 0xff00) >> 8;
	mBuf[3] = (flapSequenceNum & 0x00ff);
	mBuf[4] = (mLength & 0xff00) >> 8;
	mBuf[5] = (mLength & 0x00ff);
	mLength += 6;
//	flapSequenceNum++;
	return mLength;
}

void Buffer::print() const
{
	kdDebug(14150) << toString() << endl;
}

QString Buffer::toString() const
{
	QString output;
	for (unsigned int i=0;i<mLength;i++)
	{
		if (static_cast<unsigned char>(mBuf[i]) < 0x10)
			output += "0";
		output += QString("%1 ").arg(static_cast<unsigned char>(mBuf[i]),0,16);
		if ((i>0) && (i % 10 == 0))
			output += '\n';
	}
	return output;
}

int Buffer::addSnac(const WORD family, const WORD subtype,
	const WORD flags, const DWORD id)
{
	addWord(family);
	addWord(subtype);
	addWord(flags);
	return addDWord(id);
}

SNAC Buffer::getSnacHeader()
{
	SNAC s;
	s.family = getWord();
	s.subtype = getWord();
	s.flags = getWord();
	s.id = getDWord();
	return s;
}

BYTE Buffer::getByte()
{
	BYTE thebyte = 0x00;
	if(mLength > 0)
	{
		thebyte = mBuf[0];
		mBuf++; //advance mBuf to the next char
		mLength--;
	}
	else
	{
		emit bufError("Buffer::getByte(): mBuf empty");
	}
	return thebyte;
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

void Buffer::OnBufError(QString s)
{
	kdDebug(14150) <<
		" ============= " << endl <<
		" BUFFER ERROR: " << s << endl <<
		" ============= " << endl;
}

void Buffer::setBuf(char *b, const WORD l)
{
	if (alloc_buf)
		delete [] alloc_buf;
	alloc_buf = b;
	mBuf = b;
	mLength = l;
	alloc_length = l;
}

char *Buffer::getBlock(WORD len)
{
	char *ch = new char[len+1];
	for (unsigned int i=0;i<len;i++)
	{
		ch[i] = getByte();
	}
	ch[len] = 0;
	return ch;
}

char *Buffer::getLEBlock(WORD len)
{
	char *ch = new char[len+1];
	for (unsigned int i=0;i<len;i++)
	{
		ch[i] = getLEByte();
	}
	ch[len] = 0;
	return ch;
}

int Buffer::addTLV16(const WORD type, const WORD data)
{
	addWord(type);
	addWord(0x0002); //2 bytes long
	return addWord(data);
}

int Buffer::addTLV8(const WORD type, const BYTE data)
{
	addWord(type);
	addWord(0x0001); //1 byte long
	return addByte(data);
}

TLV Buffer::getTLV()
{
	TLV t;
	t.type = getWord();
	t.length = getWord();
	t.data = getBlock(t.length);
	return t;
}

QPtrList<TLV> Buffer::getTLVList()
{
	QPtrList<TLV> ql;
	ql.setAutoDelete(FALSE);
	while (mLength != 0)
	{
		TLV *t = new TLV;
		*t = getTLV();
		ql.append(t);
	}
	return ql;
}

int Buffer::appendFlap(const BYTE chan, const WORD len, const WORD flapSequenceNum)
{
	doResize(6);
	mBuf[mLength] = 0x2a;
	mBuf[mLength+1] = chan;
	mBuf[mLength+2] = (flapSequenceNum & 0xff00) >> 8;
	mBuf[mLength+3] = (flapSequenceNum & 0x00ff);
	mBuf[mLength+4] = (len & 0xff00) >> 8;
	mBuf[mLength+5] = (len & 0x00ff);
	mLength += 6;
//	flapSequenceNum++;
	return mLength;
}

int Buffer::addChatTLV(const WORD type, const WORD exchange,
	const QString &roomname, const WORD instance)
{
	addWord(type);
	addWord(0x0005+roomname.length());
	addWord(exchange);
	addByte(roomname.length());
	addString(roomname.latin1(),roomname.length());
	return addWord(instance);
}

void Buffer::doResize(int inc)
{
	if(static_cast<DWORD>(mLength + inc + mBuf - alloc_buf) > alloc_length)
		{//if we need a new array
		// FIXME: do what this comment says!
		// don't worry, I'll be changing this to a QByteArray pretty soon
		// in the meantime:
		// before allocating memory, check to see if we can use what is already discarded
		// if more than half the mBuffer has been discarded, we'll just relocate what we have
		if(static_cast<DWORD>(mBuf-alloc_buf) > (mLength+inc))
		{
			for (DWORD i=0; i < mLength; i++)
				alloc_buf[i] = mBuf[i];
			mBuf = alloc_buf;
		}
		else
		{
			char *tmp;
			tmp = new char[(mLength + inc)*2];

			for (DWORD i=0;i<mLength;i++)
				tmp[i] = mBuf[i];

			if (alloc_buf)
				delete[] alloc_buf;

			alloc_buf = tmp;
			mBuf = alloc_buf;
			alloc_length = (mLength + inc)*2;
		}
	}
}

char *Buffer::getLNTS()
{
	WORD len = getWord();
	return getBlock(len);
}

char *Buffer::getLELNTS()
{
	WORD len = getLEWord();
	return getLEBlock(len);
}

int Buffer::addLNTS(const char * s)
{
	unsigned int len = strlen(s);
	int ret = addWord(len);
	if(len > 0)
		ret = addString(s, len);
	return ret;
}

int Buffer::addLELNTS(const char * s)
{
	unsigned int len = strlen(s);
	addLEWord(len);
	return addLEString(s, len);
}

#include "buffer.moc"
// vim: set noet ts=4 sts=4 sw=4:
