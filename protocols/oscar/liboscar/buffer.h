/***************************************************************************
                          buffer.h  -  description
                             -------------------
    begin                : Thu Jun 6 2002

    Copyright (c) 2002 by Tom Linsky <twl6@po.cwru.edu>
    Copyright (c) 2003-2005 by Matt Rogers <mattr@kde.org>
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

#ifndef BUFFER_H
#define BUFFER_H

#include "oscartypes.h"
#include "oscarguid.h"
#include <QList>
#include <QByteArray>
#include <QStack>
#include "liboscar_export.h"
class QString;

using namespace Oscar;

/**
 * @brief A data buffer
 */
class LIBOSCAR_EXPORT Buffer
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
		Buffer(const char *b, int len);

		/**
		 * \brief Create a prefilled buffer
		 *
		 * Constructor that creates a prefilled buffer from the QByteArray \p data
		 */
		Buffer( const QByteArray& data );


		/** Default destructor */
		~Buffer();

		/**
		 * returns the raw buffer
		 */
		QByteArray buffer() const;

		/**
		 * Returns the length of the buffer. This length is independent of the
		 * current position in the buffer to be read.
		 */
		int length() const;

		/**
		 * Returns the amount of data left in the buffer to read.
		 */
		int bytesAvailable() const;

		/**
		 * adds the given string to the buffer (make sure it's NULL-terminated)
		 */
		int addString( const QByteArray& s );
		int addString(QByteArray, Oscar::DWORD);
		int addString(const char*, Oscar::DWORD);
		int addString(const unsigned char*, Oscar::DWORD);

		/**
		 * Little-endian version of addString
		 */
		int addLEString(const char *, Oscar::DWORD);

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
		int addDWord(Oscar::DWORD);

		/**
		 * adds the given word to the buffer
		 */
		int addWord(Oscar::WORD);

		/**
		 * adds the given word to the buffer in
		 * little-endian format as needed by old icq server
		 */
		int addLEWord(Oscar::WORD w);

		/**
		 * adds the given DWord to the buffer in
		 * little-endian format as needed by old icq server
		 */
		int addLEDWord(Oscar::DWORD dw);

		/**
		 * adds the given byte to the buffer
		 */
		int addByte(Oscar::BYTE);
		int addLEByte(Oscar::BYTE);

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
		int addTLV( Oscar::WORD type, const QByteArray& data );

		/**
		 * Adds a little-endian TLV with the given type and data
		 */
		int addLETLV( Oscar::WORD type, const QByteArray& data );

		/**
		 * Returns a QString representation of the buffer
		 */
		QString toString() const;

		/**
		 * gets a DWord out of the buffer
		 */
		Oscar::DWORD getDWord();

		/**
		 * Gets a word out of the buffer
		 */
		Oscar::WORD getWord();

		/**
		 * Gets a byte out of the buffer
		 * It's not a constant method. It advances the buffer
		 * to the next BYTE after returning one.
		 */
		Oscar::BYTE getByte();

		/**
		 * Skip \p bytesToSkip number of bytes in the buffer
		 * Like getByte() the buffer is advanced when skipping
		 */
		void skipBytes( int bytesToSkip );

		/**
		 * Same as above but returns little-endian
		 */
		Oscar::WORD getLEWord();
		Oscar::DWORD getLEDWord();
		Oscar::BYTE getLEByte();

		/**
		 * Set the buffer to the given values.
		 */
		void setBuf(char *, Oscar::WORD);

		/**
		 * Allocates memory for and gets a block of buffer bytes
		 */
		QByteArray getBlock(Oscar::DWORD len);
		QByteArray getBBlock(Oscar::WORD len);

		/**
		 * Allocates memory for and gets a block of buffer words
		 */
		Oscar::WORD *getWordBlock(Oscar::WORD len);

		/**
		 * Same as above but returning little-endian
		 */
		QByteArray getLEBlock(Oscar::WORD len);

		/**
		 * Convenience function that gets a LNTS (long null terminated string)
		 * from the buffer. Otherwise you'd need a getWord() + getBlock() call :)
		 */
		QByteArray getLNTS();
		QByteArray getLELNTS();

		/**
		 * adds a 32-bit long TLV
		 */
		int addTLV32(Oscar::WORD type, Oscar::DWORD data);

		/**
		 * adds a 32-bit long little-endian TLV
		 */
		int addLETLV32(Oscar::WORD type, Oscar::DWORD data);

		/**
		 * adds a 16-bit long TLV
		 */
		int addTLV16(Oscar::WORD type, Oscar::WORD data);

		/**
		 * adds a 16-bit long little-endian TLV
		 */
		int addLETLV16(Oscar::WORD type, Oscar::WORD data);

		/**
		 * adds the given byte to a TLV
		 */
		int addTLV8(Oscar::WORD type, Oscar::BYTE data);

		/**
		 * adds the given byte to a little-endian TLV
		 */
		int addLETLV8(Oscar::WORD type, Oscar::BYTE data);

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
		int addChatTLV(Oscar::WORD, Oscar::WORD, const QString &, Oscar::WORD);

		/**
		 * Similar to the LNTS functions but string is NOT null-terminated
		 */
		int addBSTR(const char * s);
		QByteArray getBSTR();
		QString peekBSTR();

		int addBUIN(const char * s);
		QByteArray getBUIN();
		QString peekBUIN();

		/** adds guid to the buffer */
		int addGuid( const Guid & g );

		/** gets guid from the buffer */
		Guid getGuid();

		/**
		 * Adds the given block to the buffer with the length in front of it.
		 * Length is little-endian WORD.
		 */
		int addLEBlock( const QByteArray& block );

		/**
		 * Gets a block form the buffer with the length in front of it.
		 * Length is little-endian WORD.
		 */
		QByteArray getLEBlock();

		/**
		 * Adds the given block to the buffer with the length in front of it.
		 * Length is little-endian DWORD.
		 */
		int addLEDBlock( const QByteArray& block );

		/**
		 * Gets a block form the buffer with the length in front of it.
		 * Length is little-endian DWORD.
		 */
		QByteArray getLEDBlock();

		/**
		 * A block types.
		 */
		enum ByteOrder { BigEndian, LittleEndian };
		enum BlockType { BWord, BDWord };

		/**
		 * Start a block
		 * @param type is a type of block length
		 * @param byteOrder is a byte order of block length
		 */
		void startBlock( BlockType type, ByteOrder byteOrder = BigEndian );

		/**
		 * End a block.
		 */
		void endBlock();

		operator QByteArray() const;

	private:
		/**
		 * Make the buffer bigger by @p inc bytes
		 */
		void expandBuffer(unsigned int inc);

	private:
		QByteArray mBuffer;
		int mReadPos;

		struct Block
		{
			BlockType type;
			ByteOrder byteOrder;
			int pos;
		};
		QStack<Block> mBlockStack;
};

#endif
// kate: tab-width 4; indent-mode csands;
// vim: set noet ts=4 sts=4 sw=4:
