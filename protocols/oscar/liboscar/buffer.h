/*
 * OSCAR Buffer
 * Copyright (c) 2002 by Tom Linksy <tw16@po.cwru.edu>
 * Copyright (c) 2003-2005 by Matt Rogers <mattr@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <QByteArray>
#include <QList>
#include "oscartypes.h"


class QString;

using namespace Oscar;

/**
 * @brief A data buffer
 */
class Buffer
{
public:
	/** Default constructor */
	Buffer();
	Buffer( const Buffer& other );

	/**
	 * \brief Create a prefilled buffer
	 *
	 * Constructor that creates a prefilled buffer of @p len length
	 * that contains the data from @p b.
	 */
	Buffer(const char *b, ulong len);

	/**
	 * \brief Create a prefilled buffer
	 *
	 * Constructor that creates a prefilled buffer from the QByteArray \p data
	 */
	Buffer( const QByteArray& data );
	

	/** Default destructor */
	~Buffer();

	/** convert to qbytearray */
	operator QByteArray() const;

	/**
	 * returns the raw buffer
	 */
	const char *buffer() const;

	/**
	 * Returns the remaining length of the buffer past the current read
	 * position.
	 */
	int length() const;

	/**
	 * adds the given string to the buffer (make sure it's NULL-terminated)
	 */
	int addString(QByteArray, DWORD);
	int addString(const char*, DWORD);

	/**
	 * Little-endian version of addString
	 */
	int addLEString(const char *, const DWORD);

	/**
	 * adds the given string to the buffer with the length in front of it
	 * (make sure it's NULL-terminated)
	 */
	int addLNTS(const char * s);
	/**
	 * Little-endian version of addLNTS
	 */
	int addLELNTS(const char * s);

	/**
	 * adds the given DWord to the buffer
	 */
	int addDWord(const DWORD);

	/**
	 * adds the given word to the buffer
	 */
	int addWord(const WORD);

	/**
	 * adds the given word to the buffer in
	 * little-endian format as needed by old icq server
	 */
	int addLEWord(const WORD w);

	/**
	 * adds the given DWord to the buffer in
	 * little-endian format as needed by old icq server
	 */
	int addLEDWord(const DWORD dw);

	/**
	 * adds the given byte to the buffer
	 */
	int addByte(const BYTE);
	int addLEByte(const BYTE);

	/**
	 * empties the current buffer.
	 */
	void clear();

	/**
	 * Adds a TLV to the buffer
	 */
	int addTLV( const TLV& t );
	
	/**
	 * Adds a TLV with the given type and data
	 */
	int addTLV(WORD, WORD, const char *);

	/**
	 * Returns a QString representation of the buffer
	 */
	QString toString() const;

	/**
	 * gets a DWord out of the buffer
	 */
	DWORD getDWord();

	/**
	 * Gets a word out of the buffer
	 */
	WORD getWord();

	/**
	 * Gets a byte out of the buffer
	 * It's not a constant method. It advances the buffer
	 * to the next BYTE after returning one.
	 */
	BYTE getByte();
	
	/**
	 * Skip \p bytesToSkip number of bytes in the buffer
	 * Like getByte() the buffer is advanced when skipping
	 */
	void skipBytes( int bytesToSkip );

	/**
	 * Same as above but returns little-endian
	 */
	WORD getLEWord();
	DWORD getLEDWord();
	BYTE getLEByte();

	/**
	 * Set the buffer to the given values. 
	 */
	void setBuf(char *, const WORD);

	/**
	 * Allocates memory for and gets a block of buffer bytes
	 */
	QByteArray getBlock(WORD len);
	QByteArray getBBlock(WORD len);

	/**
	 * Allocates memory for and gets a block of buffer words
	 */
	WORD *getWordBlock(WORD len);

	/**
	 * Same as above but returning little-endian
	 */
	QByteArray getLEBlock(WORD len);

	/**
	 * Convenience function that gets a LNTS (long null terminated string)
	 * from the buffer. Otherwise you'd need a getWord() + getBlock() call :)
	 */
	QByteArray getLNTS();
	QByteArray getLELNTS();

	/**
	 * adds a 16-bit long TLV
	 */
	int addTLV16(const WORD type, const WORD data);

	/**
	 * adds the given byte to a TLV
	 */
	int addTLV8(const WORD type, const BYTE data);

	/**
	 * Gets a TLV, storing it in a struct and returning it
	 */
	TLV getTLV();

	/**
	 * Gets a list of TLV's
	 */
	QList<TLV> getTLVList();

	/**
	 * Creates a chat data segment for a tlv and calls addTLV with that data
	 */
	int addChatTLV(const WORD, const WORD, const QString &, const WORD);

	/**
	 * Similar to the LNTS functions but string is NOT null-terminated
	 */
	int addBSTR(const char * s);
	QByteArray getBSTR();
	QString peekBSTR();

	int addBUIN(const char * s);
	QByteArray getBUIN();
	QString peekBUIN();

private:
	/**
	 * Make the buffer bigger by @p inc bytes
	 */
	void expandBuffer(unsigned int inc);

private:
	QByteArray mBuffer;
	int mReadPos;

};

#endif
// kate: tab-width 4; indent-mode csands;
