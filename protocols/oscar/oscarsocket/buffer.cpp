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
#include <kapplication.h>
#include "buffer.h"

#include <ctype.h>

Buffer::Buffer()
{
	mExtDataPointer=0L;
	mExtDataLen=0;

	mReadPos=0;
	//connect(this, SIGNAL(bufError(QString)), this, SLOT(slotBufferError(QString)));
}

Buffer::Buffer(char *b, Q_ULONG len)
{
#ifdef BUFFER_DEBUG
	kdDebug(14151) << k_funcinfo << "Creating prefilled Buffer" << endl;
#endif

	mExtDataPointer=b;
	mExtDataLen=len;
	mBuffer.setRawData(mExtDataPointer, mExtDataLen);

	mReadPos=0;
	//connect(this, SIGNAL(bufError(QString)), this, SLOT(slotBufferError(QString)));
}

Buffer::~Buffer()
{
#ifdef BUFFER_DEBUG
	kdDebug(14151) << k_funcinfo << "Called." << endl;
#endif

	if(mExtDataPointer)
	{
#ifdef BUFFER_DEBUG
		kdDebug(14151) << k_funcinfo << "Deleting prefilled Buffer without deleting contents" << endl;
		kdDebug(14151) << k_funcinfo << "mExtDataPointer=" << mExtDataPointer <<  endl;
#endif
		mBuffer.resetRawData(mExtDataPointer, mExtDataLen);
	}
	mExtDataPointer=0L;
	mExtDataLen=0;
}


int Buffer::addByte(const BYTE b)
{
	expandBuffer(1);
//	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() << endl;
	mBuffer[mBuffer.size()-1] = b;

	return mBuffer.size();
}

int Buffer::addLEByte(const BYTE b)
{
	expandBuffer(1);
//	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() << endl;
	mBuffer[mBuffer.size()-1] = ((b) & 0xff);

	return mBuffer.size();
}


int Buffer::addWord(const WORD w)
{
	expandBuffer(2);
//	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() << endl;
	mBuffer[mBuffer.size()-2] = ((w & 0xff00) >> 8);
	mBuffer[mBuffer.size()-1] = (w & 0x00ff);

	return mBuffer.size();
}

int Buffer::addLEWord(const WORD w)
{
	expandBuffer(2);
//	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() << endl;
	mBuffer[mBuffer.size()-2] = (unsigned char) ((w >> 0) & 0xff);
	mBuffer[mBuffer.size()-1] = (unsigned char) ((w >> 8) & 0xff);

	return mBuffer.size();
}


int Buffer::addDWord(const DWORD dw)
{
	expandBuffer(4);
//	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() << endl;
	mBuffer[mBuffer.size()-4] = (dw & 0xff000000) >> 24;
	mBuffer[mBuffer.size()-3] = (dw & 0x00ff0000) >> 16;
	mBuffer[mBuffer.size()-2] = (dw & 0x0000ff00) >> 8;
	mBuffer[mBuffer.size()-1] = (dw & 0x000000ff);

	return mBuffer.size();
}

int Buffer::addLEDWord(const DWORD dw)
{
	expandBuffer(4);
//	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() << endl;
	mBuffer[mBuffer.size()-4] = (unsigned char) ((dw >> 0) & 0xff);
	mBuffer[mBuffer.size()-3] = (unsigned char) ((dw >>  8) & 0xff);
	mBuffer[mBuffer.size()-2] = (unsigned char) ((dw >> 16) & 0xff);
	mBuffer[mBuffer.size()-1] = (unsigned char) ((dw >> 24) & 0xff);

	return mBuffer.size();
}


int Buffer::addString(const char *s, const DWORD len)
{
	unsigned int pos = mBuffer.size();
	expandBuffer(len);
	//concatenate the new string onto the buffer
	for(unsigned int i=0; i<len; i++)
	{
		mBuffer[pos+i]=s[i];
	}
	return mBuffer.size();
}

int Buffer::addString(const unsigned char *s, const DWORD len)
{
	return addString((const char*)s, len);
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
#ifdef BUFFER_DEBUG
	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() << endl;
#endif

	if(mExtDataPointer)
	{
#ifdef BUFFER_DEBUG
		kdDebug(14151) << k_funcinfo << "Clearing Buffer without deleting contents" << endl;
#endif
		mBuffer.resetRawData(mExtDataPointer, mExtDataLen);
		mExtDataPointer=0L;
		mExtDataLen=0;
	}
	else
	{
		mBuffer.resize(0);
	}
	mReadPos=0;
}


int Buffer::addTLV(WORD type, WORD len, const char *data)
{
//	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() << endl;
	addWord(type);
	addWord(len);
	return addString(data,len);
}

int Buffer::addFlap(const BYTE channel, const WORD flapSequenceNum)
{
	unsigned int sizeWithoutHeader = mBuffer.size();
	expandBuffer(6);

/*	kdDebug(14151) << k_funcinfo << "buffer size=" << mBuffer.size() <<
		", size without header = " << sizeWithoutHeader << endl;*/

	//create the flap header
	for (int i=sizeWithoutHeader-1; i>=0; i--) //copy over the packet
	{
		mBuffer[i+6] = mBuffer[i];
	}

	mBuffer[0] = 0x2a;
	mBuffer[1] = channel;
	mBuffer[2] = ((flapSequenceNum & 0xff00) >> 8);
	mBuffer[3] = (flapSequenceNum & 0x00ff);
	mBuffer[4] = ((sizeWithoutHeader & 0xff00) >> 8);
	mBuffer[5] = (sizeWithoutHeader & 0x00ff);

	return mBuffer.size();
}

int Buffer::changeSeqNum(const WORD flapSequenceNum)
{
	mBuffer[2] = ((flapSequenceNum & 0xff00) >> 8);
	mBuffer[3] = (flapSequenceNum & 0x00ff);
	return mBuffer.size();
}

DWORD Buffer::addSnac(const WORD family, const WORD subtype,
	const WORD flags, DWORD id)
{
#ifdef BUFFER_DEBUG
	kdDebug(14151) << k_funcinfo <<
		family << ", " << subtype << ", " << flags << ", " << id << endl;
#endif
	addWord(family);
	addWord(subtype);
	addWord(flags);
	if (id==0)
		id = KApplication::random();
	addDWord(id);
	return (id);
}

SNAC Buffer::getSnacHeader()
{
	SNAC s;
	s.family = getWord();
	s.subtype = getWord();
	s.flags = getWord();
	s.id = getDWord();
	s.error = false;
	return s;
}

SNAC Buffer::readSnacHeader()
{
	SNAC s;
	//Read the snac if the buffer contains one
	if ( mBuffer.size() >= 16 )
	{
		s.family = (mBuffer[6] << 8) | mBuffer[7];
		s.subtype = (mBuffer[8] << 8) | mBuffer[9];
		s.flags = (mBuffer[10] << 8) | mBuffer[11];
		s.id = (mBuffer[12] << 24) | (mBuffer[13] << 16) | (mBuffer[14] << 8) | mBuffer[15];
		s.error = false;
	}
	//Otherwise raise an error
	else
	{
		s.error = true;
	}

	return s;
}

BYTE Buffer::getByte()
{
	BYTE thebyte = 0x00;

	if(mReadPos < mBuffer.size())
	{
//		kdDebug(14151) << k_funcinfo << "read pos = " << mReadPos << endl;
		thebyte = mBuffer[mReadPos];
		mReadPos++;
	}
	else
	{
		//emit bufError("Buffer::getByte(): mBuffer empty");
		kdDebug(14151) << "Buffer::getByte(): mBuffer empty" << endl;
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
/*
void Buffer::slotBufferError(QString s)
{
	kdDebug(14151) << " BUFFER ERROR: " << s << endl << "Stopping reporting errors" << endl;

	disconnect(this, SIGNAL(bufError(QString)), this, SLOT(slotBufferError(QString)));
}
*/
void Buffer::setBuf(char *b, const WORD len)
{
#ifdef BUFFER_DEBUG
	kdDebug(14151) << k_funcinfo << "Called." << endl;
#endif

	if(mExtDataPointer)
	{
#ifdef BUFFER_DEBUG
		kdDebug(14151) << k_funcinfo << "Deleting prefilled Buffer without deleting contents" << endl;
#endif
		mBuffer.resetRawData(mExtDataPointer, mExtDataLen);
	}
	else
	{
		mBuffer.resize(0);
	}

	mExtDataPointer = b;
	mExtDataLen = len;
	mBuffer.setRawData(mExtDataPointer, mExtDataLen);

	mReadPos = 0;
}

char *Buffer::getBlock(WORD len)
{
	char *ch=new char[len+1];
	for (unsigned int i=0; i<len; i++)
	{
		ch[i] = getByte();
	}
	ch[len]=0;
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
	kdDebug(14151) << k_funcinfo << "of length " << len << endl;
	WORD *ch=new WORD[len+1];
	for (unsigned int i=0; i<len; i++)
	{
		ch[i]=getWord();
	}
	ch[len]=0;
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
	if(length() >= 4)
	{
		t.type = getWord();
		t.length = getWord();
		t.data = getBlock(t.length);
	}
	else
	{
		t.type = 0;
		t.length = 0;
		t.data = 0L;
	}
	return t;
}

QPtrList<TLV> Buffer::getTLVList(bool debug)
{
	QPtrList<TLV> ql;
	ql.setAutoDelete(FALSE);

	while (mReadPos < mBuffer.size())
	{
		TLV *t = new TLV;

		*t = getTLV();
		if (!t)
			kdDebug(14151) << k_funcinfo << "got no TLV but NULL pointer!" << endl;

		if(debug)
			kdDebug(14151) << k_funcinfo << "got TLV(" << t->type << ")" << endl;

		ql.append(t);
	}

	return ql;
}

int Buffer::appendFlap(const BYTE chan, const WORD len, const WORD flapSequenceNum)
{
	unsigned int currentSize = mBuffer.size();
	expandBuffer(6);

	mBuffer[currentSize] = 0x2a;
	mBuffer[currentSize+1] = chan;
	mBuffer[currentSize+2] = (flapSequenceNum & 0xff00) >> 8;
	mBuffer[currentSize+3] = (flapSequenceNum & 0x00ff);
	mBuffer[currentSize+4] = (len & 0xff00) >> 8;
	mBuffer[currentSize+5] = (len & 0x00ff);

	return mBuffer.size();
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
/*	kdDebug(14151) << k_funcinfo << "Resizing from '" << mBuffer.size() << "' to " <<
		(mBuffer.size()+inc) << "' bytes" << endl;*/
	mBuffer.resize(mBuffer.size()+inc, QGArray::SpeedOptim);
}

char *Buffer::getLNTS()
{
	WORD len = getLEWord();
	return getBlock(len);
}

char *Buffer::getLELNTS()
{
	WORD len = getLEWord();
	return getLEBlock(len);
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

char *Buffer::getBSTR()
{
	WORD len = getWord();
	return getBlock(len);
}

int Buffer::addBUIN(const char * s)
{
	unsigned int len = strlen(s);
	int ret = addByte(len);
	ret = addString(s, len);
	return ret;
}

char *Buffer::getBUIN()
{
	BYTE len = getByte();
	return getBlock(len);
}

char *Buffer::buffer() const
{
	return mBuffer.data();
}

int Buffer::length() const
{
	return (mBuffer.size() - mReadPos);
}

/*void Buffer::print() const
{
	kdDebug(14151) << toString() << endl;
}*/

QString Buffer::toString() const
{
	// line format:
	//00 03 00 0b 00 00 90 b8 f5 9f 09 31 31 33 37 38   |¶;tJÛ...........|

	int i = 0;
	QString output = "\n";
	QString hex, ascii;

	QByteArray::ConstIterator it;
    for(it = mBuffer.begin(); it != mBuffer.end(); ++it)
	{
		i++;

		unsigned char c = static_cast<unsigned char>(*it);

		if(c < 0x10)
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

//#include "buffer.moc"
// vim: set noet ts=4 sts=4 sw=4:
