/***************************************************************************
                          buffer.cpp  -  description
                             -------------------
    begin                : Thu Jun 6 2002
    copyright            : (C) 2002 by twl6
    email                : twl6@paranoia.STUDENT.CWRU.Edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <string.h>
#include "buffer.h"
#include "buffer.moc"

WORD Buffer::flapSequenceNum = 0x010f;

Buffer::Buffer(QObject *parent, const char *name)
	: QObject(parent,name)
{
	length = 0;
	alloc_length = 0;
	alloc_buf = NULL;
	buf = NULL;
	connect(this, SIGNAL(bufError(QString)), this, SLOT(OnBufError(QString)));
}

Buffer::Buffer(char *b, Q_ULONG len, QObject *parent, const char *name)
	: QObject(parent,name)
{
	length = 0;
	alloc_length = 0;
	alloc_buf = NULL;
	buf = NULL;
	connect(this, SIGNAL(bufError(QString)), this, SLOT(OnBufError(QString)));
	setBuf(b, len);
}

Buffer::~Buffer()
{
}

/** adds the given byte to the buffer */
int Buffer::addByte(const BYTE b)
{
	doResize(1);
	buf[length] = b;
	return ++length;
}

int Buffer::addLEByte(const BYTE b)
{
	doResize(1);
	buf[length] = ((b) & 0xff);
	return ++length;
}

/** adds the given word to the buffer */
int Buffer::addWord(const WORD w)
{
	doResize(2);
	buf[length] = (w & 0xff00) >> 8;
	buf[length+1] = (w & 0x00ff);
	length = length + 2;
	return length;
}

/** adds the given DWord to the buffer */
int Buffer::addDWord(const DWORD dw)
{
	doResize(4);
	buf[length] = (dw & 0xff000000) >> 24;
	buf[length+1] = (dw & 0x00ff0000) >> 16;
	buf[length+2] = (dw & 0x0000ff00) >> 8;
	buf[length+3] = (dw & 0x000000ff);
	length = length + 4;
	return length;
}

/* adds the given word to the buffer in
 * little-endian format as needed by old icq server
 */
int Buffer::addLEWord(const WORD w)
{
	doResize(2);
	buf[length] = (unsigned char) ((w >> 0) & 0xff);
	buf[length+1] = (unsigned char) ((w >> 8) & 0xff);
	length = length + 2;
	return length;
}

/* adds the given DWord to the buffer in
 * little-endian format as needed by old icq server
 */
int Buffer::addLEDWord(const DWORD dw)
{
	doResize(4);
	buf[length] = (unsigned char) ((dw >> 0) & 0xff);
	buf[length+1] = (unsigned char) ((dw >>  8) & 0xff);
	buf[length+2] = (unsigned char) ((dw >> 16) & 0xff);
	buf[length+3] = (unsigned char) ((dw >> 24) & 0xff);

	length = length + 4;
	return length;
}

/** adds the given string to the buffer (make sure it's NULL-terminated) */
int Buffer::addString(const char * s, const DWORD len)
{
	doResize(len);
	for (unsigned int i=0;i<len;i++) //concatenate the new string onto the buffer
		buf[length+i] = s[i];
	length = length + len;
	return length;
}

int Buffer::addLEString(const char * s, const DWORD len)
{
	doResize(len);
	for (unsigned int i=0;i<len;i++) //concatenate the new string onto the buffer
		buf[length+i] = ( (s[i]) & 0xff);
	length = length + len;
	return length;
}

/** deletes the current buffer */
void Buffer::clear()
{
	delete[] alloc_buf;
	buf = NULL;
	length = 0;
	alloc_length = 0;
}

/** Adds a TLV with the given type and data */
int Buffer::addTLV(WORD type, WORD len, const char *data)
{
	addWord(type);
	addWord(len);
	return addString(data,len);
}

/** constructs a flap header from given channel to the beginning of the buffer, returns new buffer length */
int Buffer::addFlap(const BYTE channel)
{
	doResize(6);
	//create the flap header
	for (int i=length-1;i>=0;i--) //copy over the packet
		buf[i+6] = buf[i];
	buf[0] = 0x2a;
	buf[1] = channel;
	buf[2] = (flapSequenceNum & 0xff00) >> 8;
	buf[3] = (flapSequenceNum & 0x00ff);
	buf[4] = (length & 0xff00) >> 8;
	buf[5] = (length & 0x00ff);
	length = length + 6;
	flapSequenceNum++;
	return length;
}

/** Prints out the buffer */
void Buffer::print() const
{
	kdDebug(14150) << toString() << endl;
}

// Returns a QString representation
QString Buffer::toString() const
{
	QString output;
	for (unsigned int i=0;i<length;i++)
	{
		if (static_cast<unsigned char>(buf[i]) < 0x10)
			output += "0";
		output += QString("%1 ").arg(static_cast<unsigned char>(buf[i]),0,16);
	}
	return output;
}

/** Adds a SNAC to the end of the buffer with given family, subtype, flags, and request ID */
int Buffer::addSnac(const WORD family, const WORD subtype, const WORD flags, const DWORD id)
{
	addWord(family);
	addWord(subtype);
	addWord(flags);
	return addDWord(id);
}

/** Gets a snac header out of the buffer */
SNAC Buffer::getSnacHeader()
{
	SNAC s;
	s.family = getWord();
	s.subtype = getWord();
	s.flags = getWord();
	s.id = getDWord();
	return s;
}

/** Gets a byte out of the buffer */
BYTE Buffer::getByte()
{
	BYTE thebyte = 0x00;
	if(length > 0)
	{
		thebyte = buf[0];
		buf++; //advance buf to the next char

		/*for (unsigned int i=1;i<length;i++)
		{  //get rid of first element by shifting the array
			buf[i-1] = buf[i];
		} */

		length--;
	}
	else
		emit bufError("getByte(): buffer empty");
	return thebyte;
}

/** Gets a word out of the buffer */
WORD Buffer::getWord()
{
	WORD theword, theword2, retword;
	theword = getByte();
	theword2 = getByte();
	retword = (theword << 8) | theword2;
	return retword;
}

/** gets a Dword out of the buffer */
DWORD Buffer::getDWord()
{
	DWORD word1, word2;
	DWORD retdword;
	word1 = getWord();
	word2 = getWord();
	retdword = (word1 << 16) | word2;
	return retdword;
}

WORD Buffer::getLEWord()
{
	WORD theword1, theword2, retword;
	theword1 = getLEByte();
	theword2 = getLEByte();
	retword = (theword2 << 8) | theword1;
	return retword;
}

DWORD Buffer::getLEDWord()
{
	DWORD word1, word2, retdword;
	word1 = getLEWord();
	word2 = getLEWord();
	retdword = (word2 << 16) | word1;
	return retdword;
}

BYTE Buffer::getLEByte()
{
	BYTE b = getByte();
	return (b & 0xff);
}

void Buffer::OnBufError(QString s)
{
	kdDebug(14150) <<
		" ============= " << endl <<
		" BUFFER ERROR: " << s << endl <<
		" ============= " << endl;
}

/** sets the buffer and length to the given values */
void Buffer::setBuf(char *b, const WORD l)
{
	if (alloc_buf)
		delete [] alloc_buf;
	alloc_buf = b;
	buf = b;
	length = l;
	alloc_length = l;
}

/** Allocates memory for and gets a block of buffer bytes */
char * Buffer::getBlock(WORD len)
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

/** adds a 16-bit long TLV */
int Buffer::addTLV16(const WORD type, const WORD data)
{
	addWord(type);
	addWord(0x0002);  //2 bytes long
	return addWord(data);
}

/** adds the given byte to a TLV */
int Buffer::addTLV8(const WORD type, const BYTE data)
{
	addWord(type);
	addWord(0x0001); //1 byte long
	return addByte(data);
}

/** Gets a TLV, storing it in a struct and returning it */
TLV Buffer::getTLV()
{
	TLV t;
	t.type = getWord();
	t.length = getWord();
	t.data = getBlock(t.length);
	return t;
}

/** Gets a list of TLV's */
QPtrList<TLV> Buffer::getTLVList()
{
	QPtrList<TLV> ql;
	ql.setAutoDelete(FALSE);
	while (length != 0)
	{
		TLV *t = new TLV;
		*t = getTLV();
		ql.append(t);
	}
	return ql;
}

/** appends a flap header to the end of the buffer w/ given length and channel */
int Buffer::appendFlap(const BYTE chan, const WORD len)
{
	doResize(6);
	buf[length] = 0x2a;
	buf[length+1] = chan;
	buf[length+2] = (flapSequenceNum & 0xff00) >> 8;
	buf[length+3] = (flapSequenceNum & 0x00ff);
	buf[length+4] = (len & 0xff00) >> 8;
	buf[length+5] = (len & 0x00ff);
	length = length + 6;
	flapSequenceNum++;
	return length;
}

/** Creates a chat data segment for a tlv and calls addTLV with that data */
int Buffer::addChatTLV(const WORD type, const WORD exchange, const QString &roomname, const WORD instance)
{
	addWord(type);
	addWord(0x0005+roomname.length());
	addWord(exchange);
	addByte(roomname.length());
	addString(roomname.latin1(),roomname.length());
	return addWord(instance);
}

/** Make the buffer bigger by inc bytes, reallocating memory if needed */
void Buffer::doResize(int inc)
{
	if(static_cast<DWORD>(length+inc+buf-alloc_buf) > alloc_length) //if we need a new array
	{
		// FIXME: do what this comment says!
		// don't worry, I'll be changing this to a QByteArray pretty soon
		// in the meantime:
		// before allocating memory, check to see if we can use what is already discarded
		// if more than half the buffer has been discarded, we'll just relocate what we have
		if(static_cast<DWORD>(buf-alloc_buf) > (length+inc))
		{
			for (DWORD i=0;i<length;i++)
				alloc_buf[i] = buf[i];
			buf = alloc_buf;
		}
		else
		{
			char *tmp;
			tmp = new char[(length + inc)*2];
			for (DWORD i=0;i<length;i++)
				tmp[i] = buf[i];
			if (alloc_buf)
				delete[] alloc_buf;
			alloc_buf = tmp;
			buf = alloc_buf;
			alloc_length = (length + inc)*2;
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

// vim: set noet ts=4 sts=4 sw=4:
